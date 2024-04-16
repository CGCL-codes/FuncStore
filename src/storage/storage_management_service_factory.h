#ifndef HIVE_STORAGE_MANAGEMENT_SERVICE_FACTORY_H
#define HIVE_STORAGE_MANAGEMENT_SERVICE_FACTORY_H

#include "storage_management_service.h"
#include "slice_store.h"

namespace hive {
namespace storage {

class storage_management_service_factory : public storage_management_serviceIfFactory {
	public:

  /**
   * @brief Constructor
   * @param sstore Slice store
	*/
  explicit storage_management_service_factory(std::shared_ptr<slice_store> sstore);
	
	private:

  /**
   * @brief Get storage management service handler
   * @param conn_info Connection information
   * @return Storage management handler
  */
  storage_management_serviceIf *getHandler(const ::apache::thrift::TConnectionInfo &connInfo) override;

  /**
   * @brief Release handler
   * @param handler Storage management handler
  */
  void releaseHandler(storage_management_serviceIf *anIf) override;

  /* Slice store */
  std::shared_ptr<slice_store> sstore_;
};

}
}

#endif // HIVE_STORAGE_MANAGEMENT_SERVICE_FACTORY_H