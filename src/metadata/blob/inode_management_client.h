#ifndef HIVE_INODE_MANAGEMENT_CLIENT_H
#define HIVE_INODE_MANAGEMENT_CLIENT_H

#include "utils/inode_utils.h"
#include "inode_management_service.h"
#include <thrift/transport/TSocket.h>

namespace hive {
namespace metadata {

class inode_management_client {
	public:
		typedef inode_management_serviceClient thrift_client;

		inode_management_client() = default;
		~inode_management_client();
		inode_management_client(const std::string &host, int port);

		void connect(const std::string &host, int port);
		void disconnect();

		utils::inode put_lf(const std::string &obj_name, int64_t obj_size, int64_t obj_lifetime);
		utils::inode put(const std::string &obj_name, int64_t obj_size);
		utils::inode get(const std::string &obj_name);
		void remove(const std::string &obj_name);
		
		utils::inode hdag_put(const std::string &workflow_name, const std::string &function_name, const std::string &obj_name, int64_t obj_size);
		utils::inode hdag_get(const std::string &workflow_name, const std::string &function_name, const std::string &obj_name);
	
		void send_put(const std::string &obj_name, int64_t obj_size);
		utils::inode recv_put();
		void send_get(const std::string &obj_name);
		utils::inode recv_get();

		void send_hdag_put(const std::string &workflow_name, const std::string &function_name, const std::string &obj_name, int64_t obj_size);
		utils::inode recv_hdag_put();
		void send_hdag_get(const std::string &workflow_name, const std::string &function_name, const std::string &obj_name);
		utils::inode recv_hdag_get();

		std::shared_ptr<thrift_client>& get_thrift_client() { return client_; }

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
#endif // HIVE_INODE_MANAGEMENT_CLIENT_H