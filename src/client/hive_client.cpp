#include "hive_client.h"
#include "utils/slice_utils.h"
#include "utils/time_utils.h"
#include "spdlog/spdlog.h"

namespace hive {
namespace client {

using namespace ::hive::metadata;
using namespace ::hive::storage;
using namespace ::hive::utils;

hive_client::hive_client(const std::string &metadata_host, int metadata_port) :
#ifdef ENABLE_CACHE_CLIENT
				 client_cache_(0),
#endif // ENABLE_CACHE_CLIENT
				metadata_client_(std::make_shared<inode_management_client>(metadata_host, metadata_port))
				 {
					if(metadata_client_->get_thrift_client()==nullptr)
						throw std::runtime_error("no thrift client");
					
					hive::persistent::AWSCredentials creds(AWS_ACCESS_KEY_DEFAULT, AWS_SECRET_KEY_DEFAULT);
					hive::persistent::S3ClientConfig config { AWS_S3_REGION_DEFAULT };
					s3_client_ = std::make_shared<hive::persistent::S3Client>(creds, config);
					s3_bucket_ = AWS_S3_BUCKET_DEFAULT;
				 }

void hive_client::put_lf(const std::string &obj_name, const std::string &object, int64_t obj_lifetime)
{
	auto m_start = time_utils::now_us();
	inode info = metadata_client_->put_lf(obj_name, object.size(), obj_lifetime);
	auto m_end = time_utils::now_us();
	int64_t offset = 0;
	for (auto &slice : info.slices_) {
		auto slice_id = slice_utils::parse(slice);
#ifdef ENABLE_CACHE_CLIENT
		if (!client_cache_.find_fn(slice_utils::addr(slice), [&](std::shared_ptr<storage_management_client> &cached_client){
			cached_client->put(object.substr(offset, slice_id.size), slice_id.id, slice_id.offset, slice_id.size);
		})) {
#endif // ENABLE_CACHE_CLIENT
			auto storage_client = std::make_shared<storage_management_client>(slice_id.host, slice_id.service_port);
			storage_client->put(object.substr(offset, slice_id.size), slice_id.id, slice_id.offset, slice_id.size);
#ifdef ENABLE_CACHE_CLIENT
			client_cache_.insert(slice_utils::addr(slice), storage_client);
		}
#endif // ENABLE_CACHE_CLIENT
		offset += slice_id.size;
	}
	auto s_end = time_utils::now_us();
	spdlog::trace("hive_client: put_lf: metadata({}) + storage({})", m_end - m_start, s_end - m_end);
}

void hive_client::put(const std::string &obj_name, const std::string &object)
{
	auto m_start = time_utils::now_us();
	inode info = metadata_client_->put(obj_name, object.size());
	auto m_end = time_utils::now_us();
	if (info.lifetime_ == 0) {
		s3_client_->put_object(s3_bucket_, obj_name, object);
		return;
	}
	int64_t offset = 0;
	for (auto &slice : info.slices_) {
		auto slice_id = slice_utils::parse(slice);
#ifdef ENABLE_CACHE_CLIENT
		if (!client_cache_.find_fn(slice_utils::addr(slice), [&](std::shared_ptr<storage_management_client> &cached_client){
			cached_client->put(object.substr(offset, slice_id.size), slice_id.id, slice_id.offset, slice_id.size);
		})) {
#endif // ENABLE_CACHE_CLIENT
			auto storage_client = std::make_shared<storage_management_client>(slice_id.host, slice_id.service_port);
			storage_client->put(object.substr(offset, slice_id.size), slice_id.id, slice_id.offset, slice_id.size);
#ifdef ENABLE_CACHE_CLIENT
			client_cache_.insert(slice_utils::addr(slice), storage_client);
		}
#endif // ENABLE_CACHE_CLIENT
		offset += slice_id.size;
	}
	auto s_end = time_utils::now_us();
	spdlog::trace("hive_client: put: metadata({}) + storage({})", m_end - m_start, s_end - m_end);
}

std::string hive_client::get(const std::string &obj_name)
{
	std::string ss;
	auto m_start = time_utils::now_us();
	inode info = metadata_client_->get(obj_name);
	auto m_end = time_utils::now_us();
	if (info.lifetime_ == 0) {
		ss = s3_client_->get_object(s3_bucket_, obj_name);
		return ss;
	}
	for (auto &slice : info.slices_) {
		auto slice_id = slice_utils::parse(slice);
#ifdef ENABLE_CACHE_CLIENT
		if (!client_cache_.find_fn(slice_utils::addr(slice), [&](std::shared_ptr<storage_management_client> &cached_client){
			ss += cached_client->get(slice_id.id, slice_id.offset, slice_id.size);
		})) {
#endif // ENABLE_CACHE_CLIENT
			auto storage_client = std::make_shared<storage_management_client>(slice_id.host, slice_id.service_port);
			ss += storage_client->get(slice_id.id, slice_id.offset, slice_id.size);
#ifdef ENABLE_CACHE_CLIENT
			client_cache_.insert(slice_utils::addr(slice), storage_client);
		}
#endif // ENABLE_CACHE_CLIENT
	}
	auto s_end = time_utils::now_us();
	spdlog::trace("hive_client: get: metadata({}) + storage({})", m_end - m_start, s_end - m_end);
	return ss;
}

void hive_client::remove(const std::string &obj_name)
{
	auto m_start = time_utils::now_us();
	metadata_client_->remove(obj_name);
	auto m_end = time_utils::now_us();
	spdlog::trace("hive_client: remove: metadata({}) + storage({})", m_end - m_start, 0);
}

void hive_client::async_put(const std::string &obj_name, const std::string &object)
{
	auto m_start = time_utils::now_us();
	inode info = metadata_client_->put(obj_name, object.size());
	auto m_end = time_utils::now_us();

	int64_t offset = 0;
	auto slice_futures = std::make_shared<std::vector<slice_future>>();
	int32_t idx = 0;

	// parallel send put() request
	for (auto &slice : info.slices_) {
		auto slice_id = slice_utils::parse(slice);
		slice_seq seq;
		seq.inode_name = obj_name;
		seq.inode_idx = idx;

		std::shared_ptr<storage_management_client> storage_client;
#ifdef ENABLE_CACHE_CLIENT
		if (!client_cache_.find_fn(slice_utils::addr(slice), [&](std::shared_ptr<storage_management_client> &cached_client) {
			storage_client = cached_client;
		})) {
			storage_client = std::make_shared<storage_management_client>(slice_id.host, slice_id.service_port);
			client_cache_.insert(slice_utils::addr(slice), storage_client);
		}
#else		
		storage_client = std::make_shared<storage_management_client>(slice_id.host, slice_id.service_port);
#endif // ENABLE_CACHE_CLIENT
		storage_client->send_async_put(seq, object.substr(offset, slice_id.size), slice_id.id, slice_id.offset, slice_id.size);
		slice_futures->push_back(slice_future(storage_client));

		offset += slice_id.size;	
		idx++;
	}

	// recv put() response
	for (auto &slice_future : *slice_futures) {
		async_response ret = slice_future.client_->recv_async_put();
	}

	auto s_end = time_utils::now_us();
	spdlog::trace("hive_client: async_put: metadata({}) + storage({})", m_end - m_start, s_end - m_end);
}

std::string hive_client::async_get(const std::string &obj_name)
{
	std::string ss;
	auto m_start = time_utils::now_us();
	inode info = metadata_client_->get(obj_name);
	auto m_end = time_utils::now_us();

	auto slice_futures = std::make_shared<std::vector<slice_future>>();
	int32_t idx = 0;

	// parallel send get() request
	for (auto &slice : info.slices_) {
		auto slice_id = slice_utils::parse(slice);
		slice_seq seq;
		seq.inode_name = obj_name;
		seq.inode_idx = idx;

		std::shared_ptr<storage_management_client> storage_client;
#ifdef ENABLE_CACHE_CLIENT
		if (!client_cache_.find_fn(slice_utils::addr(slice), [&](std::shared_ptr<storage_management_client> &cached_client) {
			storage_client = cached_client;
		})) {
			storage_client = std::make_shared<storage_management_client>(slice_id.host, slice_id.service_port);
			client_cache_.insert(slice_utils::addr(slice), storage_client);
		}
#else		
		storage_client = std::make_shared<storage_management_client>(slice_id.host, slice_id.service_port);
#endif // ENABLE_CACHE_CLIENT
		storage_client->send_async_get(seq, slice_id.id, slice_id.offset, slice_id.size);
		slice_futures->push_back(slice_future(storage_client));
		
		idx++;
	}

	// recv get() response
	for (auto &slice_future : *slice_futures) {
		async_response ret = slice_future.client_->recv_async_get();
		slice_futures->at(ret.inode_idx).content_ = std::move(ret.content);
	}

	// concat all slices
	for (auto &slice_future : *slice_futures) {
		ss += std::move(slice_future.content_);
	}

	auto s_end = time_utils::now_us();
	spdlog::trace("hive_client: async_get: metadata({}) + storage({})", m_end - m_start, s_end - m_end);
	return ss;
}
		
void hive_client::vput(const std::vector<std::string> &obj_names, const std::vector<std::string> &objects)
{
	std::unordered_map<std::string, std::shared_ptr<std::vector<slice_future>>> slice_futures_map;

	// send all inode lookup requests
	for (size_t i = 0; i < obj_names.size(); i++) {
		metadata_client_->send_put(obj_names[i], objects[i].size());
	}

	// send all slices' async_put() request for all inodes
	for (size_t i = 0; i < obj_names.size(); i++) {
		inode ino = metadata_client_->recv_put();
		slice_futures_map.insert(std::make_pair(ino.name_, std::make_shared<std::vector<slice_future>>()));
		// assume recv metadata response sequence is the same as send request sequence
		// assert(ino.name_ == obj_names[i]);
		const std::string &object = objects[i];
		int32_t idx = 0;
		int64_t offset = 0;

		// send all slices' async_put() request for an inode
		for (auto &slice : ino.slices_) {
			auto slice_id = slice_utils::parse(slice);
			slice_seq seq;
			seq.inode_name = ino.name_;
			seq.inode_idx = idx;

			std::shared_ptr<storage_management_client> storage_client;
#ifdef ENABLE_CACHE_CLIENT
			if (!client_cache_.find_fn(slice_utils::addr(slice), [&](std::shared_ptr<storage_management_client> &cached_client) {
				storage_client = cached_client;
			})) {
				storage_client = std::make_shared<storage_management_client>(slice_id.host, slice_id.service_port);
				client_cache_.insert(slice_utils::addr(slice), storage_client);
			}
#else		
			storage_client = std::make_shared<storage_management_client>(slice_id.host, slice_id.service_port);
#endif // ENABLE_CACHE_CLIENT
			storage_client->send_async_put(seq, object.substr(offset, slice_id.size), slice_id.id, slice_id.offset, slice_id.size);
			auto iter = slice_futures_map.find(ino.name_);
			iter->second->push_back(slice_future(storage_client));

			offset += slice_id.size;
			idx++;
		} // send all slices' async_put() request for an inode
	} // send all slices' async_put() request for all inodes

	// recv all slices' async_put() request for all inodes
	for (auto &obj_name : obj_names) {
		auto slice_futures = slice_futures_map.find(obj_name);
		for (auto &slice_f : *slice_futures->second) {
			async_response ret = slice_f.client_->recv_async_put();
		}
	}
}

std::vector<std::string> hive_client::vget(const std::vector<std::string> &obj_names)
{
	std::vector<std::string> objects;
	std::unordered_map<std::string, std::shared_ptr<std::vector<slice_future>>> slice_futures_map;

	// send all inode lookup requests
	for (auto &obj_name : obj_names) {
		metadata_client_->send_get(obj_name);
	}

	// send all slices' async_get() request for all inodes
	for (auto &obj_name : obj_names) {
		inode ino = metadata_client_->recv_get();
		slice_futures_map.insert(std::make_pair(ino.name_, std::make_shared<std::vector<slice_future>>()));
		int32_t idx = 0;

		// send all slices' async_get() request for an inode
		for (auto &slice : ino.slices_) {
			auto slice_id = slice_utils::parse(slice);
			slice_seq seq;
			seq.inode_name = ino.name_;
			seq.inode_idx = idx;

			std::shared_ptr<storage_management_client> storage_client;
#ifdef ENABLE_CACHE_CLIENT
			if (!client_cache_.find_fn(slice_utils::addr(slice), [&](std::shared_ptr<storage_management_client> &cached_client) {
				storage_client = cached_client;
			})) {
				storage_client = std::make_shared<storage_management_client>(slice_id.host, slice_id.service_port);
				client_cache_.insert(slice_utils::addr(slice), storage_client);
			}
#else		
			storage_client = std::make_shared<storage_management_client>(slice_id.host, slice_id.service_port);
#endif // ENABLE_CACHE_CLIENT
			storage_client->send_async_get(seq, slice_id.id, slice_id.offset, slice_id.size);
			auto iter = slice_futures_map.find(ino.name_);
			iter->second->push_back(slice_future(storage_client));
			
			idx++;
		} // send all slices' async_get() request for an inode
	} // send all slices' async_get() request for all inodes

	// recv all slices' async_get() request for all inodes
	for (auto &obj_name : obj_names) {
		auto slice_futures = slice_futures_map.find(obj_name);
		for (auto &slice_f : *slice_futures->second) {
			async_response ret = slice_f.client_->recv_async_get();
			auto iter = slice_futures_map.find(ret.inode_name);
			iter->second->at(ret.inode_idx).content_ = std::move(ret.content);
		}
	}

	// concat all slices for each inode
	for (auto &obj_name: obj_names) {
		std::string ss;
		auto slice_futures = slice_futures_map.find(obj_name);
		for (auto &slice_f : *slice_futures->second) {
			ss += std::move(slice_f.content_);
		}
		objects.push_back(std::move(ss));
	}
	return objects;
}

void hive_client::persist_setup(const std::string &access_key, const std::string &secret_key,
																const std::string &region, const std::string &bucket)
{
	hive::persistent::AWSCredentials creds(access_key, secret_key);
	hive::persistent::S3ClientConfig config { region };
	s3_client_ = std::make_shared<hive::persistent::S3Client>(creds, config);
	s3_bucket_ = bucket;
}

void hive_client::persist_put(const std::string &obj_name, const std::string &object)
{
	s3_client_->put_object(s3_bucket_, obj_name, object);
}

std::string hive_client::persist_get(const std::string &obj_name)
{
	return s3_client_->get_object(s3_bucket_, obj_name);
}

void hive_client::hdag_put(const std::string &workflow_name, const std::string &function_name,
														const std::string &obj_name, const std::string &object)
{
	auto m_start = time_utils::now_us();
	inode info = metadata_client_->hdag_put(workflow_name, function_name, obj_name, object.size());
	auto m_end = time_utils::now_us();
	if (info.lifetime_ == 0) {
		s3_client_->put_object(s3_bucket_, obj_name, object);
		return;
	}
	int64_t offset = 0;
	for (auto &slice : info.slices_) {
		auto slice_id = slice_utils::parse(slice);
#ifdef ENABLE_CACHE_CLIENT
		if (!client_cache_.find_fn(slice_utils::addr(slice), [&](std::shared_ptr<storage_management_client> &cached_client){
			cached_client->put(object.substr(offset, slice_id.size), slice_id.id, slice_id.offset, slice_id.size);
		})) {
#endif // ENABLE_CACHE_CLIENT
			auto storage_client = std::make_shared<storage_management_client>(slice_id.host, slice_id.service_port);
			storage_client->put(object.substr(offset, slice_id.size), slice_id.id, slice_id.offset, slice_id.size);
#ifdef ENABLE_CACHE_CLIENT
			client_cache_.insert(slice_utils::addr(slice), storage_client);
		}
#endif // ENABLE_CACHE_CLIENT
		offset += slice_id.size;
	}
	auto s_end = time_utils::now_us();
	spdlog::trace("hive_client: hdag_put: metadata({}) + storage({})", m_end - m_start, s_end - m_end);
}

std::string hive_client::hdag_get(const std::string &workflow_name, const std::string &function_name,
																	const std::string &obj_name)
{
	std::string ss;
	auto m_start = time_utils::now_us();
	inode info = metadata_client_->hdag_get(workflow_name, function_name, obj_name);
	auto m_end = time_utils::now_us();
	if (info.lifetime_ == 0) {
		ss = s3_client_->get_object(s3_bucket_, obj_name);
		return ss;
	}
	for (auto &slice : info.slices_) {
		auto slice_id = slice_utils::parse(slice);
#ifdef ENABLE_CACHE_CLIENT
		if (!client_cache_.find_fn(slice_utils::addr(slice), [&](std::shared_ptr<storage_management_client> &cached_client){
			ss += cached_client->get(slice_id.id, slice_id.offset, slice_id.size);
		})) {
#endif // ENABLE_CACHE_CLIENT
			auto storage_client = std::make_shared<storage_management_client>(slice_id.host, slice_id.service_port);
			ss += storage_client->get(slice_id.id, slice_id.offset, slice_id.size);
#ifdef ENABLE_CACHE_CLIENT
			client_cache_.insert(slice_utils::addr(slice), storage_client);
		}
#endif // ENABLE_CACHE_CLIENT
	}
	auto s_end = time_utils::now_us();
	spdlog::trace("hive_client: hdag_get: metadata({}) + storage({})", m_end - m_start, s_end - m_end);
	return ss;
}

void hive_client::hdag_async_put(const std::string &workflow_name, const std::string &function_name,
																	const std::string &obj_name, const std::string &object)
{
	auto m_start = time_utils::now_us();
	inode info = metadata_client_->hdag_put(workflow_name, function_name, obj_name, object.size());
	auto m_end = time_utils::now_us();

	int64_t offset = 0;
	auto slice_futures = std::make_shared<std::vector<slice_future>>();
	int32_t idx = 0;

	// parallel send put() request
	for (auto &slice : info.slices_) {
		auto slice_id = slice_utils::parse(slice);
		slice_seq seq;
		seq.inode_name = obj_name;
		seq.inode_idx = idx;

		std::shared_ptr<storage_management_client> storage_client;
#ifdef ENABLE_CACHE_CLIENT
		if (!client_cache_.find_fn(slice_utils::addr(slice), [&](std::shared_ptr<storage_management_client> &cached_client) {
			storage_client = cached_client;
		})) {
			storage_client = std::make_shared<storage_management_client>(slice_id.host, slice_id.service_port);
			client_cache_.insert(slice_utils::addr(slice), storage_client);
		}
#else		
		storage_client = std::make_shared<storage_management_client>(slice_id.host, slice_id.service_port);
#endif // ENABLE_CACHE_CLIENT
		storage_client->send_async_put(seq, object.substr(offset, slice_id.size), slice_id.id, slice_id.offset, slice_id.size);
		slice_futures->push_back(slice_future(storage_client));

		offset += slice_id.size;	
		idx++;
	}

	// recv put() response
	for (auto &slice_future : *slice_futures) {
		async_response ret = slice_future.client_->recv_async_put();
	}

	auto s_end = time_utils::now_us();
	spdlog::trace("hive_client: hdag_async_put: metadata({}) + storage({})", m_end - m_start, s_end - m_end);
}

std::string hive_client::hdag_async_get(const std::string &workflow_name, const std::string &function_name, const std::string &obj_name)
{
	std::string ss;
	auto m_start = time_utils::now_us();
	inode info = metadata_client_->hdag_get(workflow_name, function_name, obj_name);
	auto m_end = time_utils::now_us();

	auto slice_futures = std::make_shared<std::vector<slice_future>>();
	int32_t idx = 0;

	// parallel send get() request
	for (auto &slice : info.slices_) {
		auto slice_id = slice_utils::parse(slice);
		slice_seq seq;
		seq.inode_name = obj_name;
		seq.inode_idx = idx;

		std::shared_ptr<storage_management_client> storage_client;
#ifdef ENABLE_CACHE_CLIENT
		if (!client_cache_.find_fn(slice_utils::addr(slice), [&](std::shared_ptr<storage_management_client> &cached_client) {
			storage_client = cached_client;
		})) {
			storage_client = std::make_shared<storage_management_client>(slice_id.host, slice_id.service_port);
			client_cache_.insert(slice_utils::addr(slice), storage_client);
		}
#else		
		storage_client = std::make_shared<storage_management_client>(slice_id.host, slice_id.service_port);
#endif // ENABLE_CACHE_CLIENT
		storage_client->send_async_get(seq, slice_id.id, slice_id.offset, slice_id.size);
		slice_futures->push_back(slice_future(storage_client));
		
		idx++;
	}

	// recv get() response
	for (auto &slice_future : *slice_futures) {
		async_response ret = slice_future.client_->recv_async_get();
		slice_futures->at(ret.inode_idx).content_ = std::move(ret.content);
	}

	// concat all slices
	for (auto &slice_future : *slice_futures) {
		ss += std::move(slice_future.content_);
	}

	auto s_end = time_utils::now_us();
	spdlog::trace("hive_client: hdag_async_get: metadata({}) + storage({})", m_end - m_start, s_end - m_end);
	return ss;
}

void hive_client::hdag_vput(const std::string &workflow_name, const std::string &function_name, const std::vector<std::string> &obj_names, const std::vector<std::string> &objects)
{
	std::unordered_map<std::string, std::shared_ptr<std::vector<slice_future>>> slice_futures_map;

	// send all inode lookup requests
	for (size_t i = 0; i < obj_names.size(); i++) {
		metadata_client_->send_hdag_put(workflow_name, function_name, obj_names[i], objects[i].size());
	}

	// send all slices' async_put() request for all inodes
	for (size_t i = 0; i < obj_names.size(); i++) {
		inode ino = metadata_client_->recv_hdag_put();
		slice_futures_map.insert(std::make_pair(ino.name_, std::make_shared<std::vector<slice_future>>()));
		// assume recv metadata response sequence is the same as send request sequence
		// assert(ino.name_ == obj_names[i]);
		const std::string &object = objects[i];
		int32_t idx = 0;
		int64_t offset = 0;

		// send all slices' async_put() request for an inode
		for (auto &slice : ino.slices_) {
			auto slice_id = slice_utils::parse(slice);
			slice_seq seq;
			seq.inode_name = ino.name_;
			seq.inode_idx = idx;

			std::shared_ptr<storage_management_client> storage_client;
#ifdef ENABLE_CACHE_CLIENT
			if (!client_cache_.find_fn(slice_utils::addr(slice), [&](std::shared_ptr<storage_management_client> &cached_client) {
				storage_client = cached_client;
			})) {
				storage_client = std::make_shared<storage_management_client>(slice_id.host, slice_id.service_port);
				client_cache_.insert(slice_utils::addr(slice), storage_client);
			}
#else		
			storage_client = std::make_shared<storage_management_client>(slice_id.host, slice_id.service_port);
#endif // ENABLE_CACHE_CLIENT
			storage_client->send_async_put(seq, object.substr(offset, slice_id.size), slice_id.id, slice_id.offset, slice_id.size);
			auto iter = slice_futures_map.find(ino.name_);
			iter->second->push_back(slice_future(storage_client));

			offset += slice_id.size;
			idx++;
		} // send all slices' async_put() request for an inode
	} // send all slices' async_put() request for all inodes

	// recv all slices' async_put() request for all inodes
	for (auto &obj_name : obj_names) {
		auto slice_futures = slice_futures_map.find(obj_name);
		for (auto &slice_f : *slice_futures->second) {
			async_response ret = slice_f.client_->recv_async_put();
		}
	}
}

std::vector<std::string> hive_client::hdag_vget(const std::string &workflow_name, const std::string &function_name, const std::vector<std::string> &obj_names)
{
	std::vector<std::string> objects;
	std::unordered_map<std::string, std::shared_ptr<std::vector<slice_future>>> slice_futures_map;

	// send all inode lookup requests
	for (auto &obj_name : obj_names) {
		metadata_client_->send_hdag_get(workflow_name, function_name, obj_name);
	}

	// send all slices' async_get() request for all inodes
	for (auto &obj_name : obj_names) {
		inode ino = metadata_client_->recv_hdag_get();
		slice_futures_map.insert(std::make_pair(ino.name_, std::make_shared<std::vector<slice_future>>()));
		int32_t idx = 0;

		// send all slices' async_get() request for an inode
		for (auto &slice : ino.slices_) {
			auto slice_id = slice_utils::parse(slice);
			slice_seq seq;
			seq.inode_name = ino.name_;
			seq.inode_idx = idx;

			std::shared_ptr<storage_management_client> storage_client;
#ifdef ENABLE_CACHE_CLIENT
			if (!client_cache_.find_fn(slice_utils::addr(slice), [&](std::shared_ptr<storage_management_client> &cached_client) {
				storage_client = cached_client;
			})) {
				storage_client = std::make_shared<storage_management_client>(slice_id.host, slice_id.service_port);
				client_cache_.insert(slice_utils::addr(slice), storage_client);
			}
#else		
			storage_client = std::make_shared<storage_management_client>(slice_id.host, slice_id.service_port);
#endif // ENABLE_CACHE_CLIENT
			storage_client->send_async_get(seq, slice_id.id, slice_id.offset, slice_id.size);
			auto iter = slice_futures_map.find(ino.name_);
			iter->second->push_back(slice_future(storage_client));
			
			idx++;
		} // send all slices' async_get() request for an inode
	} // send all slices' async_get() request for all inodes

	// recv all slices' async_get() request for all inodes
	for (auto &obj_name : obj_names) {
		auto slice_futures = slice_futures_map.find(obj_name);
		for (auto &slice_f : *slice_futures->second) {
			async_response ret = slice_f.client_->recv_async_get();
			auto iter = slice_futures_map.find(ret.inode_name);
			iter->second->at(ret.inode_idx).content_ = std::move(ret.content);
		}
	}

	// concat all slices for each inode
	for (auto &obj_name: obj_names) {
		std::string ss;
		auto slice_futures = slice_futures_map.find(obj_name);
		for (auto &slice_f : *slice_futures->second) {
			ss += std::move(slice_f.content_);
		}
		objects.push_back(std::move(ss));
	}
	return objects;
}

}
}