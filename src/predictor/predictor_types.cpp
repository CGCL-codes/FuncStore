#include "predictor_types.h"
#include "inference_client.h"
#include <fstream>
#include <unordered_map>
#include <list>
#include <algorithm>
#include <stdexcept>
#include <filesystem>

namespace hive {
namespace predictor {

hdag_lifetime_info::hdag_lifetime_info(const hdag &workflow)
{
  /**
   * Extract features for each state
  */
  app_ = workflow.app_;
  hdag_ = workflow.name_;
  for (auto &state : workflow.intermediate_states_) {
    try{
      auto info = std::make_shared<state_info>();
      info->features_ = extract_features(state, workflow);
      hdag_info_.insert(std::make_pair(state, info));
    }catch(std::exception &e){
      throw std::runtime_error("hdag_lifetime_info.cpp: [CONSTRUCT FUNCTION] failed");
    }
  }
}

void hdag_lifetime_info::inference(const std::string &model_host, int32_t model_port)
{
  /**
   * Call model inference service for all states
  */
  auto client = inference_client(model_host, model_port);
  std::vector<std::string> name_vec;
  std::vector<std::vector<int32_t>> features_vec;

  for (auto &kv : hdag_info_) {
    name_vec.push_back(kv.first);
    features_vec.push_back(kv.second->features_);
  }
  
  auto label_vec = client.inference(app_, features_vec);
  
  for (size_t i = 0; i < name_vec.size(); i++) {
    auto info = hdag_info_.at(name_vec[i]);
    info->predict_ = label_vec[i];
  }
}

int32_t hdag_lifetime_info::get(const std::string &state)
{
  return hdag_info_.at(state)->predict_;
}

void hdag_lifetime_info::set(const std::string &state, int32_t lifetime)
{
  hdag_info_.at(state)->real_ = lifetime;
}

void hdag_lifetime_info::dump(const std::string &path)
{
  /**
   * Dump <features[], real> to ${path}/${app_}/${hdag_}.csv
  */
  auto dataset_file = path + "/" + app_ + "/" + hdag_ + "-training.csv";
  // Create directories if they don't exist
  std::filesystem::create_directories(std::filesystem::path(dataset_file).parent_path());
  std::ofstream dataset(dataset_file);

  if (dataset.is_open()) {
    for (auto iter = hdag_info_.begin(); iter != hdag_info_.end(); ++iter) {
      auto state_info = iter->second;
      for (auto &feature : state_info->features_) {
        dataset << feature << ",";
      }
      dataset << state_info->real_ << "\n";
    }
    dataset.close();
  }
  else {
    throw std::logic_error("Can not open dataset file.");
  }
}


/*
  Extract features for a state
  1. each state's stage M
  2. each state's read function number R
  3. each state's read function referenced state numbers W
  4. influnce the state's lifetime longest path length S
  5. the cross stage number N of each state longest read/write path
*/

void findPaths(int32_t startStageId, hive::utils::pFuncNode readFunc, std::list<std::string> &&currentPath, std::vector<std::list<std::string>>& Paths) {
  if(readFunc->isStart) return;

  for(auto parent: readFunc->parents){
    if(parent->stageId == startStageId){
      currentPath.push_front(parent->name);
      Paths.push_back(std::move(currentPath));
      return;
    }
    else if(parent->stageId > startStageId){
      currentPath.push_front(parent->name);
      findPaths(startStageId, parent, std::move(currentPath), Paths);
    }
  }
  return;
}

std::vector<int32_t> hdag_lifetime_info::extract_features(const std::string &state, const hdag &workflow) {
  //implement here
  int32_t M = 0;
  int32_t R = 0;
  int32_t S = 0;
  int32_t N = 0;
  // std::vector<int32_t> W = {};
  int32_t W = 0;


  //1. each state's stage M
  for(auto &func: workflow.func_list) {
    std::for_each(func->outputs.begin(), func->outputs.end(), [&](const auto& output) {
      if(output == state) {
        M = func->stageId;
      }
    });
  }
  //2. each state's read function number R
  //4. influnce the state's lifetime longest path length S
  //5. the cross stage number N of each state longest read/write path
  bool isFound = false;
  try{
    for(auto &pipe: workflow.pipes_) {
      for(auto &branch: pipe->branches) {
        if(branch.obj_name == state) {
          R = branch.downstreams.size();

          for(auto &downstream: branch.downstreams) {
            // go in this state be read function
            std::vector<std::list<std::string> > tmpPaths = {};
            try{
              findPaths(M, downstream, std::list<std::string> {downstream->name}, tmpPaths);
            }
            catch(std::exception& e) {
              // if the exception is segment fault, print the state name and conitnue
              throw std::runtime_error("hdag_lifetime_info.cpp: [EXTRACT FEATURES] findPaths failed");
            }
            // Find the longest path
            S = std::max_element(tmpPaths.begin(), tmpPaths.end(), [](const auto& path1, const auto& path2) {
                return path1.size() < path2.size();
            })->size();

            int32_t tmpN =  downstream->stageId - pipe->upstreams[0]->stageId;
            if(tmpN > N) {
              N = tmpN;
            }
          }
          // go out these two loops
          isFound = true;
          break;
        }
      }
      if(isFound) break;
    }
  }
  catch(std::exception& e) {
    // if the exception is segment fault, print the state name and conitnue
    throw std::runtime_error(e.what());
  }
  //3. each state's read function referenced state numbers W
  for(auto &pipe: workflow.pipes_) {
    std::for_each(pipe->branches.begin(), pipe->branches.end(), [&](auto& branch) { if(branch.obj_name == state) { std::for_each(branch.downstreams.begin(), branch.downstreams.end(), [&](auto& downstream) { /*W.push_back(downstream->inputs.size());*/W += downstream->inputs.size(); }); } });
  }

  // push back features in order
  std::vector<int32_t> features = {M, R, W, S, N};
  // features.insert(features.end(), W.begin(), W.end());
  return features;
}

  std::string hdag_lifetime_info::toString() {
    std::string str = "";
    str += "App Name: " + app_ + "\n";
    str += "HiveDAG Name: " + hdag_ + "\n";

    for(const auto& kv: hdag_info_) {
      str += "State Name: " + kv.first + "\n";
      str += "Features: ";
      for(const auto& feature: kv.second->features_) {
        str += std::to_string(feature) + " ";
      }
      str += "\n";
      str += "Real: " + std::to_string(kv.second->real_) + "\n";
      str += "Predict: " + std::to_string(kv.second->predict_) + "\n";
    }
    if(str.size() > 0 && str.back() != '\n') {
      str += "\n";
    }
    return str;
  }
}
}