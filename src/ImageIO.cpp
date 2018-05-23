#include "ImageIO.h"

#include <iostream>
#include <string>
#include <vector>


ImageProducer::~ImageProducer()
{
	
}


ImageConsumer::~ImageConsumer()
{
	
}


ImageSocket::ImageSocket(tcp::socket& sock,
	std::shared_ptr<ImageConverter> converter)
	:
	sock_(sock), converter_(converter)
{
	onError = []() {};
}

cv::Mat ImageSocket::get()
{
	error_code ec;
	int bytesRead = 0;
	char buff[BUFF_SIZE];
	std::string data;
	//*****READ IMAGE SIZE*****
	bytesRead = read(sock_, boost::asio::buffer(buff, BUFF_SIZE),
		boost::asio::transfer_exactly(BYTES_PER_NUM), ec);
	if (ec)
	{
		std::cout << "Failed to read from socket: " << ec.message()
			<< "\nBytes transferred: " << bytesRead << std::endl;
		onError();
		return cv::Mat();
	}
	bytesRead = 0;

	data = std::string(buff, buff + BYTES_PER_NUM);
	std::cout << "Waiting for " << data << " bytes..." << std::endl;
	int bufferLength = 0;
	try
	{
		bufferLength = stoi(data);
		data.clear();
	}
	catch (const std::invalid_argument& ex)
	{
		std::cout << "Failed to parse size of the image.\n"
			<< "Check length (should be 6 bytes)" << std::endl;
		onError();
		return cv::Mat();
	}
	//=========================
	//*****READ IMAGE DATA*****
	int bytesRemain = bufferLength;
	do
	{
		bytesRead = read(sock_, boost::asio::buffer(buff, BUFF_SIZE),
			boost::asio::transfer_exactly(bytesRemain), ec);
		data.append(buff, bytesRead);
		bytesRemain -= bytesRead;
		std::cout << "Bytes (image): " << bytesRead << std::endl;
	} while (!ec && bytesRemain > 0);

	if (ec)
	{
		std::cout << "Failed to read from socket: " << ec.message()
			<< "\nBytes transferred: " << bufferLength - bytesRemain << std::endl;
		onError();
		return cv::Mat();
	}
	//=========================

	std::vector<unsigned char> ucdata(data.begin(), data.end());
	cv::Mat image = converter_->fromData(ucdata);
	return image;
}

void ImageSocket::put(cv::Mat image)
{
	if (image.empty())
	{
		return;
	}

	std::vector<unsigned char> buff = converter_->toData(image);
	std::string sizeString = std::to_string(buff.size());
	if (sizeString.size() > BYTES_PER_NUM)
	{
		std::cout << "Encoded image is too big. Skipping..." << std::endl;
		return;
	}
	sizeString = std::string(BYTES_PER_NUM - sizeString.size(), '0') + sizeString;
	std::cout << "Sending back: " << sizeString << " bytes..." << std::endl;
	error_code ec;
	std::cout << write(sock_, boost::asio::buffer(sizeString), ec) << std::endl;
	if (ec)
	{
		std::cout << "Failed to write to socket: " << ec.message() << std::endl;
		onError();
		return;
	}
	std::cout << write(sock_, boost::asio::buffer(buff), ec) << std::endl;
	if (ec)
	{
		std::cout << "Failed to write to socket: " << ec.message() << std::endl;
		onError();
		return;
	}
}


cv::Mat ThreadSafeQueue::get()
{
	std::unique_lock<std::mutex> lock(mutex_);
	while (queue_.empty())
	{
		cond_.wait(lock);
	}
	cv::Mat img = queue_.front();
	queue_.pop();
	return img;
}

void ThreadSafeQueue::put(cv::Mat image)
{
	{
		std::lock_guard<std::mutex> lock(mutex_);
		queue_.push(image);
	}
	cond_.notify_one();
}