#ifndef HIVE_SLICE_UTILS_H
#define HIVE_SLICE_UTILS_H

#include <string>
#include <tuple>
#include <sstream>

namespace hive {
namespace utils {

struct slice_id {
  std::string host;
  int32_t service_port;
  int32_t id;
	int32_t offset;
	int32_t size;
};

class slice_utils {
	public:
	/**
   * @brief Slice parser
   * @param name Slice name
   * @return Slice identifier structure
   */
  static slice_id parse(const std::string &name)
	{
		std::string host, service_port, block_id, offset, size;
  	try {
    	std::istringstream in(name);
    	std::getline(in, host, ':');
    	std::getline(in, service_port, ':');
    	std::getline(in, block_id, ':');
    	std::getline(in, offset, ':');
    	std::getline(in, size, ':');
    	return {host, std::stoi(service_port), std::stoi(block_id), std::stoi(offset), std::stoi(size)};
  	} catch (std::exception &e) {
    	throw std::invalid_argument("Could not parse slice name: " + name + "; " + e.what());
  	}
	}

  /**
   * @brief Make a slice name by merging all parts into a single string
   * @param host Hostname
   * @param service_port Service port number
   * @param id Slice identifier
	 * @param offset Offset of slice in bucket
	 * @param size Size of slice
   * @return Slice name
   */
  static std::string make(const std::string &host, int32_t service_port, int32_t id, int32_t offset, int32_t size)
	{
		return host + ":" + std::to_string(service_port) + ":" + std::to_string(id) + ":" + std::to_string(offset) + ":" + std::to_string(size);
	}

	/**
	 * @brief Make a slice name by merging all parts into a single string
   * @param bid Block name
	 * @param offset Offset of slice in bucket
	 * @param size Size of slice
   * @return Slice name
	*/
  static std::string make(const std::string &bid, int32_t offset, int32_t size)
	{
		return bid + ":" + std::to_string(offset) + ":" + std::to_string(size);
	}

	/**
	 * @brief Get block id from a slice
	 * @param slice Slice string id
	 * @return Block string id
	*/
	static std::string blockid(const std::string &slice)
	{
		return slice.substr(0, slice.substr(0, slice.find_last_of(':')).find_last_of(':'));
	}

	/**
	 * @brief Get slice size
	 * @param slice Slice string id
	 * @return Slice size
	*/
	static int32_t size(const std::string &slice)
	{
		return std::stoi(slice.substr(slice.find_last_of(':') + 1));
	}

	/**
	 * @brief Get slice address
	 * @param slice Slice string id
	 * @return Slice address
	*/
	static std::string addr(const std::string &slice)
	{
		return slice.substr(0, slice.find(':', slice.find(':') + 1));
	}

};

}
}
#endif // HIVE_SLICE_UTILS_H