#include "ImageProcessor.h"

#include <chrono>
#include <iostream>
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
	endpoint_(endpoint), timeout_(timeout), sock_(service)
{
}

void BoostSocketAPIProcessor::process(cv::Mat& image)
{
	bool done = false;
	boost::asio::deadline_timer timer(sock_.get_io_service());
	timer.expires_from_now(timeout_);
	timer.async_wait([&done, this](const auto& error)
	{
		if (!error)
		{
			if (!done)
			{
				sock_.cancel();
			}
		}
	});

	boost::system::error_code ec;
	sock_.connect(endpoint_, ec);
	if (ec)
	{
		std::cout << "Failed to connect: " << ec.message() << std::endl;
		return;
	}
	if (!write(image))
	{
		return;
	}
	std::string filename = read();
	done = true;
	timer.cancel();
	auto retImage = cv::imread(filename);
	if (retImage.empty())
	{
		std::cout << "Cant read result of API" << std::endl;
		return;
	}
	image = retImage;
	sock_.close();
}

bool BoostSocketAPIProcessor::write(const cv::Mat& image)
{
	std::vector<unsigned char> buff;
	cv::imencode(".jpg", image, buff);
	std::string sizeString = std::to_string(buff.size());
	if (sizeString.size() > BYTES_PER_NUM)
	{
		std::cout << "Encoded image is too big. Skipping..." << std::endl;
		return false;
	}
	sizeString = std::string(BYTES_PER_NUM - sizeString.size(), '0') + sizeString;
	boost::system::error_code ec;
	boost::asio::write(sock_, boost::asio::buffer(sizeString), ec);
	if (ec)
	{
		std::cout << "Failed to write to socket: " << ec.message() << std::endl;
		return false;
	}
	boost::asio::write(sock_, boost::asio::buffer(buff), ec);
	if (ec)
	{
		std::cout << "Failed to write to socket: " << ec.message() << std::endl;
		return false;
	}
	return true;
}

std::string BoostSocketAPIProcessor::read()
{
	boost::asio::streambuf buff;
	boost::system::error_code ec;
	boost::asio::read_until(sock_, buff, '\n', ec);
	if (ec)
	{
		std::cout << "Failed to read response from server: "
			<< ec.message() << std::endl;
	}
	std::istream is(&buff);
	std::string ret;
	is >> ret;
	return ret;
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