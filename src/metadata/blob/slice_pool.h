#ifndef HIVE_SLICE_POOL_H
#define HIVE_SLICE_POOL_H

#include "bucket.h"
#include "bucket_allocator.h"
#include "libcuckoo/cuckoohash_map.hh"

namespace hive {
namespace metadata {

class slice_pool {
	public:
		slice_pool() = delete;
		slice_pool(uint64_t invalid_ts, std::shared_ptr<bucket_allocator>bucket_alloc);
		~slice_pool() = default;

		/**
		 * @brief Allocate a slice
		 * @param size Required size
		 * @return A slice with size
		*/
		std::string alloc(int32_t size);

		/**
		 * @brief Deallocate a slice
		 * @param slice Slice need to be deallocate
		*/
		void free(const std::string &slice);

	private:
		/* Bucket invalid timestamp */
		uint64_t invalid_ts_;
		/* Bucket allocator */
		std::shared_ptr<bucket_allocator> bucket_alloc_;
		/* Immutable (blockid, bucket) hashtable, need to be thread safe */
		libcuckoo::cuckoohash_map<std::string, std::shared_ptr<bucket>> immutable_buckets_;
		/* Current mutable bucket */
		std::shared_ptr<bucket> current_;
		/* Mutex for @current_ */
		mutable std::mutex mtx_;
};

}
}

#endif // HIVE_SLICE_POOL_H