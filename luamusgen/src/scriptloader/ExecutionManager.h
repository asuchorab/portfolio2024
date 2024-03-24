//
// Created by rutio on 13.08.2019.
//

#ifndef LUAMUSGEN_EXECUTIONMANAGER_H
#define LUAMUSGEN_EXECUTIONMANAGER_H

#ifdef __linux__

#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

#else

#include <filesystem>

namespace fs = std::filesystem;

#endif

#include <string>
#include <vector>
#include <map>
#include <set>
#include <mutex>
#include <thread>
#include <processgraph/ProcessGraph.h>
#include <deque>
#include <util/TimeUtil.h>
#include <lua5.1/lua.hpp>
#include "SharedLuaVariables.h"

/**
 * Handles the lua instance
 */
class ExecutionManager {
public:
  explicit ExecutionManager(std::string project_filename);

  ~ExecutionManager();

  // Functions that are callable from the project lua file
  void setSegmentsDirectory(const std::string& name);

  void setWorkingDirectory(const std::string& name);

  void addRequireDirectory(const std::string& name);

  void setNumThreads(int new_num_threads);

  void setPrintGraph(bool v);

  void setProcessUnchangedFiles(bool v);

  void setUseFFI(bool v);

  void setMaxProcessedSegments(int v);

  void setLoadVariables(bool v);

  void addSegment(const std::string& name, const std::vector<std::string>& prerequisites);

  // Start execution
  void execute();

  class SegmentNode {
  public:
    explicit SegmentNode(ExecutionManager& manager, std::string name, std::string path,
                         std::vector<SegmentNode*> prerequisites);

    enum Stage {
      INITIAL, GRAPH_BUILT, GRAPH_PROCESSED, FAILED
    };

    bool canBuildGraph();

    bool canProcessGraph();

    bool isFinished();

    void buildGraph();

    void processGraph();

    ExecutionManager& manager;
    Stage stage = INITIAL;
    std::string name;
    std::string path;
    std::vector<SegmentNode*> prerequisites;
    std::unique_ptr<ProcessGraph> process_graph;
    std::mutex time_measure_mutex;
    bool time_measure_started = false;
    bool time_measure_finished = false;
    timeutil::point time_measure_start;

  private:
    static thread_local std::ostream* thread_print_output;
    static int lua_print(lua_State* L);
  };

private:

  static int lua_traceback(lua_State* L);

  void threadFunction();

  ExecutionManager::SegmentNode* findSegmentToBuild();

  ExecutionManager::SegmentNode* findSegmentToProcess();

  bool areAllSegmentsFinished();

  timeutil::point execution_start;
  unsigned int num_threads;
  unsigned int next_segment = 0;
  unsigned int max_processing_segments = std::numeric_limits<int>::max();
  std::list<SegmentNode*> initialized_segments;
  std::set<SegmentNode*> built_segments;
  std::map<SegmentNode*, unsigned int> running_segments;
  std::set<SegmentNode*> finished_segments;
  std::set<SegmentNode*> failed_segments;
  std::list<SegmentNode> segments;
  std::mutex segments_mutex;
  std::vector<std::thread> threads;
  fs::path executable_path;
  fs::path project_path;
  fs::path project_file_path;
  fs::path segments_path;
  fs::path working_path;
  std::string require_paths;
  bool print_graph = false;
  bool process_unchanged_files = false;
  bool use_ffi = true;
  bool load_variables = true;
};

#endif //LUAMUSGEN_EXECUTIONMANAGER_H
