#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <chrono>
#include <ctime>
#include <experimental/filesystem>

#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>

#include <opencv2/opencv.hpp>

#include "ImageListener.h"

namespace fs = std::experimental::filesystem;
using namespace boost::archive::iterators;
using boost::property_tree::ptree;

typedef insert_linebreaks<base64_from_binary<transform_width<std::vector<unsigned char>::const_iterator,6,8> >, 72 > it_base64_t;

const size_t PORT = 8080;
const fs::path IMG_DIR = "img";


void save(cv::Mat img)
{
	const int BUFF_LEN = 25;
	char buff[BUFF_LEN];
	time_t t = time(0);
	strftime(buff, BUFF_LEN, "%Y-%m-%d %H:%M:%S", localtime(&t));
	std::string fullname = (IMG_DIR / (std::string(buff) + ".jpg")).string();
	cv::imwrite(fullname, img);
}


int main()
{
	std::error_code ec;
	fs::create_directories(IMG_DIR, ec);

	boost::asio::io_service service;
	ImageListener imgListener(service, PORT);

	std::queue<cv::Mat> frames;
	std::mutex mFrames;
	imgListener.setOnReceived([&](cv::Mat img)
		{
			if (img.empty())
				return;

			save(img);
			std::lock_guard<std::mutex> lock(mFrames);
			frames.push(img);
		});

	std::thread t1([&imgListener]() { imgListener.handleConnections(); });

	//*****ANDLE RECEIVED IMAGES*****
	while (true)
	{
		while (frames.empty())
			std::this_thread::sleep_for(std::chrono::milliseconds(20));

		cv::Mat img;
		{
			std::lock_guard<std::mutex> lock(mFrames);
			img = frames.front();
			frames.pop();
		}
		std::cout << "Got image from queue" << std::endl;

		int length = img.total() * img.elemSize();
		std::string data(it_base64_t(img.data), it_base64_t(img.data + length));

		//PACKING
		ptree pt;
		pt.put("data", data);
		pt.put("width", img.cols);
		pt.put("height", img.rows);
		pt.put("channels", img.elemSize());
		std::stringstream ss;
		write_json(ss, pt);
		try
		{
			imgListener.getSocket().write_some(boost::asio::buffer(ss.str()));
		}
		catch (const boost::system::system_error& ex)
		{
			std::cout << "Failed to write to client: " << ex.what() << std::endl;
		}
	}

	t1.join();
	return 0;
}