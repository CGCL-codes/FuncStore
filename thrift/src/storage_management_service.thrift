namespace cpp hive.storage

struct slice_seq {
	1: string inode_name,
	2: i32 inode_idx
}

struct async_response {
	1: string inode_name,
	2: i32 inode_idx,
	3: string content
}

service storage_management_service {
	// Put a slice to storage server
	void put(1: string object, 2: i32 block_id, 3: i32 offset, 4: i32 size),

	// Get a slice from storage server
	string get(1: i32 block_id, 2: i32 offset, 3: i32 size),

	// Put a slice to storage server, for nonblocking APIs(send/recv)
	async_response async_put(1: slice_seq seq, 2: string object, 3: i32 block_id, 4: i32 offset, 5: i32 size),

	// Get a slice from storage server, for nonblocking APIs(send/recv)
	async_response async_get(1: slice_seq seq, 2: i32 block_id, 3: i32 offset, 4: i32 size),
}