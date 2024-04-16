#include "monitor_types.h"
#include "utils/time_utils.h"

namespace hive {
namespace monitor {

using namespace predictor;

void dec_ref(int32_t &ref)
{
  ref--;
}

hdag_monitor_info::hdag_monitor_info(const hdag &workflow)
{
  for (auto &pipe : workflow.pipes_) {
    commit_buffer.insert(pipe->upstreams[0]->name, std::make_shared<std::list<std::string>>());
    for (auto &branch : pipe->branches) {
      ref_table.insert(branch.obj_name, branch.downstreams.size());
    }
  }
}

void hdag_monitor_info::buffer_add(const std::string &function, const std::string &obj_name)
{
  auto buf = commit_buffer.find(function);
  buf->insert(buf->begin(), obj_name);
}

void hdag_monitor_info::buffer_commit(const std::string &function, std::shared_ptr<blob_store> bstore, std::shared_ptr<hdag_lifetime_info> info)
{
  auto buf = commit_buffer.find(function);
  auto timestamp = utils::time_utils::now_ms();
  for (auto &obj_name : *buf) {
    ref_table.update_fn(obj_name, dec_ref);
    if (ref_table.find(obj_name) == 0) {
      bstore->remove(obj_name);
      auto lifetime = static_cast<int32_t>(timestamp - timetable.find(obj_name));
      info->set(obj_name, lifetime);
    }
  }
  buf->clear();
}

void hdag_monitor_info::touch(const std::string &obj_name)
{
  auto start_ts = utils::time_utils::now_ms();
  timetable.insert(obj_name, start_ts);
}

}
}