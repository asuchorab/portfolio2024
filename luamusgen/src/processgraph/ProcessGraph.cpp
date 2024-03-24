//
// Created by rutio on 26.04.19.
//

#include <parameters/kinds/ParConst.h>
#include "ProcessGraph.h"
#include <processgraph/kinds/ParameterNode.h>
#include <util/MathUtil.h>
#include <sstream>
#include <processgraph/kinds/BufferNode.h>
#include <thread>
#include <random>
#include <util/TimeUtil.h>

ProcessGraph::ProcessGraph(ProcessGraph&& other) noexcept {
  std::lock_guard<std::mutex> lock(other.ready_nodes_mutex);
  std::lock_guard<std::mutex> lock2(other.node_count_mutex);
  started_execution = other.started_execution;
  constant_parameters = std::move(other.constant_parameters);
  argless_other_parameters = std::move(other.argless_other_parameters);
  ready_nodes = std::move(other.ready_nodes);
  parameters = std::move(other.parameters);
  transforms = std::move(other.transforms);
  buffers = std::move(other.buffers);
  dummies = std::move(other.dummies);
}

ProcessGraph& ProcessGraph::operator=(ProcessGraph&& other) noexcept {
  std::lock_guard<std::mutex> lock(other.ready_nodes_mutex);
  std::lock_guard<std::mutex> lock2(other.node_count_mutex);
  started_execution = other.started_execution;
  constant_parameters = std::move(other.constant_parameters);
  argless_other_parameters = std::move(other.argless_other_parameters);
  ready_nodes = std::move(other.ready_nodes);
  parameters = std::move(other.parameters);
  transforms = std::move(other.transforms);
  buffers = std::move(other.buffers);
  dummies = std::move(other.dummies);
  return *this;
}

ProcessNode* ProcessGraph::addNode(std::unique_ptr<ProcessNode> node) {
  return addNode(node.release());
}

ProcessNode* ProcessGraph::addNode(ProcessNode* node) {
  if (started_execution) {
    logErrorC("Attempted to add a node while the graph was executing");
    return nullptr;
  }

  if (!node) {
    logWarningC("Attempted to add a null node");
    return nullptr;
  }

  ProcessNode* ptr = node;

  if (auto p = dynamic_cast<ParameterNode*>(ptr)) {
    return addParameterNode(p);
  } else if (auto p = dynamic_cast<TransformNode*>(ptr)) {
    return addTransformNode(p);
  } else if (auto p = dynamic_cast<BufferNode*>(ptr)) {
    return addBufferNode(p);
  } else if (auto p = dynamic_cast<DummyNode*>(ptr)) {
    return addDummyNode(p);
  } else {
    logErrorC("Unknown node type");
    return nullptr;
  }
}

template<class NodeType>
void ProcessGraph::printGraphVectorInfo(std::ostream& out, const std::vector<std::unique_ptr<NodeType>>& vec) {
  for (auto& node : vec) {
    out << "*" << node->display_code;
    if (!node->prerequisites.empty()) {
      out << "\n -prerequisites: ";
      bool first_elem = true;
      for (auto p : node->prerequisites) {
        if (first_elem) {
          first_elem = false;
        } else {
          out << ", ";
        }
        out << p->display_code;
      }
      //if (node->prerequisites.size() != node->not_completed_prerequisites_counter) {
      //  out << " COUNTER IS WRONG: " << node->not_completed_prerequisites_counter;
      //}
    }
    if (!node->dependent_nodes.empty()) {
      out << "\n -dependent: ";
      bool first_elem = true;
      for (auto p : node->dependent_nodes) {
        if (first_elem) {
          first_elem = false;
        } else {
          out << ", ";
        }
        out << p->display_code;
      }
    }
    out << "\n";
  }
}

void ProcessGraph::printGraph(std::ostream& out) {
  out << parameters.size() + transforms.size() + buffers.size() + dummies.size() << " nodes: "
      << parameters.size() << " parameters, " << transforms.size() << " transforms, "
      << buffers.size() << " buffers, " << dummies.size() << " dummy nodes\n";
  printGraphVectorInfo(out, parameters);
  printGraphVectorInfo(out, transforms);
  printGraphVectorInfo(out, buffers);
  printGraphVectorInfo(out, dummies);

}

bool ProcessGraph::execute() {
  //auto t0 = timeutil::now();
  if (!started_execution) {
    start_time = timeutil::now();
  }
  started_execution = true;
  {
    std::lock_guard<std::mutex> lock(failed_threads_mutex);
    all_threads.insert(std::this_thread::get_id());
  }

  std::list<ProcessNode*> preferred_nodes;
  std::vector<ProcessNode*> temp_ready;
  std::vector<ProcessNode*> temp_initial;
  ProcessNode* current_node = pickReadyNode();
  int accumulated_failed_completions = 0;
  int thread_finished_nodes = 0;


  while (current_node) {

    // Complete the node
    //auto t1 = timeutil::now();
    if (!current_node->complete()) {
      //stat_compute_time.fetch_add(timeutil::duration(t1) * 1e9);
      if (current_node->state == ProcessNode::READY) {
        //auto t3 = timeutil::now();
        returnReadyNode(current_node);debugProcessC("Returned node " + current_node->display_code + " to the queue");
        //stat_return_node_time.fetch_add(timeutil::duration(t3) * 1e9);
      } else { debugProcessC(
            "Did not complete " + current_node->display_code + " (state: " + std::to_string(current_node->state) + ")");
      }
      accumulated_failed_completions++;
      /*if (accumulated_failed_completions > 1000) {
        accumulated_failed_completions = 0;debugProcessC("Sleeping, could not find a valid node");
        auto t2 = timeutil::now();
        std::this_thread::sleep_for(std::chrono::microseconds(10));
        stat_fail_sleeping_time.fetch_add(timeutil::duration(t2) * 1e9);
      }*/
    } else {
      //stat_compute_time.fetch_add(timeutil::duration(t1) * 1e9);
      thread_finished_nodes++;

      // Extract all dependent nodes, extract ready and initial nodes

      //auto t2 = timeutil::now();
      {
        std::lock_guard<std::mutex> lock(current_node->dependent_nodes_mutex);

        for (auto node : current_node->dependent_nodes) {
          if (node->state == ProcessNode::READY) {
            temp_ready.push_back(node);
          } else if (node->state == ProcessNode::INITIAL) {
            temp_initial.push_back(node);
          }
        }
      }
      //stat_dep_mutex_time.fetch_add(timeutil::duration(t2) * 1e9);

      // Add all initial dependent nodes to the end of the preferred nodes queue
      /*preferred_nodes.insert(preferred_nodes.end(), temp_initial.begin(), temp_initial.end());
      temp_initial.clear();*/
    }

    // Complete ready dependent nodes right away
    if (!temp_ready.empty()) {
      //stat_ready++;
      current_node = temp_ready.back();
      temp_ready.pop_back();

      // If some other thread is starving for ready nodes, give
      if (!temp_ready.empty()) {
        if (ready_nodes_request) {
          {
            std::lock_guard<std::mutex> lock(ready_nodes_mutex);
            //ready_nodes.insert(ready_nodes.begin(), temp_ready.begin(), temp_ready.end());
            for (auto p : temp_ready) {
              ready_nodes.push_front(p);
            }
            ready_nodes_request = false;
          }
          temp_ready.clear();
        }
      }

      // If there is no immediate ready node and the preferred nodes queue is not empty, take something from the queue
    } else {
      //stat_no_ready++;

      //auto t2 = timeutil::now();
      while (true) {
        if (temp_initial.empty()) {
          current_node = pickReadyNode();
          break;
        }
        current_node = temp_initial.back();
        temp_initial.pop_back();
        if (current_node->state == ProcessNode::INITIAL) {
          //stat_initial++;
          while (true) {
            ProcessNode* next_searched_node = nullptr;
            {
              std::lock_guard<std::mutex> lock(current_node->state_mutex);
              for (auto node : current_node->prerequisites) {
                if (node->state == ProcessNode::READY) {
                  temp_ready.push_back(node);
                } else if (node->state == ProcessNode::INITIAL) {
                  next_searched_node = node;
                }
              }
            }
            if (!temp_ready.empty()) {
              current_node = temp_ready.back();
              temp_ready.pop_back();
              //stat_found_ready++;
              goto break_1;
            }
            if (next_searched_node == nullptr) {
              break;
            } else {
              current_node = next_searched_node;
            }
          }
        } else if (current_node->state == ProcessNode::READY) {
          //stat_ready_in_queue++;
          break;
        } else {
          //stat_other_in_queue++;
        }
      }
      break_1:;
      temp_initial.clear();
      //stat_search_initial_time.fetch_add(timeutil::duration(t2) * 1e9);
      //current_node = pickReadyNode();
    }
  }

  bool fail = thread_finished_nodes == 0;
  if (fail) {
    {
      std::lock_guard<std::mutex> lock(failed_threads_mutex);
      failed_threads.insert(std::this_thread::get_id());
      // All threads failed, so the segment is impossible to complete
      if (failed_threads.size() == all_threads.size()) {
        //stat_execution_time.fetch_add(timeutil::duration(t0) * 1e9);
        return false;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  } else {
    {
      std::lock_guard<std::mutex> lock(failed_threads_mutex);
      failed_threads.clear();
    }
    std::lock_guard<std::mutex> lock(node_count_mutex);
    finished_node_count += thread_finished_nodes;
    //logInfoC("t: %s, finished: %d, q size: %d, req: %d", timeutil::timeFormat(timeutil::duration(start_time)).c_str(), thread_finished_nodes, ready_nodes.size(), ready_nodes_request);
  }debugProcessC("Exiting, nothing left to process, finished: %d, all: %d", finished_node_count, node_count);
  //stat_execution_time.fetch_add(timeutil::duration(t0) * 1e9);
  return true;
}

ProcessNode* ProcessGraph::pickReadyNode() {
  //timeutil::point t0 = timeutil::now();
  if (ready_nodes.size() < 50) {
    ready_nodes_request = true;
  }
  std::lock_guard<std::mutex> lock(ready_nodes_mutex);
  if (ready_nodes.empty()) {
    /*stat_queue_mutex_time.fetch_add(timeutil::duration(t0) * 1e9);
    stat_failed++;*/
    return nullptr;
  }
  ProcessNode* node;
  node = ready_nodes.back();
  ready_nodes.pop_back();
  while (node->state > ProcessNode::READY) {
    if (ready_nodes.empty()) {
      /*stat_queue_mutex_time.fetch_add(timeutil::duration(t0) * 1e9);
      stat_failed++;*/
      return nullptr;
    }
    node = ready_nodes.back();
    ready_nodes.pop_back();
  }
  /*stat_queue_mutex_time.fetch_add(timeutil::duration(t0) * 1e9);
  stat_succeed++;*/
  return node;
}

void ProcessGraph::returnReadyNode(ProcessNode* node) {
  if (!node) {
    return;
  }
  std::lock_guard<std::mutex> lock(ready_nodes_mutex);
  ready_nodes.push_front(node);
}

int ProcessGraph::constBucketIndex(double value) {
  int index = static_cast<int>(std::log(value) * 12.0) + 96;
  if (index < 0 || index > 256) {
    return 256;
  } else {
    return index;
  }
}

ProcessNode* ProcessGraph::addParameterNode(ParameterNode* node) {
  if (!node) {
    logErrorC("null pointer provided");
    return nullptr;
  }
  if (!node->parameter) {
    logErrorC("node's parameter is null");
    return nullptr;
  }
  if (!node->buffers.empty()) {
    logWarningC("parameter has declared buffer usage (parameter nodes don't use buffers)");
  }

  auto const_res = node->getConstValue();
  bool is_const = const_res.first;
  double value = const_res.second;
  size_t args_size = node->arguments.size();

  // Has no arguments
  if (args_size == 0) {

    // Has no arguments and is const
    if (is_const) {
      int bucket_index = constBucketIndex(value);
      std::map<double, ParameterNode*>& chosen_map = constant_parameters[bucket_index];
      auto lower_bound = chosen_map.lower_bound(value);
      if (lower_bound != chosen_map.end()) {
        if (mathut::equals(value, lower_bound->first)) {
          delete node;
          return lower_bound->second;
        }
      }
      if (lower_bound != chosen_map.begin()) {
        auto below_lower_bound = lower_bound;
        --below_lower_bound;
        if (mathut::equals(value, below_lower_bound->first)) {
          delete node;
          return below_lower_bound->second;
        }
      }
      chosen_map.emplace_hint(lower_bound, value, node);

      // Has no arguments and isn't const
    } else {
      bool found = false;
      for (auto p : argless_other_parameters) {
        auto merge_result = node->parameter->merge(*p->parameter);
        if (merge_result == ParGenerator::FULL && node->not_completed_prerequisites_counter == 0) {
          for (auto node_dep : node->dependent_nodes) {
            p->dependent_nodes.insert(node_dep);
            node_dep->prerequisites.insert(p);
            node_dep->prerequisites.erase(node);
            node_dep->not_completed_prerequisites_counter = node_dep->prerequisites.size();
          }
          delete node;
          return p;
        } else if (merge_result != ParGenerator::FAILURE) {
          found = true;
          p->dependent_nodes.insert(node);
          node->prerequisites.insert(p);
          node->not_completed_prerequisites_counter = node->prerequisites.size();
          node->parameter_view_node = p;
          for (auto node_dep : node->dependent_nodes) {
            p->dependent_nodes.insert(node_dep);
            node_dep->prerequisites.insert(p);
            node_dep->prerequisites.erase(node);
            node_dep->not_completed_prerequisites_counter = node_dep->prerequisites.size();
          }
          break;
        }
      }
      if (!found) {
        argless_other_parameters.emplace(node);
      }
    }

    // Has arguments
  } else {
    if (is_const) {
      logWarningC("const parameter with arguments");
    }
    /*
    std::vector<ProcessNode*> temp;
    temp.reserve(node->arguments.size());
    for (auto& arg : node->arguments) {
      temp.emplace_back(arg.node);
    }
    std::sort(temp.begin(), temp.end());
    size_t unique_args_count = std::distance(temp.begin(), std::unique(temp.begin(), temp.end()));
    if (node->not_completed_prerequisites_counter != unique_args_count) {
      logWarningC("parameter has prerequisites that aren't parameters (arguments)")
    }*/
    ParameterNode* checked_arg = node->arguments[0].node;
    for (auto dep : checked_arg->dependent_nodes) {
      if (auto p = dynamic_cast<ParameterNode*>(dep)) {
        if (dep->arguments != node->arguments || p == node) {
          continue;
        }
        auto merge_result = node->parameter->merge(*p->parameter);

        // Full merge, the node being added is identical to one that already exists
        if (merge_result == ParGenerator::FULL && node->not_completed_prerequisites_counter == args_size) {
          for (auto node_dep : node->dependent_nodes) {
            p->dependent_nodes.insert(node_dep);
            node_dep->prerequisites.insert(p);
            node_dep->prerequisites.erase(node);
            node_dep->not_completed_prerequisites_counter = node_dep->prerequisites.size();
          }
          for (auto node_pre : node->prerequisites) {
            node_pre->dependent_nodes.insert(p);
            node_pre->dependent_nodes.erase(node);
          }
          delete node;
          return p;

          // Partial merge, the node still exists but parameter points to another one instead of generating
        } else if (merge_result != ParGenerator::FAILURE) {
          p->dependent_nodes.insert(node);
          node->prerequisites.insert(p);
          node->not_completed_prerequisites_counter = node->prerequisites.size();
          node->parameter_view_node = p;
          for (auto node_dep : node->dependent_nodes) {
            p->dependent_nodes.insert(node_dep);
            node_dep->prerequisites.insert(p);
            node_dep->not_completed_prerequisites_counter = node_dep->prerequisites.size();
          }
          break;
        }
      }
    }
  }

  // If it didn't fully merge with anything
  if (node->state == ProcessNode::READY) {
    ready_nodes.push_back(node);
  }

  std::stringstream ss;
  ss << "P-" << parameters.size() << "-";
  if (is_const) {
    ss << "const-" << value;
  } else {
    ss << node->parameter->getType();
  }
  node->display_code = ss.str();
  parameters.emplace_back(node);
  node_count++;
  return node;
}

ProcessNode* ProcessGraph::addTransformNode(TransformNode* node) {
  if (!node) {
    logErrorC("null pointer provided");
    return nullptr;
  }
  if (!node->transform) {
    logErrorC("node's transform is null");
    return nullptr;
  }
  if (node->arguments.empty() && node->buffers.empty()) {
    logWarningC("dangling transform node - no arguments or buffers");
  }

  for (auto arg : node->arguments) {
    auto limits = arg.getLimits(0, node->getLengthSamples());
    arg.node->parameter->registerInterval(limits.first, limits.second);
  }

  for (auto buf : node->buffers) {
    buf.ptr->requestSpace(buf.start_point, buf.length);
  }

  if (node->state == ProcessNode::READY) {
    ready_nodes.push_back(node);
  }
  std::stringstream ss;
  ss << "T-" << transforms.size() << "-" << CLASS_NAME(*node->transform);
  node->display_code = ss.str();

  transforms.emplace_back(node);
  node_count++;
  return node;
}

ProcessNode* ProcessGraph::addBufferNode(BufferNode* node) {
  if (!node) {
    logErrorC("null pointer provided");
    return nullptr;
  }
  if (!node->arguments.empty()) {
    logWarningC("buffer has declared argument usage (buffer nodes don't use arguments)");
  }
  if (!node->buffers.empty()) {
    logWarningC("buffer has declared buffer usage (buffer nodes don't use buffers)");
  }
  if (node->state == ProcessNode::READY) {
    ready_nodes.push_back(node);
  }
  std::stringstream ss;
  ss << "B-" << buffers.size();
  node->display_code = ss.str();
  buffers.emplace_back(node);
  node_count++;
  return node;
}

ProcessNode* ProcessGraph::addDummyNode(DummyNode* node) {
  if (!node) {
    logErrorC("null pointer provided");
    return nullptr;
  }
  if (!node->arguments.empty()) {
    logWarningC("node has declared argument usage (dummy nodes don't use arguments)");
  }
  if (!node->buffers.empty()) {
    logWarningC("node has declared buffer usage (dummy nodes don't use buffers)");
  }
  if (node->state == ProcessNode::READY) {
    ready_nodes.push_back(node);
  }
  std::stringstream ss;
  ss << "D-" << dummies.size();
  node->display_code = ss.str();
  dummies.emplace_back(node);
  node_count++;
  return node;
}

bool ProcessGraph::graphFinished() {
  std::lock_guard<std::mutex> lock(node_count_mutex);
  return node_count == finished_node_count;
}