//
// Created by rutio on 26.04.19.
//

#ifndef LUAMUSGEN_PROCESSNODE_H
#define LUAMUSGEN_PROCESSNODE_H

#include <vector>
#include <set>
#include <limits>
#include <parameters/Parameter.h>
#include <buffer/Buffer.h>

class ParameterNode;

/**
 * Describes a node in the process graph, see ProcessGraph.
 */
class ProcessNode {
public:
  enum State {
    // not ready to process
    INITIAL = 0,
    // ready to process
    READY = 1,
    // processed
    COMPLETED = 2,
    // deallocated because no uncompleted node depends on it
    DEAD = 3
  };

  struct ArgumentData {
    ArgumentData(ParameterNode* node, int64_t rel_start_point, bool fixed_start_point = false,
                 int64_t alt_length = std::numeric_limits<int64_t>::max(), bool touch_zero = false)
        : node(node), rel_start_point(rel_start_point), alt_length(alt_length), fixed_start_point(fixed_start_point),
          touch_zero(touch_zero) {}

    ParameterNode* node;
    int64_t rel_start_point;
    int64_t alt_length;
    bool fixed_start_point;
    bool touch_zero;

    inline bool hasLength() { return alt_length != std::numeric_limits<int64_t>::max(); }

    inline bool operator==(const ArgumentData& other) const {
      return node == other.node && rel_start_point == other.rel_start_point && alt_length == other.alt_length &&
             fixed_start_point == other.fixed_start_point && touch_zero == other.touch_zero;
    }

    std::pair<int64_t, int64_t> getLimits(int64_t offset, int64_t base_length);

    int64_t getStartPoint(int64_t offset);

    int64_t getLength(int64_t base_length);

  };

  struct BufferData {
    BufferData(Buffer* ptr, ProcessNode* node, int64_t start_point, int64_t length)
        : ptr(ptr), node(node), start_point(start_point), length(length) {}

    Buffer* ptr;
    ProcessNode* node;
    int64_t start_point;
    int64_t length;
  };

  virtual double getSampleRate();

  virtual int64_t getLengthSamples();

  virtual int64_t getOffset();

  void addArgument(double start_point, ProcessNode* node,
                   bool fixed_start_point = false, double other_length = -1e18,
                   bool touch_zero = false);

  void addBuffer(double start_point, ProcessNode* node,
                 double other_length = -1e18, bool allow_other_sample_rate = false);

  void addPrerequisite(ProcessNode* node);

  virtual ~ProcessNode() = default;

  bool complete();

protected:
  virtual void process(const std::vector<const Buffer::TransformData*>& buffers,
                       const std::vector<const ParData*>& arguments) = 0;

  virtual void deallocate() = 0;

  std::vector<ArgumentData> arguments;
  std::vector<BufferData> buffers;

private:
  State state = READY;
  std::mutex state_mutex;
  bool is_being_processed = false;
  size_t not_completed_prerequisites_counter = 0;
  std::mutex prerequisites_mutex;
  std::set<ProcessNode*> prerequisites;
  std::mutex dependent_nodes_mutex;
  std::set<ProcessNode*> dependent_nodes;

  std::string display_code;

  friend class ProcessGraph;
};


#endif //LUAMUSGEN_PROCESSNODE_H
