#include "monitor.h"

namespace hive {
namespace monitor {

monitor::monitor(std::shared_ptr<blob_store> bstore, std::shared_ptr<hive::predictor::predictor> ptr)
        : bstore_(std::move(bstore)),
          predictor_(std::move(ptr)) {}

void monitor::create(const hdag &workflow)
{
  auto refs = std::make_shared<hdag_monitor_info>(workflow);
  instances_.insert(workflow.name_, refs);
}

void monitor::remove(const std::string &workflow_name)
{
  instances_.erase(workflow_name);
}

void monitor::hook_get(const std::string &workflow_name, const std::string &function, const std::string &obj_name)
{
  auto refs = instances_.find(workflow_name);
  refs->buffer_add(function, obj_name);
}

void monitor::hook_put(const std::string &workflow_name, const std::string &function, const std::string &obj_name)
{
  auto refs = instances_.find(workflow_name);
  auto lifetime_info = predictor_->info(workflow_name);
  // commit function's buffer, compute lifetime of invalid data and write into correspond state_info
  refs->buffer_commit(function, bstore_, lifetime_info);
  // record created timestamp of object<obj_name>
  refs->touch(obj_name);
}

std::shared_ptr<hive::predictor::predictor> monitor::predictor()
{
  return predictor_;
}

}
}