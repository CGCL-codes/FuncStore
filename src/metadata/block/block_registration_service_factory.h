#ifndef HIVE_BLOCK_REGISTRATION_SERVICE_FACTORY_H
#define HIVE_BLOCK_REGISTRATION_SERVICE_FACTORY_H

#include "block_registration_service.h"
#include "block_allocator.h"

namespace hive {
namespace metadata {

/* Block registration service factory class */
class block_registration_service_factory : public block_registration_serviceIfFactory {
	public:

  /**
   * @brief Constructor
   * @param alloc Block allocator
	*/
  explicit block_registration_service_factory(std::shared_ptr<block_allocator> alloc);
	
	private:

  /**
   * @brief Get block allocation service handler
   * @param conn_info Connection information
   * @return Block allocation handler
  */
  block_registration_serviceIf *getHandler(const ::apache::thrift::TConnectionInfo &connInfo) override;

  /**
   * @brief Release handler
   * @param handler Block allocation handler
  */
  void releaseHandler(block_registration_serviceIf *anIf) override;

  /* Block allocator */
  std::shared_ptr<block_allocator> alloc_;
};

}
}

#endif // HIVE_BLOCK_REGISTRATION_SERVICE_FACTORY_H