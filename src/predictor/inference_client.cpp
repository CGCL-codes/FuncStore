#include "inference_client.h"
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>

namespace hive {
namespace predictor {

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;

inference_client::~inference_client()
{
	if (transport_ != nullptr)
		disconnect();
}

inference_client::inference_client(const std::string &host, int port)
{
	connect(host, port);
}

void inference_client::connect(const std::string &host, int port) {
  socket_ = std::make_shared<TSocket>(host, port);
  transport_ = std::shared_ptr<TTransport>(new TBufferedTransport(socket_));
  protocol_ = std::shared_ptr<TProtocol>(new TBinaryProtocol(transport_));
  client_ = std::make_shared<thrift_client>(protocol_);
  transport_->open();
}

void inference_client::disconnect() {
  if (transport_->isOpen()) {
    transport_->close();
  }
}

std::vector<int32_t> inference_client::inference(const std::string &app, std::vector<std::vector<int32_t>> features_vec)
{
  rpc_features_vec rpc_features;
  rpc_label_vec rpc_label;

  rpc_features.vec = std::move(features_vec);
  client_->inference(rpc_label, app, rpc_features);

  return std::move(rpc_label.vec);
}

}
}