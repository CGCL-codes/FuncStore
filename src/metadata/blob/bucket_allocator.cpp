#include "bucket_allocator.h"

namespace hive {
namespace metadata {

std::shared_ptr<bucket> bucket_allocator::alloc()
{
	return std::make_shared<bucket>(block_alloc_->allocate(1, {}).at(0), block_alloc_->blocksize());
}

bool bucket_allocator::free(std::shared_ptr<bucket> bucket)
{
	block_alloc_->free({bucket->block()});
	return true;
}

}
}