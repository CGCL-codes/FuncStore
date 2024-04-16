#include "storage_management_client.h"
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>

namespace hive {
namespace storage {

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;

storage_management_client::~storage_management_client()
{
	if (transport_ != nullptr)
		disconnect();
}

storage_management_client::storage_management_client(const std::string &host, int port)
{
	connect(host, port);
}

void storage_management_client::connect(const std::string &host, int port) {
  socket_ = std::make_shared<TSocket>(host, port);
#ifdef USE_NONBLOCKING_STORAGE_SERVER
  transport_ = std::shared_ptr<TTransport>(new TFramedTransport(socket_));
#else
  transport_ = std::shared_ptr<TTransport>(new TBufferedTransport(socket_));
#endif // USE_NONBLOCKING_STORAGE_SERVER
  protocol_ = std::shared_ptr<TProtocol>(new TBinaryProtocol(transport_));
  client_ = std::make_shared<thrift_client>(protocol_);
  transport_->open();
}

void storage_management_client::disconnect() {
  if (transport_->isOpen()) {
    transport_->close();
  }
}

void storage_management_client::put(const std::string &object, int32_t block_id, int32_t offset, int32_t size)
{
	client_->put(object, block_id, offset, size);
}

std::string storage_management_client::get(int32_t block_id, int32_t offset, int32_t size)
{
	std::string ret;
	client_->get(ret, block_id, offset, size);
	return ret;
}

void storage_management_client::send_async_put(const slice_seq &seq, const std::string &object, int32_t block_id, int32_t offset, int32_t size)
{
  client_->send_async_put(seq, object, block_id, offset, size);
}

async_response storage_management_client::recv_async_put()
{
  async_response ret;
  client_->recv_async_put(ret);
  return ret;
}

void storage_management_client::send_async_get(const slice_seq &seq, int32_t block_id, int32_t offset, int32_t size)
{
  client_->send_async_get(seq, block_id, offset, size);
}

async_response storage_management_client::recv_async_get()
{
  async_response ret;
  client_->recv_async_get(ret);
  return ret;
}

}
}