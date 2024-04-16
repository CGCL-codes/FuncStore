#ifndef HIVE_INODE_UTILS_H
#define HIVE_INODE_UTILS_H

#include <string>
#include <vector>

namespace hive {
namespace utils {

class inode {
	public:
		inode() = default;
		inode(const std::string &name, std::vector<std::string> slices, uint64_t lifetime) : name_(name), slices_(std::move(slices)), lifetime_(lifetime) {}
		~inode() = default;

		std::string name_;
		std::vector<std::string> slices_;
		uint64_t lifetime_;
};

}
}

#endif // HIVE_INODE_UTILS_H