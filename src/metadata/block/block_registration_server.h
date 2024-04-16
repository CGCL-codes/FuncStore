#ifndef HIVE_BLOCK_REGISTRATION_SERVER_H
#define HIVE_BLOCK_REGISTRATION_SERVER_H

#include <thrift/server/TThreadedServer.h>
#include "block_allocator.h"

namespace hive {
namespace metadata {

/* Block registration server class */
class block_registration_server {
 public:

  /**
   * @brief Create block allocation server
   * @param alloc Block allocator
   * @param address Socket address
   * @param port Socket port number
   * @return Block allocation server
   */

  static std::shared_ptr<apache::thrift::server::TThreadedServer> create(std::shared_ptr<block_allocator> alloc,
                                                                         const std::string &address,
                                                                         int port);

};

}
}
#endif // HIVE_BLOCK_REGISTRATION_SERVER_H