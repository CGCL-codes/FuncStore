#ifndef HIVE_BLOCK_USAGE_TRACKER_H
#define HIVE_BLOCK_USAGE_TRACKER_H

#include <thread>
#include <atomic>
#include <memory>
#include "block_allocator.h"

namespace hive {
namespace metadata {

/* Block usage tracker class */
class block_usage_tracker {
	public:
	
	/**
   * @brief Constructor
   * @param alloc Block allocator
   * @param periodicity_ms Working period
   * @param output_file Output file name
  */
  block_usage_tracker(std::shared_ptr<block_allocator> alloc,
                    uint64_t periodicity_ms,
                    const std::string &output_file);

  /**
   * @brief Destructor
  */
  ~block_usage_tracker();

  /**
   * @brief Start worker
   * Report block usage and sleep until next period
  */
  void start();

  /**
   * @brief Stop worker
  */
  void stop();

 private:

  /**
   * @brief Report number of allocated blocks
   * @param out Output file stream
  */
  void report_allocated_blocks(std::ofstream &out);

  /* Working period */
  std::chrono::milliseconds periodicity_ms_;
  /* Stop bool */
  std::atomic_bool stop_{false};
  /* Worker thread */
  std::thread worker_;
  /* Output file name */
  std::string output_file_;
  /* Block allocator */
  std::shared_ptr<block_allocator> alloc_;
};

}
}

#endif //  HIVE_BLOCK_USAGE_TRACKER_H