#include "inode_management_server.h"
#include "inode_management_service_factory.h"

#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/concurrency/ThreadManager.h>

namespace hive {
namespace metadata {

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

std::shared_ptr<apache::thrift::server::TThreadedServer> inode_management_server::create(std::shared_ptr<blob_store> bstore,
                                                                                        std::shared_ptr<monitor::monitor> mtr,
                                                                                        const std::string &address,
                                                                                        int port) {
  std::shared_ptr<inode_management_serviceIfFactory>
      clone_factory(new inode_management_service_factory(std::move(bstore), std::move(mtr)));
  std::shared_ptr<inode_management_serviceProcessorFactory>
      proc_factory(new inode_management_serviceProcessorFactory(clone_factory));
  std::shared_ptr<TServerSocket> sock(new TServerSocket(address, port));
  std::shared_ptr<TBufferedTransportFactory> transport_factory(new TBufferedTransportFactory());
  std::shared_ptr<TBinaryProtocolFactory> protocol_factory(new TBinaryProtocolFactory());
  std::shared_ptr<TThreadedServer> server(new TThreadedServer(proc_factory, sock, transport_factory, protocol_factory));
  return server;
}

}
}