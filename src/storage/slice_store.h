#ifndef HIVE_SLICE_STORE_H
#define HIVE_SLICE_STORE_H

#include <string>

namespace hive {
namespace storage {

class slice_store {
	public:
		slice_store() = delete;
		slice_store(uint64_t capacity, int32_t blocksize);
		~slice_store();

		bool init();
		void destroy();
		void put(const std::string &object, int32_t block_id, int32_t offset, int32_t size);
		std::string get(int32_t block_id, int32_t offset, int32_t size);
	
	private:
		uint64_t capacity_;
		int32_t blocksize_;
		char *addr_;
};

}
}

#endif // HIVE_SLICE_STORE_H