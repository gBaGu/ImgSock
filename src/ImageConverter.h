#pragma once
#include <iostream>
#include <vector>

#include <opencv2/opencv.hpp>


class ImageConverter
{
public:
	virtual ~ImageConverter() {}

	virtual cv::Mat fromData(const std::vector<unsigned char>& data) = 0;
	virtual std::vector<unsigned char> toData(cv::Mat image) = 0;
};


class JPEGConverter : public ImageConverter
{
public:
	JPEGConverter(int quality) : ImageConverter(), quality_(quality)
	{}

	cv::Mat fromData(const std::vector<unsigned char>& data);
	std::vector<unsigned char> toData(cv::Mat image);

private:
	int quality_;
};


class RGB565Converter : public ImageConverter
{
public:
	RGB565Converter() : ImageConverter() {}

	cv::Mat fromData(const std::vector<unsigned char>& data);
	std::vector<unsigned char> toData(cv::Mat image);
};


class YUVConverter : public ImageConverter
{
public:
	YUVConverter() : ImageConverter() {}

	cv::Mat fromData(const std::vector<unsigned char>& data);
	std::vector<unsigned char> toData(cv::Mat image);
};