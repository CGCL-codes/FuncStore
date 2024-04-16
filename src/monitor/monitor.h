#ifndef HIVE_MONITOR_H
#define HIVE_MONITOR_H

#include "utils/hdag_utils.h"
#include "monitor_types.h"
#include "metadata/blob/blob_store.h"
#include "predictor/predictor.h"

namespace hive {
namespace monitor {

using namespace metadata;
using namespace utils;

class monitor {
  public:
    monitor() = delete;
    monitor(std::shared_ptr<blob_store> bstore, std::shared_ptr<hive::predictor::predictor> ptr);
    ~monitor() = default;

    /**
     * @brief Create a hdag_monitor_info for hdag instance and save it into instance repo
     * @param workflow Hdag instance
    */
    void create(const hdag &workflow);

    /**
     * @brief Remove a hdag_monitor_info from instance repo
     * @param workflow_name Hdag name
    */
    void remove(const std::string &workflow_name);

    /**
     * @brief Put a key in function's commit buffer, called by upper-level get()
     * @param workflow_name Workflow name
     * @param function Function who issues the get() op
     * @param obj_name Object name
    */
    void hook_get(const std::string &workflow_name, const std::string &function, const std::string &obj_name);

    /**
     * @brief Commit keys in function's commit buffer and record object's created time, called by upper-level put()
     * @param workflow_name Workflow name
     * @param function Function who issues the put() op
     * @param obj_name Object name to put
    */
    void hook_put(const std::string &workflow_name, const std::string &function, const std::string &obj_name);

    /**
     * @brief Get associated predictor
     * @return Pointer to associated predictor
    */
    std::shared_ptr<hive::predictor::predictor> predictor();

  private:
    /* Inode store managed by monitor */
    std::shared_ptr<blob_store> bstore_;
    /* Lifetime predictor */
    std::shared_ptr<hive::predictor::predictor> predictor_;
    /* <workflow_instance, current_ref> */
    libcuckoo::cuckoohash_map<std::string, std::shared_ptr<hdag_monitor_info>> instances_;
};

}
}
#endif // HIVE_MONITOR_H