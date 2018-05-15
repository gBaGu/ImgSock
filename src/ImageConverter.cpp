#include "ImageConverter.h"

#include "Setting.h"


cv::Mat JPEGConverter::fromData(const std::vector<unsigned char>& data)
{
	cv::Mat tmp(data);
	cv::Mat img = cv::imdecode(tmp, 1);
	return img;
}


cv::Mat RGB565Converter::fromData(const std::vector<unsigned char>& data)
{
	float factor5Bit = 255.0 / 31.0;
	float factor6Bit = 255.0 / 63.0;

	std::vector<unsigned char> rgb888data;
	for (int i = 0; i < data.size() - 1; i += 2)
	{
		unsigned char r5 = (data[i] & 0b11111000) >> 3;
		unsigned char g6 = ((data[i] & 0b00000111) << 3) | ((data[i + 1] & 0b11100000) >> 5);
		unsigned char b5 = data[i + 1] & 0b00011111;
		unsigned char r8 = floor((r5 * factor5Bit) + 0.5);
        unsigned char g8 = floor((g6 * factor6Bit) + 0.5);
        unsigned char b8 = floor((b5 * factor5Bit) + 0.5);
		rgb888data.push_back(r8);
		rgb888data.push_back(g8);
		rgb888data.push_back(b8);
	}
	cv::Mat img(HEIGHT, WIDTH, CV_8UC3, rgb888data.data());

	return img;
}


cv::Mat YUVConverter::fromData(const std::vector<unsigned char>& data)
{
	std::vector<unsigned char> tmpdata = data;
	cv::Mat imgYV12 = cv::Mat(HEIGHT * 3/2, WIDTH, CV_8UC1,
		const_cast<unsigned char*>(tmpdata.data()));
	cv::Mat img;
	cv::cvtColor(imgYV12, img, CV_YUV2BGR_YV12);
	return img;
}