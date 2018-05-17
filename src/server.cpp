#include <chrono>
#include <experimental/filesystem>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>

#include "Setting.h"
#include "ImageIO.h"
#include "ImageConverter.h"

namespace fs = std::experimental::filesystem;
using boost::asio::ip::tcp;

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

	std::shared_ptr<ImageConverter> converter = std::make_shared<JPEGConverter>();

	boost::asio::io_service service;
	tcp::acceptor acceptor(service, tcp::endpoint(tcp::v4(), PORT));
	tcp::socket sock(service);
	bool isConnected = false;
	ImageSocket imsock(sock, converter);
	imsock.setOnError([&isConnected]() { isConnected = false; });

	std::queue<cv::Mat> frames;
	std::mutex mFrames;
	std::thread t([&]()
	{
		while (true)
		{
			std::cout << "Listening..." << std::endl;
			acceptor.accept(sock);
			std::cout << "Got connection!\n";
			isConnected = true;
			while (isConnected)
			{
				auto img = imsock.get();
				if (img.empty())
				{
					std::cout << "Image is empty." << std::endl;
					continue;
				}

				save(img);
				std::lock_guard<std::mutex> lock(mFrames);
				frames.push(img);
			}
			sock.close();
		}
	});

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

		imsock.put(img);
	}

	t.join();
	return 0;
}