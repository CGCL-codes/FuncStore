#ifndef HIVE_BUCKET_H
#define HIVE_BUCKET_H

#include <string>
#include <atomic>
#include <shared_mutex>
#include "utils/block_utils.h"
#include "metadata/block/block_allocator.h"

namespace hive {
namespace metadata {

class bucket {
	public:
		bucket() = delete;
		bucket(const std::string &host, int32_t port, int32_t id, int32_t capacity);
		bucket(const std::string &block_name, int32_t capacity);
		~bucket() = default;

		/**
		 * @brief Get a slice from bucket, called after holding the current lock
		 * @param slice Allocated slice
		 * @param size Slice size
		 * @return If allocated successfully
		*/
		bool get_st(std::string &slice, int32_t size);
		
		/**
		 * @brief Put a slice to bucket, called after holding the current lock
		 * @param slice Slice to be deallocated
		 * @return Whether memory freed in the bucket equals allocated after this put()
		*/
		bool put_st(const std::string &slice);

		/**
		 * @brief Put a slice to bucket, called after the bucket has been immutable
		 * @param slice Slice to be deallocated
		 * @return Whether memory freed in the bucket equals allocated after this put()
		*/
		bool put_mt(const std::string &slice);

		/**
		 * @brief Fetch block name
		 * @return Block name
		*/
		inline std::string block() { return bid_; }

		/**
		 * @brief Reset cur_offset_ and freed_ to zero, called after holding the current lock
		*/
		inline void reset_st() {
			cur_offset_ = 0;
			freed_memory_ = 0;
		}

	private:
		/* Bucket attached block ID */
		std::string bid_;
		/* Bucket capacity */
		int32_t capacity_;
		/**
		 * Bucket current offset
		 * 
		 * When the bucket is mutable, the mutex semantics are guaranteed by the caller,
		 * which means it is accessed by concurrent get_st() threads and put_st()[maybe followed by reset_st()] threads, and these threads are synchronized by upper-level mutex
		 * 
		 * When the bucket is immutable, the value of @cur_offset_ is read-only,
		 * so there is no need to make it mutex
		*/
		int32_t cur_offset_;
		/**
		 * Memory freed in the bucket
		 * 
		 * When the bucket is mutable, the mutex semantics are guaranteed by the caller,
		 * which means it is accessed by concurrent put_st()[maybe followed by reset_st()] threads, and these threads are synchronized by upper-level mutex
		 * 
		 * When the bucket is immutable, it is accessed by concurrent put_mt(), which use @mtx_ to keep operations mutex
		*/
		int32_t freed_memory_;
		/* Operation mutex for @put_mt() */
		mutable std::mutex mtx_;
};

}
}

#endif // HIVE_BUCKET_H