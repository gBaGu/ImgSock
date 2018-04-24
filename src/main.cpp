#include <iostream>

#include <boost/asio.hpp>

#include "ImageListener.h"


int main()
{
	boost::asio::io_service service;
	ImageListener imgListener(service, 8080);
	imgListener.handleConnections();

	return 0;
}