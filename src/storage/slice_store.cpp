#include "slice_store.h"
#include <cstdlib>
#include "utils/time_utils.h"
#include "spdlog/spdlog.h"

namespace hive {
namespace storage {

slice_store::slice_store(uint64_t capacity, int32_t blocksize)
				: capacity_(capacity), 
				blocksize_(blocksize),
				addr_(nullptr) {}

slice_store::~slice_store() {}

bool slice_store::init()
{
	if(addr_) {
		free(addr_);
	}
	addr_ = (char *)malloc(capacity_);
	return (addr_) ? true : false;
}

void slice_store::destroy()
{
	if (addr_) {
		free(addr_);
	}
}

void slice_store::put(const std::string &object, int32_t block_id, int32_t offset, int32_t size)
{
	auto start = utils::time_utils::now_us();
	char *slice_start = addr_ + static_cast<uint64_t>(blocksize_) * block_id + offset;
	object.copy(slice_start, size);
	auto end = utils::time_utils::now_us();
	spdlog::trace("slice_store: put: {}", end - start);
}

std::string slice_store::get(int32_t block_id, int32_t offset, int32_t size)
{
	char *slice_start = addr_ + static_cast<uint64_t>(blocksize_) * block_id + offset;
	return std::string(slice_start, size);
}

}
}