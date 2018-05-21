#pragma once
#include <functional>
#include <memory>

#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>

#include "ImageConverter.h"

using boost::asio::ip::tcp;
using boost::system::error_code;


class ImageProducer
{
public:
	virtual cv::Mat get() = 0;
};


class ImageConsumer
{
public:
	virtual void put(cv::Mat) = 0;
};


class ImageSocket : public ImageProducer, public ImageConsumer
{
public:
	ImageSocket(tcp::socket& sock, std::shared_ptr<ImageConverter> converter);

	virtual cv::Mat get();
	virtual void put(cv::Mat image);

	void setOnError(std::function<void()> f) { onError = f; }

	static const int BUFF_SIZE = 4096;
	static const int BYTES_PER_NUM = 6;

private:
	std::function<void()> onError;

	tcp::socket& sock_;
	std::shared_ptr<ImageConverter> converter_;
};