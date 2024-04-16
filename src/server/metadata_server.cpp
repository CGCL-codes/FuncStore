#include <thread>
#include <vector>
#include <fstream>
#include "metadata/block/block_registration_server.h"
#include "metadata/block/block_registration_service_factory.h"
#include "metadata/block/block_registration_service_handler.h"
#include "metadata/block/random_block_allocator.h"
#include "metadata/block/block_usage_tracker.h"
#include "metadata/blob/blob_store.h"
#include "metadata/blob/inode_management_server.h"
#include "metadata/blob/inode_management_service_factory.h"
#include "metadata/blob/inode_management_service_handler.h"
#include "workflow/workflow_management_server.h"
#include "workflow/workflow_management_service_factory.h"
#include "workflow/workflow_management_service_handler.h"
#include "monitor/monitor.h"
#include "predictor/predictor.h"
#include "spdlog/spdlog.h"
#include "nlohmann/json.hpp"

using namespace ::hive::metadata;
using namespace ::hive::utils;
using namespace ::hive::monitor;
using namespace ::hive::predictor;
using namespace ::hive::workflow;

using namespace ::apache::thrift;
using json = nlohmann::json;

int main(int argc, char **argv)
{
	// spdlog::set_level(spdlog::level::trace);
	std::string host("127.0.0.1");
	int block_port = 9998;
	int inode_port = 9999;
	std::vector<uint64_t> invalid_tsv = {100, 1000, 10000};

	std::string model_host("127.0.0.1");
	int model_port = 9034;
	std::string datasets_path(".");

	int32_t blocksize = 64*1024;
	std::string usage_track_file("");
	uint64_t usage_track_period = 1000;

	int workflow_port = 10000;

	if (argc > 1) {
		auto conf_path = argv[1];
		std::ifstream f(conf_path);
		json conf = json::parse(f);

		host = conf["metadata"]["host"].get<std::string>();
		block_port = conf["metadata"]["block_port"].get<int>();
		inode_port = conf["metadata"]["inode_port"].get<int>();
		invalid_tsv = conf["metadata"]["lifetime"].get<std::vector<uint64_t>>();

		model_host = conf["model"]["host"].get<std::string>();
		model_port = conf["model"]["port"].get<int>();
		datasets_path = conf["model"]["datasets_path"].get<std::string>();

		blocksize = conf["block"]["size"].get<int>();
		usage_track_file = conf["block"]["usage_track_path"].get<std::string>();
		usage_track_period = conf["block"]["usage_track_period"].get<uint64_t>();

		workflow_port = conf["metadata"]["workflow_port"].get<int>();
	}

	auto block_alloc = std::make_shared<random_block_allocator>(blocksize);
	auto block_server = block_registration_server::create(block_alloc, host, block_port);
	std::thread block_server_thread([&block_server] {block_server->serve();});
	spdlog::info("Block registration server listening on {}:{}", host, block_port);

	auto bstore = std::make_shared<blob_store>(block_alloc, invalid_tsv);
	auto ptr = std::make_shared<predictor>(model_host, model_port, datasets_path);
	auto mtr = std::make_shared<monitor>(bstore, ptr);
	auto inode_server = inode_management_server::create(bstore, mtr, host, inode_port);
	std::thread inode_server_thread([&inode_server] {inode_server->serve();});
	spdlog::info("Inode management server listening on {}:{}", host, inode_port);

	auto workflow_server = workflow_management_server::create(mtr, host, workflow_port);
	std::thread workflow_server_thread([&workflow_server] {workflow_server->serve();});
	spdlog::info("Workflow management server listening on {}:{}", host, workflow_port);

	if (usage_track_file != "") {
		spdlog::info("Block usage tracker logging into {}", usage_track_file);
		block_usage_tracker tracker(block_alloc, usage_track_period, usage_track_file);
		tracker.start();
	}

	inode_server_thread.join();
	block_server_thread.join();

	return 0;
}