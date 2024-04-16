#include "predictor.h"

namespace hive {
namespace predictor {

predictor::predictor(const std::string &model_host, int32_t model_port, const std::string &datasets_path) :
        model_host_(model_host),
        model_port_(model_port),
        datasets_path_(datasets_path) {}

void predictor::create(const hdag &workflow)
{
  auto info = std::make_shared<hdag_lifetime_info>(workflow);
  // inference all states' lifetime
  info->inference(model_host_, model_port_);
  instances_.insert(workflow.name_, info);
}

void predictor::remove(const std::string &workflow_name)
{
  try {
    auto info = instances_.find(workflow_name);
    info->dump(datasets_path_);
    instances_.erase(workflow_name);
  }
  catch (std::out_of_range &e) {
    return;
  }
}

uint32_t predictor::predict(const std::string &workflow_name, const std::string &state)
{
  auto info = instances_.find(workflow_name);
  return info->get(state);
}

std::shared_ptr<hdag_lifetime_info> predictor::info(const std::string &workflow_name)
{
  return instances_.find(workflow_name);
}

}
}