#include "inode_management_service_handler.h"
#include "inode_type_conversions.h"
#include "utils/time_utils.h"
#include "spdlog/spdlog.h"

namespace hive {
namespace metadata {

inode_management_service_handler::inode_management_service_handler(std::shared_ptr<blob_store> bstore, std::shared_ptr<monitor::monitor> mtr)
				: bstore_(std::move(bstore)),
					monitor_(std::move(mtr)) {}

void inode_management_service_handler::put_lf(rpc_inode &_return, const std::string &obj_name, int64_t obj_size, int64_t obj_lifetime)
{
	_return = inode_type_conversions::to_rpc(bstore_->put(obj_name, obj_size, obj_lifetime));
}

void inode_management_service_handler::put(rpc_inode &_return, const std::string &obj_name, int64_t obj_size)
{
	auto start = utils::time_utils::now_us();
	int64_t obj_lifetime = INT32_MAX;
	hive::utils::inode inode;
	try {
		inode = bstore_->put(obj_name, obj_size, obj_lifetime);
	}
	catch (std::out_of_range &e) {
		spdlog::info("inode_mgnt_service_handler::put({}, {}): {}", obj_name, obj_size, e.what());
		inode.lifetime_ = 0;
	}
	_return = inode_type_conversions::to_rpc(inode);
	auto end = utils::time_utils::now_us();
	spdlog::trace("inode_mgnt_service_handler: put: {}", end - start);
}

void inode_management_service_handler::get(rpc_inode &_return, const std::string &obj_name)
{
	auto start = utils::time_utils::now_us();
	hive::utils::inode inode;
	try {
		inode = bstore_->get(obj_name);
	}
	catch (std::out_of_range &e) {
		spdlog::info("inode_mgnt_service_handler::get({}): {}", obj_name, e.what());
		inode.lifetime_ = 0;
	}
	_return = inode_type_conversions::to_rpc(inode);
	auto end = utils::time_utils::now_us();
	spdlog::trace("inode_mgnt_service_handler: get: {}", end - start);
}

void inode_management_service_handler::remove(const std::string &obj_name)
{
	bstore_->remove(obj_name);
}

void inode_management_service_handler::hdag_put(rpc_inode &_return, const std::string &workflow_name, const std::string &function_name, const std::string &obj_name, int64_t obj_size)
{
	monitor_->hook_put(workflow_name, function_name, obj_name);
	auto obj_lifetime = monitor_->predictor()->predict(workflow_name, obj_name);
	hive::utils::inode inode;
	try {
		inode = bstore_->put(obj_name, obj_size, obj_lifetime);
	}
	catch (std::out_of_range &e) {
		spdlog::info("inode_mgnt_service_handler::hdag_put({}, {}, {}, {}): {}", workflow_name, function_name, obj_name, obj_size, e.what());
		inode.lifetime_ = 0;
	}
	_return = inode_type_conversions::to_rpc(inode);
}

void inode_management_service_handler::hdag_get(rpc_inode &_return, const std::string &workflow_name, const std::string &function_name, const std::string &obj_name)
{
	monitor_->hook_get(workflow_name, function_name, obj_name);
	hive::utils::inode inode;
	try {
		inode = bstore_->get(obj_name);
	}
	catch (std::out_of_range &e) {
		spdlog::info("inode_mgnt_service_handler::hdag_get({}, {}, {}): {}", workflow_name, function_name, obj_name, e.what());
		inode.lifetime_ = 0;
	}
	_return = inode_type_conversions::to_rpc(inode);
}

}
}