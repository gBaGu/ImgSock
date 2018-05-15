#pragma once
#include <iostream>
#include <vector>

#include <opencv2/opencv.hpp>


class ImageConverter
{
public:
	virtual ~ImageConverter() {}

	virtual cv::Mat fromData(const std::vector<unsigned char>& data) = 0;
};


class JPEGConverter : public ImageConverter
{
public:
	JPEGConverter() : ImageConverter() {}

	cv::Mat fromData(const std::vector<unsigned char>& data);
};


class RGB565Converter : public ImageConverter
{
public:
	RGB565Converter() : ImageConverter() {}

	cv::Mat fromData(const std::vector<unsigned char>& data);
};


class YUVConverter : public ImageConverter
{
public:
	YUVConverter() : ImageConverter() {}

	cv::Mat fromData(const std::vector<unsigned char>& data);
};