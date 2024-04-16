#ifndef HIVE_PREDICTOR_TYPES_H
#define HIVE_PREDICTOR_TYPES_H

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include "utils/hdag_utils.h"

namespace hive {
namespace predictor {

using namespace ::hive::utils;

struct state_info {
  std::vector<int32_t> features_ = {};
  int32_t predict_ = 0;
  int32_t real_ = 0;
};

class hdag_lifetime_info {
  public:
    hdag_lifetime_info() = delete;
    hdag_lifetime_info(const hdag &workflow);
    ~hdag_lifetime_info() = default;

    /**
     * @brief Predict all states' lifetime, and save in correspond state_info
     * @param model_host Model service host
     * @param model_port Model service port
    */
    void inference(const std::string &model_host, int32_t model_port);
    
    /**
     * @brief Get a state's predicted lifetime
     * @param state State name
     * @return Predicted lifetime
    */
    int32_t get(const std::string &state);
    
    /**
     * @brief Set a state's real lifetime
     * @param state State name
     * @param lifetime Real lifetime
    */
    void set(const std::string &state, int32_t lifetime);
    
    /**
     * @brief Save all states' <features[], real_> as dataset
     * @param path Path of dataset file
    */
    void dump(const std::string &path);

    std::vector<int32_t> extract_features(const std::string &state, const hdag &workflow);
    std::string toString();
  private:
    /* App name, used for correspond model */
    std::string app_;
    /* HDAG name */
    std::string hdag_;
    /* <state, state_info> for all states in hdag */
    std::unordered_map<std::string, std::shared_ptr<state_info>> hdag_info_;
};

}
}

#endif // HIVE_PREDICTOR_TYPES_H