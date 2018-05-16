#pragma once
#include <functional>
#include <memory>

#include <boost/asio.hpp>

#include <opencv2/opencv.hpp>

#include "ImageConverter.h"

using boost::asio::ip::tcp;
using boost::system::error_code;


class ImageListener
{
public:
	ImageListener(boost::asio::io_service& ioService, size_t port);

	void handleConnections();
	void setOnReceived(std::function<void(cv::Mat)> f) { onReceived = f; }

	tcp::socket& getSocket() { return sock; }

private:
	tcp::acceptor acceptor;
	tcp::socket sock;
	std::unique_ptr<ImageConverter> converter;

	std::function<void(cv::Mat)> onReceived;
};