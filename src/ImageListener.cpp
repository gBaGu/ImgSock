#include "ImageListener.h"

#include <iostream>
#include <vector>

#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/remove_whitespace.hpp>
#include <boost/bind.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

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


void ImageListener::handleConnections()
{
	char buff[1024];
	while (true)
	{
		std::cout << "Listening..." << std::endl;
		acceptor.accept(sock);
		std::cout << "Got connection!\n";

		bool alive = true;
		while (alive)
		{
			error_code ec;
			int bytesRead = read(sock, boost::asio::buffer(buff),
				boost::bind(read_complete, buff, _1, _2), ec);
			if (ec)
			{
				std::cout << "Failed to read data from socket." << std::endl;
				alive = false;
				continue;
			}

			cv::Mat img;
			try
			{
				img = parseJson(buff, bytesRead);
			}
			catch (const std::exception& ex)
			{
				std::cout << "Failed to parse json: " << ex.what() << std::endl;
				continue;
			}
			onReceived(img);
		}
	}
}

cv::Mat ImageListener::parseJson(char* buff, int bytes)
{
	ptree pt;
	std::stringstream ss;
	ss.write(buff, bytes);
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