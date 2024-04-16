#ifndef HIVE_WORKFLOW_MANAGEMENT_SERVICE_FACTORY_H
#define HIVE_WORKFLOW_MANAGEMENT_SERVICE_FACTORY_H

#include "workflow_management_service.h"
#include "monitor/monitor.h"

namespace hive {
namespace workflow {

class workflow_management_service_factory : public workflow_management_serviceIfFactory {
	public:

  /**
   * @brief Constructor
   * @param mtr Monitor
	*/
  explicit workflow_management_service_factory(std::shared_ptr<monitor::monitor> mtr);
	
	private:

  /**
   * @brief Get workflow management service handler
   * @param conn_info Connection information
   * @return workflow management handler
  */
  workflow_management_serviceIf *getHandler(const ::apache::thrift::TConnectionInfo &connInfo) override;

  /**
   * @brief Release handler
   * @param handler workflow management handler
  */
  void releaseHandler(workflow_management_serviceIf *anIf) override;

	/* Monitor */
  std::shared_ptr<monitor::monitor> monitor_;
};

}
}

#endif // HIVE_WORKFLOW_MANAGEMENT_SERVICE_FACTORY_H