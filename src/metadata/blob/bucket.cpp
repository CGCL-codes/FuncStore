#include "bucket.h"
#include "utils/slice_utils.h"
#include <mutex>
#include <atomic>

namespace hive {
namespace metadata {

bucket::bucket(const std::string &host, int32_t port, int32_t id, int32_t capacity):
				bid_(utils::block_utils::make(host, port, id)),
				capacity_(capacity),
				cur_offset_(0),
				freed_memory_(0) {}

bucket::bucket(const std::string &block_name, int32_t capacity):
				bid_(block_name),
				capacity_(capacity),
				cur_offset_(0),
				freed_memory_(0) {}

bool bucket::get_st(std::string &slice, int32_t size)
{
	if (capacity_ - cur_offset_ >= size) {
		slice = utils::slice_utils::make(bid_, cur_offset_, size);
		cur_offset_ += size;
		return true;
	}
	return false;
}

bool bucket::put_st(const std::string &slice)
{
	freed_memory_ += utils::slice_utils::size(slice);
	if (freed_memory_ == cur_offset_) {
		return true;
	}
	return false;
}

bool bucket::put_mt(const std::string &slice)
{
	std::unique_lock<std::mutex> lock(mtx_);
	freed_memory_ += utils::slice_utils::size(slice);
	if (freed_memory_ == cur_offset_) {
		return true;
	}
	return false;
}

}
}