#ifndef HIVE_STORAGE_MANAGEMENT_SERVICE_HANDLER_H
#define HIVE_STORAGE_MANAGEMENT_SERVICE_HANDLER_H

#include "storage_management_service.h"
#include "slice_store.h"

namespace hive {
namespace storage {

class storage_management_service_handler : public storage_management_serviceIf {
	public:
		storage_management_service_handler() = delete;
		storage_management_service_handler(std::shared_ptr<slice_store> sstore);
		~storage_management_service_handler() = default;

		void put(const std::string &object, int32_t block_id, int32_t offset, int32_t size);
		void get(std::string &_return, int32_t block_id, int32_t offset, int32_t size);
	
		void async_put(async_response &_return, const slice_seq &seq, const std::string &object, int32_t block_id, int32_t offset, int32_t size);
		void async_get(async_response &_return, const slice_seq &seq, int32_t block_id, int32_t offset, int32_t size);

	private:
		std::shared_ptr<slice_store> sstore_;
};

}
}

#endif // HIVE_STORAGE_MANAGEMENT_SERVICE_HANDLER_H