#ifndef HIVE_INFERENCE_CLIENT_H
#define HIVE_INFERENCE_CLIENT_H

#include "inference_service.h"
#include <thrift/transport/TSocket.h>

namespace hive {
namespace predictor {

class inference_client {
	public:
		typedef inference_serviceClient thrift_client;

		inference_client() = default;
		~inference_client();
		inference_client(const std::string &host, int port);

		void connect(const std::string &host, int port);
		void disconnect();

    std::vector<int32_t> inference(const std::string &app, std::vector<std::vector<int32_t>> features_vec);

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

#endif // HIVE_INFERENCE_CLIENT_H