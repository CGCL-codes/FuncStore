#ifndef HIVE_BLOCK_ALLOCATOR_H
#define HIVE_BLOCK_ALLOCATOR_H

#include <string>
#include <vector>

namespace hive {
namespace metadata {

/* Block allocator virtual class */
class block_allocator {
 public:
  virtual ~block_allocator() = default;

  virtual std::vector<std::string> allocate(std::size_t count, const std::vector<std::string> &exclude_list) = 0;
  virtual void free(const std::vector<std::string> &block_name) = 0;
  virtual void add_blocks(const std::vector<std::string> &block_names) = 0;
  virtual void remove_blocks(const std::vector<std::string> &block_names) = 0;

  virtual std::size_t num_free_blocks() = 0;
  virtual std::size_t num_allocated_blocks() = 0;
  virtual std::size_t num_total_blocks() = 0;

  virtual int32_t blocksize() = 0;
};

}
}

#endif // HIVE_BLOCK_ALLOCATOR_H