#ifndef HIVE_INODE_MANAGEMENT_SERVER_H
#define HIVE_INODE_MANAGEMENT_SERVER_H

#include "blob_store.h"
#include "monitor/monitor.h"
#include <thrift/server/TThreadedServer.h>

namespace hive {
namespace metadata {

class inode_management_server {
	public:

		/**
		 * @brief Create inode management server
		 * @param bstore Blob store
		 * @param address Socket ip address
		 * @param port Socket port
		 * @return Inode management server
		*/
		static std::shared_ptr<apache::thrift::server::TThreadedServer> create(std::shared_ptr<blob_store> bstore,
																																						std::shared_ptr<monitor::monitor> mtr, 
																																						const std::string &address,
																																						int port);
};

}
}

#endif // HIVE_INODE_MANAGEMENT_SERVER_H