#ifndef HIVE_BLOB_STORE_H
#define HIVE_BLOB_STORE_H

#include "slice_pool.h"
#include "utils/inode_utils.h"
#include "metadata/block/block_allocator.h"
#include <map>

namespace hive {
namespace metadata {

using utils::inode;

class blob_store {
	public:
		blob_store() = delete;
		blob_store(std::shared_ptr<block_allocator> block_alloc, std::vector<uint64_t> invalid_tsv);
		~blob_store() = default;

		/**
		 * @brief Allocate space for object
		 * @param obj_name Object name
		 * @param obj_size Object size
		 * @param obj_lifetime Object lifetime
		 * @return Inode for indicating allocated space
		*/
		inode put(const std::string &obj_name, int64_t obj_size, uint64_t obj_lifetime);
		
		/**
		 * @brief Get allocated space for object
		 * @param obj_name Object name
		 * @return Inode for indicating allocated space
		*/
		inode get(const std::string &obj_name);
		
		/**
		 * @brief Remove object
		 * @param obj_name Object name
		*/
		void remove(const std::string &obj_name);

	private:
		std::shared_ptr<block_allocator> block_alloc_;
		std::map<uint64_t, std::shared_ptr<slice_pool>> slice_pools_;
		libcuckoo::cuckoohash_map<std::string, inode> index_table_;
		/* Mutex for remove() from @index_table_ */
		std::mutex mtx_;
};

}
}

#endif // HIVE_BLOB_STORE_H