#include "hdag_utils.h"

namespace hive {
namespace utils {

/************************** func_node operations **************************/

func_node::func_node(std::string name, 
                     std::string id,
                     std::string stage,
                     int stageId,
                     bool isStart, 
                     std::vector<std::string> inputs, 
                     std::vector<std::string> outputs, 
                     std::vector<std::string> next) :
        name(name),
        id(id),
        stage(stage),
        stageId(stageId),
        isStart(isStart),
        inputs(inputs),
        outputs(outputs),
        next(next) {}

std::vector<std::string> func_node::get_neighbor_objects()
{
  std::vector<std::string> neighbors;
  for (auto &branch : downstream->branches) {
    neighbors.push_back(branch.obj_name);
  }
  return neighbors;
}

std::string func_node::to_string() {
  std::string str = "func_node: " + name + "\n";
  str += "  id: " + id + "\n";
  str += "  stage: " + stage + "\n";
  str += "  stageId: " + std::to_string(stageId) + "\n";
  if(isStart) str += "  isStart: true\n";
  else str += "  isStart: false\n";
  str += "  inputs: ";
  for (auto &input : inputs) {
    str += input + " ";
  }
  str += "\n";
  str += "  outputs: ";
  for (auto &output : outputs) {
    str += output + " ";
  }
  str += "\n";
  str += "  next: ";
  for (auto &n : next) {
    str += n + " ";
  }
  str += "\n";
  // str += "  downstream pipe: ";
  // str += downstream->to_string();
  if(str.size() > 0 && str[str.size()-1] != '\n') str += "\n";
  return str;
}

/************************** pipe_branch operations **************************/

inline std::vector<func_node*> pipe_branch::get_neighbor_functions()
{
  return downstreams;
}

/************************** pipe_node operations **************************/

std::vector<func_node*> pipe_node::get_neighbor_functions()
{
  std::vector<func_node*> neighbors;
  for (auto &branch : branches) {
    neighbors.insert(neighbors.end(), branch.downstreams.begin(), branch.downstreams.end());
  }
  return neighbors;
}

std::string pipe_node::to_string()
{
  std::string str = "pipe_node: \n";
  str += "  branches: ";
  for (auto &branch : branches) {
    str += branch.obj_name + " " + "\ndownstreams: ";
    for (auto &downstream : branch.downstreams) {
      if(downstream == branch.downstreams.back())
        str += downstream->name;
      else
        str += downstream->name + " ";
    }
  }
  str += "\n";
  return str;
}

/************************** hdag operations **************************/

hdag::hdag(const std::string &app, const std::string &name) :
        app_(app),
        name_(name) {}

hdag::~hdag()
{
  for (auto &function : func_list) {
    if (function != nullptr) {
        delete function;
        function = nullptr;
    }
  }
  for (auto &pipe : pipes_) {
    if (pipe != nullptr) {
        delete pipe;
        pipe = nullptr;
    }
  }
}

void hdag::add_node(func_node* func)
{
  auto pipe = new pipe_node();
  pipe->upstreams.push_back(func);
  func->downstream = pipe;

  functions_.insert(std::make_pair(func->name, func));
  pipes_.push_back(pipe);
}

func_node *hdag::get_func(const std::string &func_name)
{
  return functions_.at(func_name);
}

pipe_node *hdag::get_pipe(const std::string &func_name)
{
  return functions_.at(func_name)->downstream;
}

void hdag::add_branch(const std::string &func_name, pipe_branch branch)
{
  auto pipe = functions_.at(func_name)->downstream;
  pipe->branches.push_back(std::move(branch));
}

void hdag::make_head_funcs() {
  for(auto &func : func_list) 
    if(func->isStart) head_funcs.push_back(func);
}

void hdag::make_tail_funcs() {
  for(auto &func : func_list) 
    if(func->children.size() == 0) tail_funcs.push_back(func);
}

void hdag::construct() {
  this->make_head_funcs();
  size_t num_funcs = func_list.size();
  for(size_t i=0; i<num_funcs; i++) {
    for(size_t j=0; j<num_funcs; j++) {
      if (i == j) continue;
      
      auto j_name = func_list[j]->name;
      if([&]() -> bool {
        for(auto &next : func_list[i]->next) {
          if(next == j_name) return true;
        }
        return false;
      }()) {
        func_list[i]->children.push_back(func_list[j]);
        func_list[j]->parents.push_back(func_list[i]);
      }
    }
  }
  this->make_tail_funcs();

  for(auto func : func_list) {
    for(auto &input : func->inputs) states_.insert(input);
    for(auto &output : func->outputs) states_.insert(output);
    if(!func->isStart) intermediate_states_.insert(func->inputs.begin(), func->inputs.end());
    if(!func->children.empty()) intermediate_states_.insert(func->outputs.begin(), func->outputs.end());
  }

  
  for(auto &func : func_list) {
    // create pipes
    struct pipe_node *pipe;
    if(func->children.size() == 0) continue;
    // // if there already exists a pipe point to the function's child, don't create a new pipe
    // // For each function's child
    // for(auto &child : func->children) {
    //   // For each pipe
    //   for(auto &pipe_ : pipes_) {
    //     // For each branch
    //     for(auto &branch : pipe->branches) {
    //       // For each downstream
    //       for(auto &downstream : branch.downstreams) {
    //         // If the downstream is a child of the function
    //         if(downstream == child){
    //           // Connect the function to the pipe
    //           pipe_->upstreams.push_back(func);
    //           func->downstream = pipe_;
    //           pipe = pipe_;
    //           // build branches
    //           goto build_branches;
    //         }
    //       }
    //     }
    //   }
    // }
    
    // If no pipe is found, create a new pipe
    pipe = new pipe_node();
    pipe->upstreams.push_back(func);
    func->downstream = pipe;
    pipes_.push_back(pipe);

// build_branches:
    for(auto &output : func->outputs) {
      pipe_branch branch;
      branch.obj_name = output;

      // find which child function has the output as input
      for(auto &child : func->children) {
        for(auto &input : child->inputs) {
          if(input == output)  branch.downstreams.push_back(child);
        }
      }

      pipe->branches.push_back(branch);
    }
  }
}

}
}