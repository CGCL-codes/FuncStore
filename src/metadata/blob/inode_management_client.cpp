#include "inode_management_client.h"
#include "inode_type_conversions.h"
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <iostream>
#include "utils/time_utils.h"
#include "spdlog/spdlog.h"

namespace hive {
namespace metadata {

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;

inode_management_client::~inode_management_client()
{
	if (transport_ != nullptr)
		disconnect();
}

inode_management_client::inode_management_client(const std::string &host, int port)
{
	connect(host, port);
}

void inode_management_client::connect(const std::string &host, int port) {
  socket_ = std::make_shared<TSocket>(host, port);
  transport_ = std::shared_ptr<TTransport>(new TBufferedTransport(socket_));
  protocol_ = std::shared_ptr<TProtocol>(new TBinaryProtocol(transport_));
  client_ = std::make_shared<thrift_client>(protocol_);
  transport_->open();
  if(client_ == nullptr)
    throw std::runtime_error("inode_management_client: failed to create thrift client");
}

void inode_management_client::disconnect() {
  if (transport_->isOpen()) {
    transport_->close();
  }
}

utils::inode inode_management_client::put_lf(const std::string &obj_name, int64_t obj_size, int64_t obj_lifetime)
{
  rpc_inode rpc_info;
	client_->put_lf(rpc_info, obj_name, obj_size, obj_lifetime);
  return inode_type_conversions::from_rpc(rpc_info);
}

utils::inode inode_management_client::put(const std::string &obj_name, int64_t obj_size)
{
  auto start = utils::time_utils::now_us();
	rpc_inode rpc_info;
  if(client_ == nullptr)
    throw std::runtime_error("inode_management_client: put: client is null");
  // else std::cerr<<"inode_management_client: put: client is not null"<<std::endl;
  try{
    client_->put(rpc_info, obj_name, obj_size);
  }
  catch (const std::exception& e) {
    std::cerr << "inode_management_client: put: " << e.what() << std::endl;
  }
  auto end = utils::time_utils::now_us();
  spdlog::trace("inode_mgnt_client: put: {}", end - start);
  return inode_type_conversions::from_rpc(rpc_info);
}

utils::inode inode_management_client::get(const std::string &obj_name)
{
  auto start = utils::time_utils::now_us();
	rpc_inode rpc_info;
  client_->get(rpc_info, obj_name);
  auto end = utils::time_utils::now_us();
  spdlog::trace("inode_mgnt_client: get: {}", end - start);
  return inode_type_conversions::from_rpc(rpc_info);
}

void inode_management_client::remove(const std::string &obj_name)
{
	client_->remove(obj_name);
}

utils::inode inode_management_client::hdag_put(const std::string &workflow_name, const std::string &function_name, const std::string &obj_name, int64_t obj_size)
{
  auto start = utils::time_utils::now_us();
	rpc_inode rpc_info;
  client_->hdag_put(rpc_info, workflow_name, function_name, obj_name, obj_size);
  auto end = utils::time_utils::now_us();
  spdlog::trace("inode_mgnt_client: hdag_put: {}", end - start);
  return inode_type_conversions::from_rpc(rpc_info);
}

utils::inode inode_management_client::hdag_get(const std::string &workflow_name, const std::string &function_name, const std::string &obj_name)
{
  auto start = utils::time_utils::now_us();
	rpc_inode rpc_info;
  client_->hdag_get(rpc_info, workflow_name, function_name, obj_name);
  auto end = utils::time_utils::now_us();
  spdlog::trace("inode_mgnt_client: hdag_get: {}", end - start);
  return inode_type_conversions::from_rpc(rpc_info);
}

void inode_management_client::send_put(const std::string &obj_name, int64_t obj_size)
{
  auto start = utils::time_utils::now_us();
  client_->send_put(obj_name, obj_size);
  auto end = utils::time_utils::now_us();
  spdlog::trace("inode_mgnt_client: send_put: {}", end - start);
}

utils::inode inode_management_client::recv_put()
{
  rpc_inode rpc_info;
  auto start = utils::time_utils::now_us();
  client_->recv_put(rpc_info);
  auto end = utils::time_utils::now_us();
  spdlog::trace("inode_mgnt_client: recv_put: {}", end - start);
  return inode_type_conversions::from_rpc(rpc_info);
}

void inode_management_client::send_get(const std::string &obj_name)
{
  auto start = utils::time_utils::now_us();
  client_->send_get(obj_name);
  auto end = utils::time_utils::now_us();
  spdlog::trace("inode_mgnt_client: send_get: {}", end - start);
}

utils::inode inode_management_client::recv_get()
{
  rpc_inode rpc_info;
  auto start = utils::time_utils::now_us();
  client_->recv_get(rpc_info);
  auto end = utils::time_utils::now_us();
  spdlog::trace("inode_mgnt_client: recv_get: {}", end - start);
  return inode_type_conversions::from_rpc(rpc_info);
}

void inode_management_client::send_hdag_put(const std::string &workflow_name, const std::string &function_name, const std::string &obj_name, int64_t obj_size)
{
  auto start = utils::time_utils::now_us();
  client_->send_hdag_put(workflow_name, function_name, obj_name, obj_size);
  auto end = utils::time_utils::now_us();
  spdlog::trace("inode_mgnt_client: send_hdag_put: {}", end - start);
}

utils::inode inode_management_client::recv_hdag_put()
{
  rpc_inode rpc_info;
  auto start = utils::time_utils::now_us();
  client_->recv_hdag_put(rpc_info);
  auto end = utils::time_utils::now_us();
  spdlog::trace("inode_mgnt_client: recv_hdag_put: {}", end - start);
  return inode_type_conversions::from_rpc(rpc_info);
}

void inode_management_client::send_hdag_get(const std::string &workflow_name, const std::string &function_name, const std::string &obj_name)
{
  auto start = utils::time_utils::now_us();
  client_->send_hdag_get(workflow_name, function_name, obj_name);
  auto end = utils::time_utils::now_us();
  spdlog::trace("inode_mgnt_client: send_hdag_get: {}", end - start);
}

utils::inode inode_management_client::recv_hdag_get()
{
  rpc_inode rpc_info;
  auto start = utils::time_utils::now_us();
  client_->recv_hdag_get(rpc_info);
  auto end = utils::time_utils::now_us();
  spdlog::trace("inode_mgnt_client: recv_hdag_get: {}", end - start);
  return inode_type_conversions::from_rpc(rpc_info);
}

}
}