#include "workflow_management_service_handler.h"
#include "utils/parse_workflow.h"
#include "spdlog/spdlog.h"


namespace hive {
namespace workflow {

workflow_management_service_handler::workflow_management_service_handler(std::shared_ptr<monitor::monitor> mtr)
        : monitor_(std::move(mtr)) {}

void workflow_management_service_handler::workflow_register(const std::string &workflow)
{
  utils::Parser parser;
  parser.parse(workflow);
  monitor_->predictor()->create(parser.get_hive_dag());
  monitor_->create(parser.get_hive_dag());
  spdlog::info("Receive workflow register request: {}", parser.get_hive_dag().name_);
}

void workflow_management_service_handler::workflow_deregister(const std::string &workflow_name)
{
  spdlog::info("Receive workflow deregister request: {}", workflow_name);
  monitor_->predictor()->remove(workflow_name);
  monitor_->remove(workflow_name);
}

}
}