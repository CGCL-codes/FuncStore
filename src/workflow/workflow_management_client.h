#ifndef HIVE_WORKFLOW_MANAGEMENT_CLIENT_H
#define HIVE_WORKFLOW_MANAGEMENT_CLIENT_H

#include "workflow_management_service.h"
#include <thrift/transport/TSocket.h>

namespace hive {
namespace workflow {

class workflow_management_client {
	public:
		typedef workflow_management_serviceClient thrift_client;

		workflow_management_client() = default;
		~workflow_management_client();
		workflow_management_client(const std::string &host, int port);

		void connect(const std::string &host, int port);
		void disconnect();

    void workflow_register(const std::string &workflow);
    void workflow_deregister(const std::string &workflow_name);

	private:
		/* Socket */
  	std::shared_ptr<apache::thrift::transport::TSocket> socket_{};
  	/* Transport */
  	std::shared_ptr<apache::thrift::transport::TTransport> transport_{};
  	/* Protocol */
  	std::shared_ptr<apache::thrift::protocol::TProtocol> protocol_{};
  	/* Client */
  	std::shared_ptr<thrift_client> client_{};
};

}
}
#endif // HIVE_WORKFLOW_MANAGEMENT_CLIENT_H