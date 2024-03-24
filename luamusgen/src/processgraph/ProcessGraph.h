//
// Created by rutio on 26.04.19.
//

#ifndef LUAMUSGEN_PROCESSGRAPH_H
#define LUAMUSGEN_PROCESSGRAPH_H


#include <map>
#include <atomic>
#include <array>
#include <processgraph/kinds/ParameterNode.h>
#include <processgraph/kinds/TransformNode.h>
#include <processgraph/kinds/BufferNode.h>
#include <processgraph/kinds/DummyNode.h>
#include "ProcessNode.h"
#include <thread>
#include <util/ExpandableCircularQueue.h>
#include <util/TimeUtil.h>

/**
 * Describes graph of nodes that describe the sound that gets executed
 * in parallel after declaring it.
 */
class ProcessGraph {
public:
  ProcessGraph(): ready_nodes(2048) {}
  ProcessGraph(ProcessGraph&& other) noexcept;
  ProcessGraph& operator=(ProcessGraph&& other) noexcept;

  /**
   * Adds node to the graph, takes ownership of the pointer.
   * @return inserted node, may not be the same pointer
   */
  ProcessNode* addNode(std::unique_ptr<ProcessNode> node);

  /**
   * Adds node to the graph, takes ownership of the pointer.
   * @return inserted node, may not be the same pointer
   */
  ProcessNode* addNode(ProcessNode* node);

  /**
   * Execute nodes, can be started from multiple threads.
   * @return false if the execution is stuck, true if nothing bad happened
   */
  bool execute();

  /**
   * Prints the info about the graph (nodes, prerequisites, dependent nodes)
   * @param out stream to put the info
   */
  void printGraph(std::ostream& out);

  /**
   * Returns whether the graph has been processed
   */
  bool graphFinished();

private:
  bool started_execution = false;
  timeutil::point start_time;
  std::mutex ready_nodes_mutex;
  std::mutex node_count_mutex;
  std::mutex failed_threads_mutex;
  int node_count = 0;
  int finished_node_count = 0;
  std::array<std::map<double, ParameterNode*>, 257> constant_parameters;
  std::set<ParameterNode*> argless_other_parameters;
  //std::list<ProcessNode*> ready_nodes;
  util::ExpandableCircularQueue<ProcessNode*> ready_nodes;
  std::vector<std::unique_ptr<ParameterNode>> parameters;
  std::vector<std::unique_ptr<TransformNode>> transforms;
  std::vector<std::unique_ptr<BufferNode>> buffers;
  std::vector<std::unique_ptr<DummyNode>> dummies;
  std::set<std::thread::id> all_threads;
  std::set<std::thread::id> failed_threads;
  bool ready_nodes_request = false;

  ProcessNode* addParameterNode(ParameterNode* node);
  ProcessNode* addTransformNode(TransformNode* node);
  ProcessNode* addBufferNode(BufferNode* node);
  ProcessNode* addDummyNode(DummyNode* node);

  ProcessNode* pickReadyNode();
  void returnReadyNode(ProcessNode* node);

  template <class NodeType>
  void printGraphVectorInfo(std::ostream& out, const std::vector<std::unique_ptr<NodeType>>& vec);

  static int constBucketIndex(double value);

};


#endif //LUAMUSGEN_PROCESSGRAPH_H
