#ifndef HIVE_MONITOR_TYPES_H
#define HIVE_MONITOR_TYPES_H

#include <string>
#include <vector>
#include <list>
#include "metadata/blob/blob_store.h"
#include "utils/hdag_utils.h"
#include "predictor/predictor.h"
#include "libcuckoo/cuckoohash_map.hh"

namespace hive {
namespace monitor {

using namespace metadata;
using namespace utils;
using namespace predictor;

/* reference decrease function used by cuckoohash_map */
void dec_ref(int32_t &ref);

/* Per workflow instance info */
class hdag_monitor_info {
  public:
    hdag_monitor_info() = delete;
    hdag_monitor_info(const hdag &workflow);
    ~hdag_monitor_info() = default;
  
    void buffer_add(const std::string &function, const std::string &obj_name);
    void buffer_commit(const std::string &function, std::shared_ptr<blob_store> bstore, std::shared_ptr<hdag_lifetime_info> info);
  
    void touch(const std::string &obj_name);
  
  private:
    // Metadata for RC management

    /* <object, references> */
    libcuckoo::cuckoohash_map<std::string, int32_t> ref_table;
    /* <function, [object, object, ...]> */
    libcuckoo::cuckoohash_map<std::string, std::shared_ptr<std::list<std::string>>> commit_buffer;
    
    // Statistics for model training
    
    /* <object, lifetime_start_timestamp> */
    libcuckoo::cuckoohash_map<std::string, uint64_t> timetable;
};

}
}
#endif // HIVE_MONITOR_TYPES_H