#pragma once
#include <memory>

#include "ImageIO.h"
#include "ImageProcessor.h"


class ImageProcessingUnit
{
	ImageProcessingUnit(const ImageProcessingUnit&) = delete;
	ImageProcessingUnit(ImageProcessingUnit&&) = delete;
	
public:
	using producer_ptr = std::shared_ptr<ImageProducer>;
	using consumer_ptr = std::shared_ptr<ImageConsumer>;
	using processor_ptr = std::shared_ptr<ImageProcessor>;

	ImageProcessingUnit(producer_ptr producer, consumer_ptr consumer)
		: producer_(producer), consumer_(consumer) {}

	void run();

private:
	producer_ptr producer_;
	consumer_ptr consumer_;
	processor_ptr processor_ = std::make_shared<EmptyImageProcessor>();
};