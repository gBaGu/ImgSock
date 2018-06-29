#include "other.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <memory>
#include <vector>


cv::Mat sendImage(cv::Mat image)
{
	std::string host = "localhost";
	int port = 2500;
	struct hostent *server = gethostbyname(host.c_str());
	if (server == NULL)
	{
		std::cerr << "Error, no such host\n";
		return image;
	}

	struct sockaddr_in serv_addr;
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, 
		(char *)&serv_addr.sin_addr.s_addr,
		server->h_length);
	serv_addr.sin_port = htons(port);

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	auto lambdaDeleter = [] (int* pInt)
		{
			if (*pInt != -1)
			{
				close(*pInt);
			}
		};
	auto socketGuard = std::unique_ptr<int, decltype(lambdaDeleter)>(&sockfd, lambdaDeleter);
	if (sockfd == -1)
	{
		std::cerr << "Error occurred while opening socket\n";
		return image;
	}
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)
	{
		std::cerr << "Error connecting to " << host << std::endl;
		return image;
	}

	const int SMALL_BUFFER_SIZE = 256;
	const int BIG_BUFFER_SIZE = 8192;
	char smallBuffer[SMALL_BUFFER_SIZE] = { '\0' };

	//*****SEND IMAGE*****
	std::vector<unsigned char> buff;
	cv::imencode(".jpg", image, buff);
	auto data = buff.data();
	int size = buff.size();
	auto itData = data;
	
	sprintf(smallBuffer, "%06i", size);
	int sendSize = strlen(smallBuffer);
	int n = write(sockfd, smallBuffer, sendSize);
	bzero(smallBuffer, sendSize);
	if (n == -1)
	{
		std::cout << "Failed to write to " << host << std::endl;
		return image;
	}

	int dataSent = 0;
	while (itData != (data + size))
	{
		int packetSize = std::min(BIG_BUFFER_SIZE, size - dataSent);
		n = write(sockfd, itData, packetSize);
		if (n == -1)
		{
			std::cerr << "Failed to write to " << host << std::endl;
			return image;
		}
		dataSent += n;
		itData = data + dataSent;
	}
	//===============================
	//-----READ ANSWER FROM NETWORK-----
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(sockfd, &rfds);
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 200000;

	n = select(sockfd + 1, &rfds, NULL, NULL, &tv);
	if (n > 0)
	{
		n = read(sockfd, smallBuffer, SMALL_BUFFER_SIZE);
	}
	if (n <= 0) //-1 for error, 0 for timeout on select()
	{
		std::cerr << "Failed to read from " << host << std::endl;
		return image;
	}

	auto retImage = cv::imread(std::string(smallBuffer));
	return retImage.empty() ? image : retImage;
}