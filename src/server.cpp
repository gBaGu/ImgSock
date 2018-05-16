#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <chrono>
#include <ctime>
#include <experimental/filesystem>

#include <boost/asio.hpp>

#include <opencv2/opencv.hpp>

#include "Setting.h"
#include "ImageListener.h"

namespace fs = std::experimental::filesystem;

const fs::path IMG_DIR = "img";


void save(cv::Mat img)
{
	static int n = 0;
	std::string fullname = (IMG_DIR / (std::to_string(n++) + ".jpg")).string();
	std::cout << fullname << std::endl;
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
			{
				std::cout << "Image is empty." << std::endl;
				return;
			}

			save(img);
			std::lock_guard<std::mutex> lock(mFrames);
			frames.push(img);
		});

	std::thread t1([&imgListener]() { imgListener.handleConnections(); });

	//*****HANDLE RECEIVED IMAGES*****
	while (true)
	{
		while (frames.empty())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		}

		cv::Mat img;
		{
			std::lock_guard<std::mutex> lock(mFrames);
			img = frames.front();
			frames.pop();
		}
		std::cout << "Got image from queue" << std::endl;

		boost::system::error_code ec;
		std::vector<unsigned char> buff;
		cv::imencode(".jpg", img, buff);
		int size = buff.size();

		std::string data = std::to_string(size);
		if (data.size() > BYTES_PER_NUM)
		{
			std::cout << "Encoded image is too big. Skipping..." << std::endl;
			continue;
		}
		data = std::string(BYTES_PER_NUM - size, '0') + data;
		write(imgListener.getSocket(), boost::asio::buffer(data), ec);
		if (ec)
		{
			std::cout << "Unable to write to socket: " << ec.message() << std::endl;
			//TODO: insert reaction on error
		}
		write(imgListener.getSocket(), boost::asio::buffer(buff), ec);
		if (ec)
		{
			std::cout << "Unable to write to socket: " << ec.message() << std::endl;
			//TODO: insert reaction on error
		}
	}

	t1.join();
	return 0;
}