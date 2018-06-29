#include "ImageProcessor.h"

#include <chrono>
#include <thread>
#include <vector>

#include <boost/lambda/lambda.hpp>
#include <boost/system/system_error.hpp>

#include "other.h"
#include "yolo_v2_class.hpp"

using boost::lambda::var;
using boost::lambda::_1;


ImageProcessor::~ImageProcessor()
{
	
}


void CSocketAPIProcessor::process(cv::Mat& image)
{
	if (image.empty())
	{
		return;
	}
	cv::rotate(image, image, cv::ROTATE_90_CLOCKWISE);
	image = sendImage(image);
}


BoostSocketAPIProcessor::BoostSocketAPIProcessor(boost::asio::io_service& service,
	tcp::endpoint endpoint, boost::posix_time::time_duration timeout)
	:
	endpoint_(endpoint), deadline_(service), timeout_(timeout), sock_(service)
{
}

void BoostSocketAPIProcessor::process(cv::Mat& image)
{
	std::cout << '.';
	std::cout.flush();
	deadline_.expires_from_now(boost::posix_time::milliseconds(110));
	boost::system::error_code ec = boost::asio::error::would_block;
	sock_.async_connect(endpoint_,
		[&ec](const boost::system::error_code& error)
		{
			ec = error; std::cout << '='; std::cout.flush();
		});
	do sock_.get_io_service().run_one(); while (ec == boost::asio::error::would_block);
	std::cout << ',';
	std::cout.flush();
	if (ec || !sock_.is_open())
	{
		std::cout << "Failed to connect to socket" << std::endl;
		return;
	}

	std::vector<unsigned char> buff;
	cv::imencode(".jpg", image, buff);
	std::string sizeString = std::to_string(buff.size());
	if (sizeString.size() > BYTES_PER_NUM)
	{
		std::cout << "Encoded image is too big. Skipping..." << std::endl;
		return;
	}
	sizeString = std::string(BYTES_PER_NUM - sizeString.size(), '0') + sizeString;
	std::cout << write(sock_, boost::asio::buffer(sizeString), ec) << std::endl;
	if (ec)
	{
		std::cout << "Failed to write to socket: " << ec.message() << std::endl;
		return;
	}
	std::cout << write(sock_, boost::asio::buffer(buff), ec) << std::endl;
	if (ec)
	{
		std::cout << "Failed to write to socket: " << ec.message() << std::endl;
		return;
	}

	char retbuff[512] = { '\0' };
	deadline_.expires_from_now(boost::posix_time::milliseconds(110));
	ec = boost::asio::error::would_block;
	boost::asio::async_read(sock_, boost::asio::buffer(retbuff), var(ec) = _1);
	do sock_.get_io_service().run_one(); while (ec == boost::asio::error::would_block);

	std::string filename(retbuff);
	auto retImage = cv::imread(filename);
	if (retImage.empty())
	{
		std::cout << "Cant read result of API" << std::endl;
	}
	else
	{
		std::cout << "Got image from Haik" << std::endl;
		image = retImage;
	}
	sock_.close();
}


DarknetProcessor::DarknetProcessor(const std::string& cfg, const std::string& weights)
{
	detector = std::make_shared<Detector>(cfg, weights, 0);
}

void DarknetProcessor::process(cv::Mat& image)
{
	cv::rotate(image, image, cv::ROTATE_90_CLOCKWISE);
	auto boxes = detector->detect(image, 0.2f);
	for (auto box : boxes)
	{
		cv::Rect rect(box.x, box.y, box.w, box.h);
		cv::rectangle(image, rect, cv::Scalar(255, 20, 147), 2);
	}
}


void DelayProcessor::process(cv::Mat& image)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	static int num = 0;
	cv::putText(image, std::to_string(num++), cv::Point(image.size()) / 4,
		cv::FONT_HERSHEY_SIMPLEX, 3, cv::Scalar(255, 20, 147), 2);
}