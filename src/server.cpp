#include <chrono>
#include <experimental/filesystem>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>

#include "ImageConverter.h"
#include "ImageIO.h"
#include "ImageProcessingUnit.h"
#include "Setting.h"

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

	boost::asio::io_service service;
	tcp::acceptor acceptor(service, tcp::endpoint(tcp::v4(), PORT));
	tcp::socket sock(service);
	while (true)
	{
		std::cout << "Listening..." << std::endl;
		acceptor.accept(sock);
		bool isConnected = true;
		std::cout << "Got connection!\n";

		//*****Input/Output*****
		std::shared_ptr<ImageConverter> converter = std::make_shared<JPEGConverter>();
		std::shared_ptr<ImageSocket> imsock =
			std::make_shared<ImageSocket>(sock, converter);
		imsock->setOnError([&isConnected]() { isConnected = false; });
		//======================
		//*****ImageProcessingUnits communication*****
		std::shared_ptr<ThreadSafeQueue> queue =
			std::make_shared<ThreadSafeQueue>();
		//============================================
		//*****Creating and linking ImageProcessingUnits*****
		ImageProcessingUnit recievingUnit(imsock, queue);
		ImageProcessingUnit sendingUnit(queue, imsock);
		//===================================================
		//*****Launching units*****
		std::thread tRecievingUnit(&ImageProcessingUnit::run, &recievingUnit);
		std::thread tSendingUnit(&ImageProcessingUnit::run, &sendingUnit);
		tRecievingUnit.join();
		tSendingUnit.join();
		//=========================

		sock.close();
	}
	
	return 0;
}