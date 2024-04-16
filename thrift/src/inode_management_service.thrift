namespace cpp hive.metadata

struct rpc_inode {
	1: required string name,
	2: required list<string> slices,
	3: required i64 lifetime
}

service inode_management_service {
	// Allocate an inode for object
	rpc_inode put_lf(1: string obj_name, 2: i64 obj_size, 3: i64 obj_lifetime),

	// Allocate an inode for object
	rpc_inode put(1: string obj_name, 2: i64 obj_size),

	// Lookup an inode for object
	rpc_inode get(1: string obj_name),

	// Remove an inode
	void remove(1: string obj_name),

	// Allocate an inode for object in specific workflow and function
	rpc_inode hdag_put(1: string workflow_name, 2: string function_name, 3: string obj_name, 4: i64 obj_size),

	// Get an inode for object in specific workflow and function
	rpc_inode hdag_get(1: string workflow_name, 2: string function_name, 3: string obj_name),
}