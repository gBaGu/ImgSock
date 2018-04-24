#include <iostream>
#include <ctime>
#include <experimental/filesystem>

#include <boost/asio.hpp>

#include "ImageListener.h"

namespace fs = std::experimental::filesystem;

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
	imgListener.setOnReceived(save);

	imgListener.handleConnections();

	return 0;
}