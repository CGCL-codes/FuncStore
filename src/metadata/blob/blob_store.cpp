#include "blob_store.h"
#include "utils/slice_utils.h"
#include "utils/time_utils.h"
#include "spdlog/spdlog.h"

namespace hive {
namespace metadata {

blob_store::blob_store(std::shared_ptr<block_allocator> block_alloc, std::vector<uint64_t> invalid_tsv):
				block_alloc_(std::move(block_alloc)) {
					for (auto &ts : invalid_tsv) {
						std::shared_ptr<slice_pool> pool = std::make_shared<slice_pool>(ts, std::make_shared<bucket_allocator>(block_alloc_));
						slice_pools_.insert(std::make_pair(ts, std::move(pool)));
					}
				}

inode blob_store::put(const std::string &obj_name, int64_t obj_size, uint64_t obj_lifetime)
{
	auto start = utils::time_utils::now_us();
	if (obj_size < block_alloc_->blocksize()) {
		/* blobsize < blocksize, allocate from slice pool */
		auto iter = slice_pools_.upper_bound(obj_lifetime);
		std::shared_ptr<slice_pool> pool = (iter != slice_pools_.end()) ? (iter->second) : (slice_pools_.rbegin()->second);

		inode info(obj_name, {pool->alloc(obj_size)}, obj_lifetime);
		index_table_.insert(obj_name, info);
		auto end1 = utils::time_utils::now_us();
		spdlog::trace("blob_store: put({}): {}", obj_size, end1 - start);
		return info;
	}
	
	/* blobsize >= blocksize, allocate from block pool */
	int32_t block_size = block_alloc_->blocksize();
	int32_t full_blocks = obj_size / block_size;
	int32_t odds = obj_size - full_blocks * block_size;
	int32_t total_blocks = (odds > 0) ? (full_blocks + 1) : full_blocks;

	auto blocks = block_alloc_->allocate(total_blocks, {});
	std::vector<std::string> slices;
	for (int i = 0; i < full_blocks; i++) {
		slices.emplace_back(utils::slice_utils::make(blocks[i], 0, block_size));
	}
	if (odds > 0) {
		slices.emplace_back(utils::slice_utils::make(blocks.at(full_blocks), 0, odds));
	}

	inode info(obj_name, std::move(slices), obj_lifetime);
	index_table_.insert(obj_name, info);
	auto end2 = utils::time_utils::now_us();
	spdlog::trace("blob_store: put({}): {}", obj_size, end2 - start);
	return info;
}

inode blob_store::get(const std::string &obj_name)
{
	auto start = utils::time_utils::now_us();
	auto ino = index_table_.find(obj_name);
	auto end1 = utils::time_utils::now_us();
	spdlog::trace("blob_store: get({}): {}", obj_name, end1 - start);
	return ino;
}

void blob_store::remove(const std::string &obj_name)
{
	std::unique_lock<std::mutex> lock(mtx_);
	utils::inode ino;
	try {
		ino = index_table_.find(obj_name);
	}
	catch (const std::out_of_range &e) {
		return;
	}
	index_table_.erase(obj_name);
	lock.unlock();
	
	if (ino.slices_.size() == 1 && utils::slice_utils::size(ino.slices_[0]) != block_alloc_->blocksize()) {
		auto pool_iter = slice_pools_.upper_bound(ino.lifetime_);
		std::shared_ptr<slice_pool> pool = (pool_iter != slice_pools_.end()) ? (pool_iter->second) : (slice_pools_.rbegin()->second);
		pool->free(ino.slices_[0]);
	}
	else {
		std::vector<std::string> blocks;
		for (auto &slice : ino.slices_)
			blocks.emplace_back(utils::slice_utils::blockid(slice));
		block_alloc_->free(blocks);
	}
}

}
}