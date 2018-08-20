#pragma once
#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>

using boost::asio::ip::tcp;

class Detector;


class ImageProcessor
{
public:
	virtual ~ImageProcessor() = 0;

	/*modifies input image*/
	virtual void process(cv::Mat&) = 0;
};


class EmptyImageProcessor : public ImageProcessor
{
public:
	virtual void process(cv::Mat& image) {};
};


class CSocketAPIProcessor : public ImageProcessor
{
public:
	virtual void process(cv::Mat& image);
};


//TODO: test it
class BoostSocketAPIProcessor : public ImageProcessor
{
public:
	BoostSocketAPIProcessor(boost::asio::io_service& service,
		tcp::endpoint endpoint, boost::posix_time::time_duration timeout);

	virtual void process(cv::Mat& image);

	static const int BYTES_PER_NUM = 6;

private:
	bool write(const cv::Mat& image);
	std::string read();

	tcp::endpoint endpoint_;
	boost::posix_time::time_duration timeout_;
	tcp::socket sock_;
};


class DarknetProcessor : public ImageProcessor
{
public:
	DarknetProcessor(const std::string& cfg, const std::string& weights);

	virtual void process(cv::Mat& image);

private:
	std::shared_ptr<Detector> detector;
};


class DelayProcessor : public ImageProcessor
{
public:
	DelayProcessor() {}

	virtual void process(cv::Mat& image);
};