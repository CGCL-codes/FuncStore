#ifndef HIVE_WORKFLOW_MANAGEMENT_SERVER_H
#define HIVE_WORKFLOW_MANAGEMENT_SERVER_H

#include "monitor/monitor.h"
#include <thrift/server/TThreadedServer.h>

namespace hive {
namespace workflow {

class workflow_management_server {
	public:

		/**
		 * @brief Create workflow management server
		 * @param mtr Monitor
		 * @param address Socket ip address
		 * @param port Socket port
		 * @return workflow management server
		*/
		static std::shared_ptr<apache::thrift::server::TThreadedServer> create(std::shared_ptr<monitor::monitor> mtr, 
																																						const std::string &address,
																																						int port);
};

}
}

#endif // HIVE_WORKFLOW_MANAGEMENT_SERVER_H