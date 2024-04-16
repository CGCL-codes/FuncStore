#include <thrift/transport/TSocket.h>
#include "workflow_management_service_factory.h"
#include "workflow_management_service_handler.h"
#include "spdlog/spdlog.h"

namespace hive {
namespace workflow {

workflow_management_service_factory::workflow_management_service_factory(std::shared_ptr<monitor::monitor> mtr)
    : monitor_(std::move(mtr)) {}

workflow_management_serviceIf *workflow_management_service_factory::getHandler(const ::apache::thrift::TConnectionInfo &conn_info) {
  std::shared_ptr<::apache::thrift::transport::TSocket> sock = std::dynamic_pointer_cast<::apache::thrift::transport::TSocket>(conn_info.transport);
  spdlog::trace("Incoming connection from {}", sock->getSocketInfo());
  return new workflow_management_service_handler(monitor_);
}

void workflow_management_service_factory::releaseHandler(workflow_management_serviceIf *handler) {
  spdlog::trace("Releasing connection...");
  delete handler;
}

}
}