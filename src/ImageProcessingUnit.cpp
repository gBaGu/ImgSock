#include "ImageProcessingUnit.h"


ImageProcessingUnit::ImageProcessingUnit(producer_ptr producer,
	consumer_ptr consumer)
	:
	producer_(producer), consumer_(consumer)
{

}

void ImageProcessingUnit::setProcessor(processor_ptr processor)
{
	processor_ = processor;
}

void ImageProcessingUnit::run(std::function<bool()> upCondition)
{
	while (upCondition())
	{
		auto img = producer_->get();
		processor_->process(img);
		consumer_->put(img);
	}
}