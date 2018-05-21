#pragma once
#include <opencv2/opencv.hpp>


class ImageProcessor
{
public:
	virtual ~ImageProcessor() = 0;

	/*modifies input image*/
	virtual void process(cv::Mat) = 0;
};


class EmptyImageProcessor : public ImageProcessor
{
public:
	virtual void process(cv::Mat image) {};
};