#include "storage_management_server_nonblocking.h"
#include "storage_management_service_handler.h"

#include <thrift/transport/TNonblockingServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/concurrency/ThreadManager.h>

namespace hive {
namespace storage {

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

std::shared_ptr<apache::thrift::server::TNonblockingServer> storage_management_server_nb::create(std::shared_ptr<slice_store> sstore,
                                                                                         const std::string &address,
                                                                                         int port,
                                                                                         int num_io_threads,
                                                                                         int num_cpu_threads) {
  std::shared_ptr<storage_management_serviceIf>
      handler(new storage_management_service_handler(std::move(sstore)));
  std::shared_ptr<storage_management_serviceProcessor>
      processor(new storage_management_serviceProcessor(handler));
  std::shared_ptr<TBinaryProtocolFactory> protocol_factory(new TBinaryProtocolFactory());
  std::shared_ptr<TNonblockingServerSocket> sock(new TNonblockingServerSocket(address, port));
  
  std::shared_ptr<TNonblockingServer> server(new TNonblockingServer(processor, protocol_factory, sock));

  server->setNumIOThreads(num_io_threads);
  server->setUseHighPriorityIOThreads(true);
  return server;
}

}
}