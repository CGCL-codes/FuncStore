#ifndef HIVE_INODE_MANAGEMENT_SERVICE_HANDLER_H
#define HIVE_INODE_MANAGEMENT_SERVICE_HANDLER_H

#include "inode_management_service.h"
#include "blob_store.h"
#include "monitor/monitor.h"

namespace hive {
namespace metadata {

class inode_management_service_handler : public inode_management_serviceIf {
	public:
		explicit inode_management_service_handler(std::shared_ptr<blob_store> bstore, std::shared_ptr<monitor::monitor> mtr);

		void put_lf(rpc_inode &_return, const std::string &obj_name, int64_t obj_size, int64_t obj_lifetime) override;
		void put(rpc_inode &_return, const std::string &obj_name, int64_t obj_size) override;
		void get(rpc_inode &_return, const std::string &obj_name) override;
		void remove(const std::string &obj_name) override;

		void hdag_put(rpc_inode &_return, const std::string &workflow_name, const std::string &function_name, const std::string &obj_name, int64_t obj_size);
		void hdag_get(rpc_inode &_return, const std::string &workflow_name, const std::string &function_name, const std::string &obj_name);

	private:
		std::shared_ptr<blob_store> bstore_;
		std::shared_ptr<monitor::monitor> monitor_;
};

}
}

#endif // HIVE_INODE_MANAGEMENT_SERVICE_HANDLER_H