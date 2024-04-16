#include <thrift/transport/TSocket.h>
#include "inode_management_service_factory.h"
#include "inode_management_service_handler.h"
#include "spdlog/spdlog.h"

namespace hive {
namespace metadata {

inode_management_service_factory::inode_management_service_factory(std::shared_ptr<blob_store> bstore, std::shared_ptr<monitor::monitor> mtr)
    : bstore_(std::move(bstore)),
      monitor_(std::move(mtr)) {}

inode_management_serviceIf *inode_management_service_factory::getHandler(const ::apache::thrift::TConnectionInfo &conn_info) {
  std::shared_ptr<::apache::thrift::transport::TSocket> sock = std::dynamic_pointer_cast<::apache::thrift::transport::TSocket>(conn_info.transport);
  spdlog::trace("Incoming connection from {}", sock->getSocketInfo());
  return new inode_management_service_handler(bstore_, monitor_);
}

void inode_management_service_factory::releaseHandler(inode_management_serviceIf *handler) {
  spdlog::trace("Releasing connection...");
  delete handler;
}

}
}