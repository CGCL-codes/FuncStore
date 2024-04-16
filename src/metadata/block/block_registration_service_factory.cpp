#include <thrift/transport/TSocket.h>
#include "block_registration_service_factory.h"
#include "block_registration_service_handler.h"
#include "block_allocator.h"
#include "spdlog/spdlog.h"

namespace hive {
namespace metadata {

block_registration_service_factory::block_registration_service_factory(std::shared_ptr<block_allocator> alloc)
    : alloc_(std::move(alloc)) {}

block_registration_serviceIf *block_registration_service_factory::getHandler(const ::apache::thrift::TConnectionInfo &conn_info) {
  std::shared_ptr<::apache::thrift::transport::TSocket> sock = std::dynamic_pointer_cast<::apache::thrift::transport::TSocket>(conn_info.transport);
  spdlog::trace("Incoming connection from {}", sock->getSocketInfo());
  return new block_registration_service_handler(alloc_);
}

void block_registration_service_factory::releaseHandler(block_registration_serviceIf *handler) {
  spdlog::trace("Releasing connection...");
  delete handler;
}

}
}