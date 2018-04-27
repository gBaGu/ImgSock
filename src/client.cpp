#include <iostream>

#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>

#include <opencv2/opencv.hpp>

using namespace boost::archive::iterators;
using boost::asio::ip::tcp;
using boost::property_tree::ptree;

typedef insert_linebreaks<base64_from_binary<transform_width<std::vector<unsigned char>::const_iterator,6,8> >, 72 > it_base64_t;

const size_t PORT = 8080;


size_t read_complete(char * buff, const boost::system::error_code & err, size_t bytes)
{
	if (err) return 0;
	if (bytes == 0)
	{
		return 1;
	}

	return *(buff + (bytes - 1)) == '}' ? 0 : 1;
}


int main()
{
	boost::asio::io_service service;
	tcp::socket sock(service);
	sock.connect(tcp::endpoint(tcp::v4(), PORT));

	cv::Mat img = cv::imread("img.jpg");
	int length = img.total() * img.elemSize();
	std::string data(it_base64_t(img.data), it_base64_t(img.data + length));

	//PACKING
	ptree pt;
	pt.put("type", data);
	pt.put("width", img.cols);
	pt.put("height", img.rows);
	pt.put("length", img.elemSize());
	std::stringstream ss;
	write_json(ss, pt);
	std::string json = ss.str();
	try
	{
		write(sock, boost::asio::buffer(json));
	}
	catch (const boost::system::system_error& ex)
	{
		std::cout << "Failed to write to client: " << ex.what() << std::endl;
	}
	std::cout << "Send: " << json << std::endl;

	char buff[2048];
	boost::system::error_code ec;
	data.clear();
	int bytesRead = 0;
	do
	{
		bytesRead = read(sock, boost::asio::buffer(buff),
			boost::bind(read_complete, buff, _1, _2), ec);
		data.append(buff, bytesRead);
	} while (!ec && bytesRead > 0 && *(buff + (bytesRead - 1)) != '}');

	if (ec)
	{
		std::cout << "Failed to read data from socket: "
			<< "Error code: " << ec.message() << std::endl;
	}

	std::cout << "Read: " << data << std::endl;

	return 0;
}