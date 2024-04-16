#ifndef HIVE_STORAGE_MANAGEMENT_SERVER_NONBLOCKING_H
#define HIVE_STORAGE_MANAGEMENT_SERVER_NONBLOCKING_H

#include "slice_store.h"
#include <thrift/server/TNonblockingServer.h>

namespace hive {
namespace storage {

class storage_management_server_nb {
	public:

		/**
		 * @brief Create storage management server
		 * @param sstore Slice store
		 * @param address Socket ip address
		 * @param port Socket port
		 * @return Storage management server
		*/
		static std::shared_ptr<apache::thrift::server::TNonblockingServer> create(std::shared_ptr<slice_store> sstore,
                                                                         const std::string &address,
                                                                          int port,
                                                                          int num_io_threads,
                                                                          int num_cpu_threads);
};

}
}

#endif // HIVE_STORAGE_MANAGEMENT_SERVER_NONBLOCKING_H