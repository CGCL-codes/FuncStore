#ifndef HIVE_CLIENT_H
#define HIVE_CLIENT_H

#include "metadata/blob/inode_management_client.h"
#include "storage/storage_management_client.h"
#include "libcuckoo/cuckoohash_map.hh"

#include "net/s3.hh"

#define AWS_ACCESS_KEY_DEFAULT "default-access-key"
#define AWS_SECRET_KEY_DEFAULT "default-secret-key"
#define AWS_S3_REGION_DEFAULT "default-region"
#define AWS_S3_BUCKET_DEFAULT "default-bucket"

// #define ENABLE_CACHE_CLIENT

namespace hive {
namespace client {

using namespace ::hive::storage;

struct slice_future {
	slice_future() = default;
	slice_future(std::shared_ptr<storage_management_client> client) : client_(client) {}
	~slice_future() = default;

	std::shared_ptr<storage_management_client> client_;
	std::string content_;
};

class hive_client {
	public:
		hive_client() = delete;
		hive_client(const std::string &metadata_host, int metadata_port);
		~hive_client() = default;

		/**
		 * @brief Put an object with known lifetime
		 * @param obj_name Object name
		 * @param object Object content
		 * @param obj_lifetime Known object lifetime
		*/
		void put_lf(const std::string &obj_name, const std::string &object, int64_t obj_lifetime);

		/**
		 * @brief Put an object
		 * @param obj_name Object name
		 * @param object Object content
		*/
		void put(const std::string &obj_name, const std::string &object);

		/**
		 * @brief Get an object
		 * @param obj_name Object name
		 * @return Object content
		*/
		std::string get(const std::string &obj_name);

		/**
		 * @brief Remove an object
		 * @param obj_name Object name
		*/
		void remove(const std::string &obj_name);

		/**
		 * @brief Put an object with non-blocking send/recv APIs
		 * @param obj_name Object name
		 * @param object Object content
		*/
		void async_put(const std::string &obj_name, const std::string &object);
		
		/**
		 * @brief Get an object with non-blocking send/recv APIs
		 * @param obj_name Object name
		 * @return Object content
		*/
		std::string async_get(const std::string &obj_name);
		
		/**
		 * @brief Put objects
		 * @param obj_names Object names
		 * @param objects Objects correspond to obj_names
		*/
		void vput(const std::vector<std::string> &obj_names, const std::vector<std::string> &objects);

		/**
		 * @brief Get objects
		 * @param obj_names Object names
		 * @return Objects correspond to obj_names
		*/
		std::vector<std::string> vget(const std::vector<std::string> &obj_names);

		/**
		 * @brief Setup AWS S3
		 * @param access_key AWS access key for credential
		 * @param secret_key AWS secret key for credential
		 * @param region AWS region for S3 config
		 * @param bucket AWS S3 bucket name
		*/
		void persist_setup(const std::string &access_key, const std::string &secret_key,
												const std::string &region = AWS_S3_REGION_DEFAULT, const std::string &bucket = AWS_S3_BUCKET_DEFAULT);
		
		/**
		 * @brief Put an object into AWS S3
		 * @param obj_name Object name
		 * @param object Object content
		*/
		void persist_put(const std::string &obj_name, const std::string &object);
		
		/**
		 * @brief Get an object from AWS S3
		 * @param obj_name Object name
		 * @return Object content
		*/
		std::string persist_get(const std::string &obj_name);

		/**
		 * @brief Put an object for workflow
		 * @param workflow_name Invoker workflow name
		 * @param function_name Invoker function name
		 * @param obj_name Object name
		 * @param object Object content
		*/
		void hdag_put(const std::string &workflow_name, const std::string &function_name, const std::string &obj_name, const std::string &object);
		
		/**
		 * @brief Get an object for workflow
		 * @param workflow_name Invoker workflow name
		 * @param function_name Invoker function name
		 * @param obj_name Object name
		 * @return Object content
		*/
		std::string hdag_get(const std::string &workflow_name, const std::string &function_name, const std::string &obj_name);
	
		/**
		 * @brief Put an object for workflow with non-blocking send/recv APIs
		 * @param workflow_name Invoker workflow name
		 * @param function_name Invoker function name
		 * @param obj_name Object name
		 * @param object Object content
		*/
		void hdag_async_put(const std::string &workflow_name, const std::string &function_name, const std::string &obj_name, const std::string &object);
		
		/**
		 * @brief Get an object for workflow with non-blocking send/recv APIs
		 * @param workflow_name Invoker workflow name
		 * @param function_name Invoker function name
		 * @param obj_name Object name
		 * @return Object content
		*/
		std::string hdag_async_get(const std::string &workflow_name, const std::string &function_name, const std::string &obj_name);
	
		/**
		 * @brief Put objects for workflow with send/recv pipeline
		 * @param workflow_name Invoker workflow name
		 * @param function_name Invoker function name
		 * @param obj_names Object names
		 * @param objects Objects correspond to obj_names
		*/
		void hdag_vput(const std::string &workflow_name, const std::string &function_name, const std::vector<std::string> &obj_names, const std::vector<std::string> &objects);

		/**
		 * @brief Get objects for workflow with send/recv pipeline
		 * @param workflow_name Invoker workflow name
		 * @param function_name Invoker function name
		 * @param obj_names Object names
		 * @return Objects correspond to obj_names
		*/
		std::vector<std::string> hdag_vget(const std::string &workflow_name, const std::string &function_name, const std::vector<std::string> &obj_names);

		std::shared_ptr<hive::metadata::inode_management_client>& get_metadata_client() { return metadata_client_; }
	private:
#ifdef ENABLE_CACHE_CLIENT
		libcuckoo::cuckoohash_map<std::string, std::shared_ptr<storage::storage_management_client>> client_cache_;
#endif // ENABLE_CACHE_CLIENT
		std::shared_ptr<hive::metadata::inode_management_client> metadata_client_;
		std::shared_ptr<hive::persistent::S3Client> s3_client_;
		std::string s3_bucket_;
};

}
}
#endif // HIVE_CLIENT_H