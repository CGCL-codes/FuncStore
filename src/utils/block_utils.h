#ifndef HIVE_BLOCK_UTILS_H
#define HIVE_BLOCK_UTILS_H

#include <string>
#include <tuple>
#include <sstream>

namespace hive {
namespace utils {

struct block_id {
  std::string host;
  int32_t service_port;
  int32_t id;
};

class block_utils {
	public:
	/**
   * @brief Block name parser
   * @param name Block name
   * @return Block identifier structure
   */
  static block_id parse(const std::string &name)
	{
		std::string host, service_port, block_id;
  	try {
    	std::istringstream in(name);
    	std::getline(in, host, ':');
    	std::getline(in, service_port, ':');
    	std::getline(in, block_id, ':');
    	return {host, std::stoi(service_port), std::stoi(block_id)};
  	} catch (std::exception &e) {
    	throw std::invalid_argument("Could not parse block name: " + name + "; " + e.what());
  	}
	}

  /**
   * @brief Make a block name by merging all parts into a single string
   * @param host Hostname
   * @param service_port Service port number
   * @param id Block identifier
   * @return Block name
   */
  static std::string make(const std::string &host, int32_t service_port, int32_t id)
	{
		return host + ":" + std::to_string(service_port) + ":" + std::to_string(id);
	}
};

}
}
#endif // HIVE_BLOCK_UTILS_H