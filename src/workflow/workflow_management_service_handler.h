#ifndef HIVE_WORKFLOW_MANAGEMENT_SERVICE_HANDLER_H
#define HIVE_WORKFLOW_MANAGEMENT_SERVICE_HANDLER_H

#include "workflow_management_service.h"
#include "monitor/monitor.h"

namespace hive {
namespace workflow {

class workflow_management_service_handler : public workflow_management_serviceIf {
	public:
		workflow_management_service_handler() = delete;
		workflow_management_service_handler(std::shared_ptr<monitor::monitor> mtr);
		~workflow_management_service_handler() = default;

		void workflow_register(const std::string &workflow);
    void workflow_deregister(const std::string &workflow_name);

	private:
		std::shared_ptr<monitor::monitor> monitor_;
};

}
}

#endif // HIVE_WORKFLOW_MANAGEMENT_SERVICE_HANDLER_H