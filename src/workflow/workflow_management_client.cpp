#include "workflow_management_client.h"
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include "utils/time_utils.h"
#include "spdlog/spdlog.h"

namespace hive {
namespace workflow {

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;

workflow_management_client::~workflow_management_client()
{
	if (transport_ != nullptr)
		disconnect();
}

workflow_management_client::workflow_management_client(const std::string &host, int port)
{
	connect(host, port);
}

void workflow_management_client::connect(const std::string &host, int port) {
  socket_ = std::make_shared<TSocket>(host, port);
  transport_ = std::shared_ptr<TTransport>(new TBufferedTransport(socket_));
  protocol_ = std::shared_ptr<TProtocol>(new TBinaryProtocol(transport_));
  client_ = std::make_shared<thrift_client>(protocol_);
  transport_->open();
}

void workflow_management_client::disconnect() {
  if (transport_->isOpen()) {
    transport_->close();
  }
}

void workflow_management_client::workflow_register(const std::string &workflow)
{
  client_->workflow_register(workflow);
}

void workflow_management_client::workflow_deregister(const std::string &workflow_name)
{
  client_->workflow_deregister(workflow_name);
}

}
}