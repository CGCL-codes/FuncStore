#ifndef HIVE_STORAGE_MANAGEMENT_SERVER_H
#define HIVE_STORAGE_MANAGEMENT_SERVER_H

#include "slice_store.h"
#include <thrift/server/TThreadedServer.h>

namespace hive {
namespace storage {

class storage_management_server {
	public:

		/**
		 * @brief Create storage management server
		 * @param sstore Slice store
		 * @param address Socket ip address
		 * @param port Socket port
		 * @return Storage management server
		*/
		static std::shared_ptr<apache::thrift::server::TThreadedServer> create(std::shared_ptr<slice_store> sstore,
                                                                         const std::string &address,
                                                                         int port);
};

}
}

#endif // HIVE_STORAGE_MANAGEMENT_SERVER_H