#include <iostream>
#include <algorithm>
#include "random_block_allocator.h"
#include "utils/rand_utils.h"
#include "utils/time_utils.h"
#include "spdlog/spdlog.h"

namespace hive {
namespace metadata {

std::vector<std::string> random_block_allocator::allocate(std::size_t count, const std::vector<std::string> &exclude_list)
{
  auto start = utils::time_utils::now_us();
  // std::unique_lock<std::mutex> lock(mtx_);

  if (count > free_blocks_.size_approx()) {
    throw std::out_of_range(
        "Insufficient free blocks to allocate from (requested: " + std::to_string(count) + ", have: "
            + std::to_string(free_blocks_.size_approx()));
  }

  std::vector<std::string> blocks(count);
  size_t blocks_considered = 0;
  for (std::size_t i = 0; i < count; i++) {
    if (free_blocks_.try_dequeue(blocks[i])) {
      blocks_considered++;
      continue;
    }
    break;
  }
  if (blocks_considered < count) {
    for (size_t i = 0; i < blocks_considered; i++)
      free_blocks_.enqueue(blocks[i]);
    throw std::out_of_range(
        "Insufficient free blocks to allocate from (requested: " + std::to_string(count) + ", have: "
            + std::to_string(blocks_considered));
  }
  
  for (auto &block : blocks) {
    allocated_blocks_.insert(block, block);
  }

  auto end = utils::time_utils::now_us();
  spdlog::trace("random_allocator: allocate: {}", end - start);
  return blocks;
}

void random_block_allocator::free(const std::vector<std::string> &blocks) {
  // std::unique_lock<std::mutex> lock(mtx_);
  std::vector<std::string> not_freed;
  for (auto &block_name: blocks) {
    if (allocated_blocks_.erase(block_name))
      continue;
    not_freed.push_back(block_name);
  }
  if (not_freed.size() > 0) {
    throw std::out_of_range("Could not free these blocks because they have not been allocated: " + not_freed.size());
  }

  for (auto &block : blocks) {
    free_blocks_.enqueue(block);
  }
}

void random_block_allocator::add_blocks(const std::vector<std::string> &block_names) {
  // std::unique_lock<std::mutex> lock(mtx_);
  for (auto &block_name : block_names) {
    free_blocks_.enqueue(block_name);
  }
}

void random_block_allocator::remove_blocks(const std::vector<std::string> &block_names) {
  //
}

std::size_t random_block_allocator::num_free_blocks() {
  // std::unique_lock<std::mutex> lock(mtx_);
  return free_blocks_.size_approx();
}

std::size_t random_block_allocator::num_allocated_blocks() {
  // std::unique_lock<std::mutex> lock(mtx_);
  return allocated_blocks_.size();
}

std::size_t random_block_allocator::num_total_blocks() {
  // std::unique_lock<std::mutex> lock(mtx_);
  return free_blocks_.size_approx() + allocated_blocks_.size();
}

} // metadata
} // hive