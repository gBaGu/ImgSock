#pragma once
#include <functional>

#include <boost/asio.hpp>

#include <opencv2/opencv.hpp>

using boost::asio::ip::tcp;
using boost::system::error_code;


size_t read_complete(char * buff, const error_code & err, size_t bytes);


class ImageListener
{
public:
	ImageListener(boost::asio::io_service& ioService, int port)
		:
		acceptor(ioService, tcp::endpoint(tcp::v4(), port)),
		sock(ioService)
	{
		onReceived = [](cv::Mat img) {};
	}

	void handleConnections();

	tcp::socket& getSocket() { return sock; }

private:
	tcp::acceptor acceptor;
	tcp::socket sock;

	std::function<void(cv::Mat)> onReceived;

	cv::Mat parseJson(char* buff, int bytes);
};