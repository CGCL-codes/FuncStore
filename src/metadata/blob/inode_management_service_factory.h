#ifndef HIVE_INODE_MANAGEMENT_SERVICE_FACTORY_H
#define HIVE_INODE_MANAGEMENT_SERVICE_FACTORY_H

#include "inode_management_service.h"
#include "blob_store.h"
#include "monitor/monitor.h"
#include "predictor/predictor.h"

namespace hive {
namespace metadata {

class inode_management_service_factory : public inode_management_serviceIfFactory {
	public:

  /**
   * @brief Constructor
   * @param bstore Blob store
	*/
  explicit inode_management_service_factory(std::shared_ptr<blob_store> bstore, std::shared_ptr<monitor::monitor> mtr);
	
	private:

  /**
   * @brief Get inode management service handler
   * @param conn_info Connection information
   * @return Inode management handler
  */
  inode_management_serviceIf *getHandler(const ::apache::thrift::TConnectionInfo &connInfo) override;

  /**
   * @brief Release handler
   * @param handler Inode management handler
  */
  void releaseHandler(inode_management_serviceIf *anIf) override;

  /* Blob store */
  std::shared_ptr<blob_store> bstore_;
	/* Reference count monitor */
  std::shared_ptr<monitor::monitor> monitor_;
};

}
}

#endif // HIVE_INODE_MANAGEMENT_SERVICE_FACTORY_H