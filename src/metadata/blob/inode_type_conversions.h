#ifndef HIVE_INODE_TYPE_CONVERSIONS_H
#define HIVE_INODE_TYPE_CONVERSIONS_H

#include "inode_management_service_types.h"
#include "utils/inode_utils.h"

namespace hive {
namespace metadata {

class inode_type_conversions {
	public:
		static rpc_inode to_rpc(const utils::inode &info)
		{
			rpc_inode rpc_info;
			rpc_info.name = std::move(info.name_);
			rpc_info.slices = std::move(info.slices_);
			rpc_info.lifetime = info.lifetime_;
			return rpc_info;
		}

		static utils::inode from_rpc(const rpc_inode &rpc)
		{
			utils::inode info(rpc.name, rpc.slices, rpc.lifetime);
			return info;
		}
};

}
}

#endif // HIVE_INODE_TYPE_CONVERSIONS_H