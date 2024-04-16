#ifndef HIVE_RANDOM_BLOCK_ALLOCATOR_H
#define HIVE_RANDOM_BLOCK_ALLOCATOR_H

#include <string>
#include <vector>
#include "concurrentqueue/concurrentqueue.h"
#include "libcuckoo/cuckoohash_map.hh"
#include <shared_mutex>
#include "block_allocator.h"

namespace hive {
namespace metadata {

/* Random block allocator class, inherited from block allocator */
class random_block_allocator : public block_allocator {
  public:
  random_block_allocator() = delete;

  random_block_allocator(int32_t blocksize):blocksize_(blocksize) {}

  virtual ~random_block_allocator() = default;

  /**
   * @brief Allocate blocks in different prefixes
   * @param count Number of blocks
   * @return Block names
  */
  std::vector<std::string> allocate(std::size_t count, const std::vector<std::string> &exclude_list) override;

  /**
   * @brief Free blocks
   * @param blocks Block names
  */
  void free(const std::vector<std::string> &block_name) override;

  /**
   * @brief Add blocks to free block list
   * @param block_names Block names
  */
  void add_blocks(const std::vector<std::string> &block_names) override;

  /**
   * @brief Remove blocks from free block list
   * @param block_names Block names
  */
  void remove_blocks(const std::vector<std::string> &block_names) override;

  /**
   * @brief Fetch number of free blocks
   * @return Number of free blocks
  */
  std::size_t num_free_blocks() override;

  /**
   * @brief Fetch number of allocated blocks
   * @return Number of allocated blocks
  */
  std::size_t num_allocated_blocks() override;

  /**
   * @brief Fetch number of total blocks
   * @return Number of total blocks
  */
  std::size_t num_total_blocks() override;

  /**
   * @brief Fetch size of block
   * @return Block size
  */
  int32_t blocksize() override { return blocksize_; }

  private:
  /*
   * Fetch prefix of block name
  */
  std::string prefix(const std::string &block_name) const {
    auto pos = block_name.find_last_of(':');
    if (pos == std::string::npos) {
      throw std::logic_error("Malformed block name [" + block_name + "]");
    }
    return block_name.substr(0, pos);
  }
  
  /* Operation mutex */
  // std::mutex mtx_;
  /* Block size */
  int32_t blocksize_;
  /* Allocated blocks */
  libcuckoo::cuckoohash_map<std::string, std::string> allocated_blocks_;
  /* Free blocks */
  moodycamel::ConcurrentQueue<std::string> free_blocks_;
};

}
}

#endif // HIVE_RANDOM_BLOCK_ALLOCATOR_H