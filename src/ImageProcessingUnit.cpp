#include "ImageProcessingUnit.h"


void ImageProcessingUnit::run()
{
	while (true)
	{
		auto img = producer_->get();
		processor_->process(img);
		consumer_->put(img);
	}
}