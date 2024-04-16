#include "block_registration_service_handler.h"
#include "spdlog/spdlog.h"

namespace hive {
namespace metadata {

block_registration_service_handler::block_registration_service_handler(std::shared_ptr<block_allocator> alloc)
    : alloc_(std::move(alloc)) {}

void block_registration_service_handler::add_blocks(const std::vector<std::string> &block_names) {
  try {
    spdlog::info("Received advertisement for {} blocks", block_names.size());
    alloc_->add_blocks(block_names);
  } catch (std::out_of_range &e) {
    throw make_exception(e);
  }
}

void block_registration_service_handler::remove_blocks(const std::vector<std::string> &block_names) {
  try {
    spdlog::info("Received retraction for {} blocks", block_names.size());
    alloc_->remove_blocks(block_names);
  } catch (std::out_of_range &e) {
    throw make_exception(e);
  }
}

block_registration_service_exception block_registration_service_handler::make_exception(const std::out_of_range &e) {
  block_registration_service_exception ex;
  ex.msg = e.what();
  return ex;
}

}
}