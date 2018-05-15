#include "ImageListener.h"

#include <iostream>
#include <vector>

#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/remove_whitespace.hpp>
#include <boost/bind.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "Setting.h"

using namespace boost::archive::iterators;
using boost::property_tree::ptree;

typedef transform_width<binary_from_base64<remove_whitespace<std::string::const_iterator> >, 8, 6 > it_binary_t;


size_t read_complete(char * buff, const error_code & err, size_t bytes)
{
	if (err) return 0;
	if (bytes == 0)
	{
		return 1;
	}

	return *(buff + (bytes - 1)) == '}' ? 0 : 1;
}


ImageListener::ImageListener(boost::asio::io_service& ioService, size_t port)
	:
	acceptor(ioService, tcp::endpoint(tcp::v4(), port)),
	sock(ioService)
{
	onReceived = [](cv::Mat img) {};
	converter = std::make_unique<JPEGConverter>();
}

void ImageListener::handleConnections()
{
	const int BYTES_PER_NUM = 6;
	char buff[BUFF_SIZE];

	while (true)
	{
		std::cout << "Listening..." << std::endl;
		acceptor.accept(sock);
		std::cout << "Got connection!\n";

		bool alive = true;
		while (alive)
		{
			error_code ec;
			int bytesRead = 0;
			//*****READ IMAGE SIZE*****
			bytesRead = read(sock, boost::asio::buffer(buff, BUFF_SIZE),
				boost::asio::transfer_exactly(BYTES_PER_NUM), ec);
			std::cout << "Bytes (nImageSize): " << bytesRead << std::endl;
			if (ec)
			{
				std::cout << "Unable to read from socket" << std::endl;
				alive = false;
				continue;
			}

			std::string str(buff, buff + BYTES_PER_NUM);
			std::cout << str << std::endl;
			int nImageSize = 0;
			try
			{
				nImageSize = stoi(str);
			}
			catch (const std::invalid_argument& ex)
			{
				std::cout << "Unable to parse size of the image.\n"
					"Check length (should be 6 bytes)" << std::endl;
				alive = false;
				continue;
			}
			//=========================
			//*****READ IMAGE DATA*****
			std::string data;
			bytesRead = 0;
			int bytesRemain = nImageSize;
			do
			{
				bytesRead = read(sock, boost::asio::buffer(buff, BUFF_SIZE),
					boost::asio::transfer_exactly(bytesRemain), ec);
				data.append(buff, bytesRead);
				bytesRemain -= bytesRead;
				std::cout << "Bytes (image): " << bytesRead << std::endl;
			} while (!ec && bytesRemain > 0);

			if (ec)
			{
				std::cout << "Failed to read data from socket." << std::endl;
				if (!(ec == boost::system::errc::stream_timeout ||
						ec == boost::system::errc::timed_out))
				{
					std::cout << "Error code: " << ec.message() << std::endl;
					alive = false;
				}
				continue;
			}
			//=========================
			std::vector<unsigned char> ucdata(data.begin(), data.begin() + nImageSize);
			cv::Mat img = converter->fromData(ucdata);
			onReceived(img);
		}
		sock.close();
	}
}

cv::Mat ImageListener::parseJson(const std::string& json)
{
	ptree pt;
	std::stringstream ss;
	ss << json;
	read_json(ss, pt);

	std::vector<unsigned char> data;
	int width = 0;
	int height = 0;
	int elemSize = 0;

	std::string tmp;
	tmp = pt.get<std::string>("type");
	data = std::vector<unsigned char>(it_binary_t(tmp.begin()),
									  it_binary_t(tmp.end()));

	tmp = pt.get<std::string>("width");
	std::cout << tmp << std::endl;
	width = stoi(tmp);

	tmp = pt.get<std::string>("height");
	std::cout << tmp << std::endl;
	height = stoi(tmp);

	tmp = pt.get<std::string>("length");
	std::cout << tmp << std::endl;
	elemSize = stoi(tmp);

	return cv::Mat(height, width, CV_8UC3, data.data());
}