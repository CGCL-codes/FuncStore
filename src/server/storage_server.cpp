#include <string>
#include <thread>
#include <fstream>
#include "metadata/block/block_registration_client.h"
#include "storage/slice_store.h"
#include "storage/storage_management_server.h"
#include "storage/storage_management_server_nonblocking.h"
#include "utils/block_utils.h"
#include "spdlog/spdlog.h"
#include "nlohmann/json.hpp"

using namespace ::hive::metadata;
using namespace ::hive::storage;
using namespace ::hive::utils;

using namespace ::apache::thrift;
using json = nlohmann::json;

int main(int argc, char **argv)
{
	// spdlog::set_level(spdlog::level::trace);
	std::string host("127.0.0.1");
	int service_port = 6666;
	std::string metadata_host("127.0.0.1");
	int metadata_port = 9998;
	int32_t blocksize = 64*1024;
	int32_t num_blocks = 64;

	if (argc > 1) {
		auto conf_path = argv[1];
		std::ifstream f(conf_path);
		json conf = json::parse(f);

		metadata_host = conf["metadata"]["host"].get<std::string>();
		metadata_port = conf["metadata"]["block_port"].get<int>();
		host = conf["storage"]["host"].get<std::string>();
		service_port = conf["storage"]["port"].get<int>();
		num_blocks = conf["storage"]["num_blocks"].get<int>();
		blocksize = conf["block"]["size"].get<int>();
	}

	std::vector<std::string> blocks;
	for (int i = 0; i < num_blocks; i++) {
		blocks.emplace_back(block_utils::make(host, service_port, i));
	}

	block_registration_client client(metadata_host, metadata_port);
	client.register_blocks(blocks);

	uint64_t capacity = static_cast<uint64_t>(num_blocks) * static_cast<uint64_t>(blocksize);
	auto sstore = std::make_shared<slice_store>(capacity, blocksize);
	sstore->init();

#ifdef USE_NONBLOCKING_STORAGE_SERVER
	auto storage_server = storage_management_server_nb::create(sstore, host, service_port, std::thread::hardware_concurrency() / 2, std::thread::hardware_concurrency() / 2);
#else
	auto storage_server = storage_management_server::create(sstore, host, service_port);
#endif // USE_NONBLOCKING_STORAGE_SERVER
	std::thread storage_server_thread([&storage_server] {storage_server->serve();});
	spdlog::info("Storage management server listening on {}:{}", host, service_port);

	storage_server_thread.join();

	return 0;
}