#include "ImageListener.h"

#include <iostream>
#include <vector>

#include "Setting.h"


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
				std::cout << "Unable to read from socket: " << ec.message() << std::endl;
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

			//std::cout << "Sending back " << str << " bytes (" << data.size() << ")" << std::endl;
			//std:: cout << write(sock, boost::asio::buffer(data)) << std::endl;

			std::vector<unsigned char> ucdata(data.begin(), data.begin() + nImageSize);
			cv::Mat img = converter->fromData(ucdata);
			onReceived(img);
		}
		sock.close();
	}
}