#include <thrift/transport/TSocket.h>
#include "storage_management_service_factory.h"
#include "storage_management_service_handler.h"
#include "spdlog/spdlog.h"

namespace hive {
namespace storage {

storage_management_service_factory::storage_management_service_factory(std::shared_ptr<slice_store> sstore)
    : sstore_(std::move(sstore)) {}

storage_management_serviceIf *storage_management_service_factory::getHandler(const ::apache::thrift::TConnectionInfo &conn_info) {
  std::shared_ptr<::apache::thrift::transport::TSocket> sock = std::dynamic_pointer_cast<::apache::thrift::transport::TSocket>(conn_info.transport);
  spdlog::trace("Incoming connection from {}", sock->getSocketInfo());
  return new storage_management_service_handler(sstore_);
}

void storage_management_service_factory::releaseHandler(storage_management_serviceIf *handler) {
  spdlog::trace("Releasing connection...");
  delete handler;
}

}
}