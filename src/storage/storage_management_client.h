#ifndef HIVE_STORAGE_MANAGEMENT_CLIENT_H
#define HIVE_STORAGE_MANAGEMENT_CLIENT_H

#include "utils/inode_utils.h"
#include "storage_management_service.h"
#include <thrift/transport/TSocket.h>

namespace hive {
namespace storage {

class storage_management_client {
	public:
		typedef storage_management_serviceClient thrift_client;

		storage_management_client() = default;
		~storage_management_client();
		storage_management_client(const std::string &host, int port);

		void connect(const std::string &host, int port);
		void disconnect();

		void put(const std::string &object, int32_t block_id, int32_t offset, int32_t size);
		std::string get(int32_t block_id, int32_t offset, int32_t size);
	
		void send_async_put(const slice_seq &seq, const std::string &object, int32_t block_id, int32_t offset, int32_t size);
		async_response recv_async_put();
		
		void send_async_get(const slice_seq &seq, int32_t block_id, int32_t offset, int32_t size);
		async_response recv_async_get();
	
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
#endif // HIVE_STORAGE_MANAGEMENT_CLIENT_H