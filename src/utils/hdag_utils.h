#ifndef HIVE_HDAG_UTILS_H
#define HIVE_HDAG_UTILS_H

#include <vector>
#include <string>
#include <set>
#include <unordered_map>

namespace hive {
namespace utils {

struct pipe_branch;
struct pipe_node;

/* Function node */
typedef struct func_node {
  /* Function name */
  std::string name;
  std::string id;
  std::string stage;
  int stageId;
  bool isStart;
  
  // Data dependency
  std::vector<std::string> inputs;
  std::vector<std::string> outputs;

  std::vector<std::string> next;

  // Parent and child
  std::vector<func_node*> parents;
  std::vector<func_node*> children;

  /* Downstream pipe attached to this function */
  pipe_node *downstream;

  func_node(std::string name, 
            std::string id,
            std::string stage,
            int stageId, 
            bool isStart,
            std::vector<std::string> inputs, 
            std::vector<std::string> outputs, 
            std::vector<std::string> next);
  ~func_node(){
    // delete parents and children
    for (auto &p : parents) {
      if (p != nullptr) {
        // delete p;
        p = nullptr;
      }
    }
    for (auto &c : children) {
      if (c != nullptr) {
        // delete c;
        c = nullptr;
      }
    }
    downstream = nullptr;
  }

  /**
   * @brief Get downstream branches(objects) for this function node
   * @return Object name array
  */
  std::vector<std::string> get_neighbor_objects();

  std::string to_string();
}FuncNode, *pFuncNode;

/* Component of pipe node */
struct pipe_branch {
  /* Ephemeral object name attached to this branch */
  std::string obj_name;
  /* Function(s) who read this branch's object */
  std::vector<func_node*> downstreams;

  /**
   * @brief Get downstream function nodes for this branch
   * @return Function node array
  */
  inline std::vector<func_node*> get_neighbor_functions();
};

/* Pipe node that describe data dependency between functions */
struct pipe_node {
  /* Upstream function who send object(s) to this pipe*/
  // func_node *upstream;
  std::vector<func_node*> upstreams;
  /* Branches split from this pipe */
  std::vector<pipe_branch> branches;

  /**
   * @brief Get downstream function nodes for all branches in this pipe node
   * @return Function node array
  */
 ~pipe_node(){
    // delete upstreams
    for (auto &u : upstreams) {
      if (u != nullptr) {
        u = nullptr;
      }
    }
  }
  std::vector<func_node*> get_neighbor_functions();
  std::string to_string();
};

/* Heterogeneous Directed Acyclic Graph (HDAG) definition */
typedef struct hdag {
  /* App which workflow belongs to */
  std::string app_;
  /* Workflow name */
  std::string name_;
  /* Function node hashtable */
  std::unordered_map<std::string, func_node*> functions_;
  std::vector<func_node*> func_list;
  std::vector<func_node*> head_funcs;
  std::vector<func_node*> tail_funcs;
  std::set<std::string> states_;
  std::set<std::string> intermediate_states_;
  /* Pipe node array */
  std::vector<pipe_node*> pipes_;

  hdag() = default;
  hdag(const std::string &app, const std::string &name);
  ~hdag();

  /**
   * @brief Add a function node (and a correspond pipe_node) into hdag
   * @param func Function node to be add
  */
  void add_node(func_node* func);
  
  /**
   * @brief Get a function node
   * @param func_name Function name
  */
  func_node *get_func(const std::string &func_name);
  
  /**
   * @brief Get a pipe node correspond to specific function
   * @param func_name Function name
  */
  pipe_node *get_pipe(const std::string &func_name);
  
  /**
   * @brief Add a branch to pipe node correspond to specific function
   * @param func_name Function name
   * @param branch Pipe branch
  */
  void add_branch(const std::string &func_name, pipe_branch branch);

  void make_head_funcs();
  void make_tail_funcs();

  std::vector<func_node*> get_head_funcs() {
    return head_funcs;
  }
  std::vector<func_node*> get_tail_funcs() {
    return tail_funcs;
  }

  /**
   * @brief construct the hdag
  */
  void construct();
} HDAG, *HDAGPtr;

}
}

#endif // HIVE_HDAG_UTILS_H