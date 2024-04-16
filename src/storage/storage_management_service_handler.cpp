#include "storage_management_service_handler.h"
#include "utils/time_utils.h"
#include "spdlog/spdlog.h"

namespace hive {
namespace storage {

storage_management_service_handler::storage_management_service_handler(std::shared_ptr<slice_store> sstore)
				: sstore_(std::move(sstore)) {}

void storage_management_service_handler::put(const std::string &object, int32_t block_id, int32_t offset, int32_t size)
{
	auto start = utils::time_utils::now_us();
	sstore_->put(object, block_id, offset, size);
	auto end = utils::time_utils::now_us();
	spdlog::trace("storage_mgnt_service_handler: put: {}", end - start);
}

void storage_management_service_handler::get(std::string &_return, int32_t block_id, int32_t offset, int32_t size)
{
	auto start = utils::time_utils::now_us();
	_return = sstore_->get(block_id, offset, size);
	auto end = utils::time_utils::now_us();
	spdlog::trace("storage_mgnt_service_handler: get: {}", end - start);
}

void storage_management_service_handler::async_put(async_response &_return, const slice_seq &seq, const std::string &object, int32_t block_id, int32_t offset, int32_t size)
{
	auto start = utils::time_utils::now_us();
	sstore_->put(object, block_id, offset, size);
	_return.inode_name = std::move(seq.inode_name);
	_return.inode_idx = seq.inode_idx;
	_return.content = "";
	auto end = utils::time_utils::now_us();
	spdlog::trace("storage_mgnt_service_handler: async_put: {}", end - start);
}

void storage_management_service_handler::async_get(async_response &_return, const slice_seq &seq, int32_t block_id, int32_t offset, int32_t size)
{
	auto start = utils::time_utils::now_us();
	_return.content = sstore_->get(block_id, offset, size);
	_return.inode_name = std::move(seq.inode_name);
	_return.inode_idx = seq.inode_idx;
	auto end = utils::time_utils::now_us();
	spdlog::trace("storage_mgnt_service_handler: async_get: {}", end - start);
}

}
}