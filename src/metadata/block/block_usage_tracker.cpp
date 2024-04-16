#include <iostream>
#include <fstream>
#include "block_usage_tracker.h"
#include "utils/time_utils.h"
#include "spdlog/spdlog.h"

namespace hive {
namespace metadata {

block_usage_tracker::block_usage_tracker(std::shared_ptr<block_allocator> alloc,
                                     uint64_t periodicity_ms,
                                     const std::string &output_file)
    : periodicity_ms_(periodicity_ms),
      output_file_(output_file),
      alloc_(std::move(alloc)) {}

block_usage_tracker::~block_usage_tracker() {
  stop_.store(true);
  if (worker_.joinable())
    worker_.join();
}

void block_usage_tracker::start() {
  worker_ = std::thread([&] {
    std::ofstream out(output_file_);
    stop_.store(false);
    while (!stop_.load()) {
      auto start = std::chrono::steady_clock::now();
      try {
        report_allocated_blocks(out);
      } catch (std::exception &e) {
				spdlog::error(e.what());
      }
      auto end = std::chrono::steady_clock::now();
      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

      auto time_to_wait = std::chrono::duration_cast<std::chrono::milliseconds>(periodicity_ms_ - elapsed);
      if (time_to_wait > std::chrono::milliseconds::zero()) {
        std::this_thread::sleep_for(time_to_wait);
      }
    }
    out.close();
  });
}

void block_usage_tracker::stop() {
  stop_.store(true);
}

void block_usage_tracker::report_allocated_blocks(std::ofstream &out) {
  auto timestamp = utils::time_utils::now_ms();
  out << timestamp << ", " << alloc_->num_allocated_blocks() << std::endl;
}

}
}