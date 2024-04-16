#ifndef HIVE_PREDICTOR_H
#define HIVE_PREDICTOR_H

#include "predictor_types.h"
#include "libcuckoo/cuckoohash_map.hh"

namespace hive {
namespace predictor {

class predictor {
  public:
    predictor() = delete;
    predictor(const std::string &mode_host, int32_t model_port, const std::string &datasets_path);
    ~predictor() = default;

    /**
     * @brief Create a hdag_lifetime_info and save it into instances_
     * @param workflow A hdag
    */
    void create(const hdag &workflow);
    
    /**
     * @brief Remove a hdag_lifetime_info from instances_
     * @param workflow_name Workflow name used by create()
    */
    void remove(const std::string &workflow_name);

    /**
     * @brief Get a state's predicted lifetime
     * @param workflow_name Workflow name used by create()
     * @param state State name
    */
    uint32_t predict(const std::string &workflow_name, const std::string &state);

    /**
     * @brief Get correspond hdag_lifetime_info
     * @param workflow_name Workflow name
     * @return Pointer to hdag_lifetime_info
    */
    std::shared_ptr<hdag_lifetime_info> info(const std::string &workflow_name);

  private:
    /* <workflow_instance, hdag_profile> */
    libcuckoo::cuckoohash_map<std::string, std::shared_ptr<hdag_lifetime_info>> instances_;
    /* Model inference service host */
    std::string model_host_;
    /* Model inference service port */
    int32_t model_port_;
    /* Datasets path where to dump collected data */
    std::string datasets_path_;
};

}
}

#endif // HIVE_PREDICTOR_H