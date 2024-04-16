#ifndef HIVE_BUCKET_ALLOCATOR_H
#define HIVE_BUCKET_ALLOCATOR_H

#include "metadata/block/block_allocator.h"
#include "bucket.h"
#include <vector>
#include <memory>

namespace hive {
namespace metadata {

class bucket_allocator {
	public:
		bucket_allocator() = delete;
		bucket_allocator(std::shared_ptr<block_allocator> block_alloc):block_alloc_(std::move(block_alloc)) {}
		~bucket_allocator() = default;

		/**
		 * @brief Allocate a bucket
		 * @return Bucket pointer
		*/
		std::shared_ptr<bucket> alloc();
		
		/**
		 * @brief Deallocate a bucket
		 * @param bucket Bucket required to be deallocated
		 * @return Whether deallocate the bucket successfully
		*/
		bool free(std::shared_ptr<bucket> bucket);
		
	private:
		/* Block allocator */
		std::shared_ptr<block_allocator> block_alloc_;
};

}
}

#endif // HIVE_BUCKET_ALLOCATOR_H