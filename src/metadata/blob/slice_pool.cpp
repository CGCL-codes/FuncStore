#include "slice_pool.h"
#include "utils/slice_utils.h"
#include "utils/time_utils.h"
#include "spdlog/spdlog.h"

namespace hive {
namespace metadata {

slice_pool::slice_pool(uint64_t invalid_ts, std::shared_ptr<bucket_allocator>bucket_alloc):
				invalid_ts_(invalid_ts),
				bucket_alloc_(std::move(bucket_alloc)),
				current_(nullptr) {}

std::string slice_pool::alloc(int32_t size)
{
	auto start = utils::time_utils::now_us();
	std::string slice;
	std::unique_lock<std::mutex> lock(mtx_);
	if (current_ == nullptr)
		current_ = bucket_alloc_->alloc();
	while(!current_->get_st(slice, size)) {
		immutable_buckets_.insert(current_->block(), current_);
		current_ = bucket_alloc_->alloc();
	}
	auto end = utils::time_utils::now_us();
	spdlog::trace("slice_pool: alloc: {}", end - start);
	return slice;
}

void slice_pool::free(const std::string &slice)
{
	auto block_id = utils::slice_utils::blockid(slice);

	std::unique_lock<std::mutex> lock(mtx_);
	if (current_->block() == block_id) {
		if (current_->put_st(slice))
			current_->reset_st();
		return;
	}
	lock.unlock();

	auto bkt = immutable_buckets_.find(block_id);
	if (bkt->put_mt(slice)) {
		bucket_alloc_->free(bkt);
		immutable_buckets_.erase(block_id);
	}
}

}
}