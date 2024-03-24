//
// Created by rutio on 13.08.2019.
//

#include <util/Logger.h>
#include <utility>
#include <fstream>
#include <util/TimeUtil.h>
#include "ExecutionManager.h"
#include "lua_segment_register.h"
#include "lua_segment_processing.h"

ExecutionManager::ExecutionManager(std::string project_filename) {
  num_threads = std::thread::hardware_concurrency();
  execution_start = timeutil::now();
  try {
    executable_path = fs::current_path();
    project_file_path = project_filename;
    project_path = fs::absolute(project_file_path).parent_path();
    segments_path = fs::absolute(project_path);
    working_path = fs::absolute(project_path);
  } catch (const fs::filesystem_error& e) {
    logError("ExecutionManager", e.what());
  }
}

ExecutionManager::~ExecutionManager() {
  logInfo("ExecutionManager", "finished processing in %s",
          timeutil::timeFormat(timeutil::duration(execution_start)).c_str());
}

void ExecutionManager::setSegmentsDirectory(const std::string& name) {
  fs::path new_path(name);
  if (!new_path.is_absolute()) {
    new_path = fs::absolute(project_path / new_path);
  }
  segments_path = new_path;
}

void ExecutionManager::setWorkingDirectory(const std::string& name) {
  fs::path new_path(name);
  if (!new_path.is_absolute()) {
    new_path = fs::absolute(executable_path / new_path);
  }
  working_path = new_path;
}

void ExecutionManager::addRequireDirectory(const std::string& name) {
  fs::path new_path(name);
  fs::path new_path2;
  if (!new_path.is_absolute()) {
    new_path2 = fs::absolute(executable_path / new_path);
    new_path = fs::absolute(project_path / new_path);
  }
  require_paths +=
      ";" + (new_path / "?.lua").string() +
      ";" + (new_path / "?" / "init.lua").string();

  if (!new_path2.empty()) {
    require_paths +=
        ";" + (new_path2 / "?.lua").string() +
        ";" + (new_path2 / "?" / "init.lua").string();
  }
}

void ExecutionManager::setNumThreads(int new_num_threads) {
  if (num_threads <= 0) {
    num_threads = std::thread::hardware_concurrency();
  } else {
    num_threads = static_cast<unsigned int>(new_num_threads);
  }
}

void ExecutionManager::setPrintGraph(bool v) {
  print_graph = v;
}

void ExecutionManager::setProcessUnchangedFiles(bool v) {
  process_unchanged_files = v;
}

void ExecutionManager::setUseFFI(bool v) {
  use_ffi = v;
}

void ExecutionManager::setMaxProcessedSegments(int v) {
  if (v <= 0) {
    max_processing_segments = std::numeric_limits<unsigned int>::max();
  } else {
    max_processing_segments = static_cast<unsigned int>(v);
  }
}

void ExecutionManager::setLoadVariables(bool v) {
  load_variables = v;
}

void ExecutionManager::addSegment(const std::string& name, const std::vector<std::string>& prerequisites) {
  // Check if there is no segment with the same name
  auto it = std::find_if(segments.begin(), segments.end(), [&name](auto& s) { return s.name == name; });
  if (it != segments.end()) {
    throw std::invalid_argument("segment " + name + " already exists");
  }

  // Check if all prerequisites exist and map their names to pointers
  std::vector<SegmentNode*> pre_ptrs;
  for (auto& pre : prerequisites) {
    if (pre == name) {
      throw std::invalid_argument("segment " + name + " is prerequisite of itself");
    }
    bool found = false;
    for (auto& node : segments) {
      if (pre == node.name) {
        found = true;
        pre_ptrs.emplace_back(&node);
      }
    }
    if (!found) {
      throw std::invalid_argument("prerequisite " + pre + " of " + name + " is not registered");
    }
  }

  // Add to the segments list
  segments.emplace_back(*this, name, (segments_path / name).string(), std::move(pre_ptrs));
  initialized_segments.push_back(&segments.back());
}

// Custom traceback function for better error information
int ExecutionManager::lua_traceback(lua_State* L) {
  lua_getfield(L, LUA_GLOBALSINDEX, "debug");
  lua_getfield(L, -1, "traceback");
  lua_pushvalue(L, 1);
  lua_pushinteger(L, 2);
  lua_call(L, 2, 1);
  return 1;
}

void ExecutionManager::execute() {
  auto time1 = timeutil::now();

  // Initialize lua state and load lua_segment_register.h functions
  lua_State* L = luaL_newstate();
  luaL_openlibs(L);
  lua_segment_register::global_execution_manager = this;
  lua_segment_register::load(L);

  // Fix path - enable loading libraries using require from both current directory and project directory
  // Set use_jit to whether lua is running luajit (type(jit) == 'table')
  const char* path_fix = "package.path = package.path .. require_paths require_paths = nil setUseFFI(type(jit) == 'table')";

  std::string require_project_path =
      ";" + (project_path / "?.lua").string() +
      ";" + (project_path / "?" / "init.lua").string();
  lua_pushstring(L, require_project_path.c_str());
  lua_setglobal(L, "require_paths");

  // Load shared variables utility
  if (SharedLuaVariables::getInstance()->load(L) != LUA_OK) {
    logError("ExecutionManager", "Unexpected error while loading shared lua variables: %s",
             std::string(lua_tostring(L, -1), lua_strlen(L, -1)).c_str());
    lua_close(L);
    exit(EXIT_FAILURE);
  }

  // Execute the path fix
  if (luaL_loadstring(L, path_fix) != LUA_OK) {
    logError("ExecutionManager", "Unexpected error while loading path fix: %s",
             std::string(lua_tostring(L, -1), lua_strlen(L, -1)).c_str());
    lua_close(L);
    exit(EXIT_FAILURE);
  } else if (lua_pcall(L, 0, 0, lua_gettop(L) - 1) != LUA_OK) {
    logError("ExecutionManager", "Unexpected error while fixing path: %s",
             std::string(lua_tostring(L, -1), lua_strlen(L, -1)).c_str());
    lua_close(L);
    exit(EXIT_FAILURE);

    // Execute the project file
  } else if (luaL_loadfile(L, project_file_path.string().c_str()) != LUA_OK) {
    logError("ExecutionManager", "Cannot open project file %s:\n%s", project_file_path.string().c_str(),
             std::string(lua_tostring(L, -1), lua_strlen(L, -1)).c_str());
    lua_close(L);
    exit(EXIT_FAILURE);
  }
  if (lua_pcall(L, 0, 0, lua_gettop(L) - 1) != LUA_OK) {
    logError("ExecutionManager", "Error while loading project file %s:\n%s", project_file_path.string().c_str(),
             std::string(lua_tostring(L, -1), lua_strlen(L, -1)).c_str());
    lua_close(L);
    exit(EXIT_FAILURE);
  }
  logInfo("ExecutionManager", "processed project file, time: %s",
          timeutil::timeFormat(timeutil::duration(time1)).c_str());

  // Change directory to the working path
  try {
    fs::current_path(working_path);
  } catch (const fs::filesystem_error& e) {
    logErrorC("Cannot change directory to %s: %s", working_path.string().c_str(), e.what());
  }

  // Load saved variables
  std::string variables_filename = "." + project_file_path.filename().string() + ".variables.lua";
  if (load_variables) {
    SharedLuaVariables::getInstance()->setDoOverwrite(false);
    int err = luaL_loadfile(L, variables_filename.c_str());
    if (err == LUA_OK) {
      if (lua_pcall(L, 0, 0, lua_gettop(L) - 1) != LUA_OK) {
        logWarning("ExecutionManager", "Error while loading variables file %s:\n%s", variables_filename.c_str(),
                   std::string(lua_tostring(L, -1), lua_strlen(L, -1)).c_str());
      }
    } else {
      if (err != LUA_ERRFILE) {
        logWarning("ExecutionManager", "Error while loading variables file %s:\n%s", variables_filename.c_str(),
                   std::string(lua_tostring(L, -1), lua_strlen(L, -1)).c_str());
      }
      lua_pushcfunction(L, lua_traceback);
    }
    SharedLuaVariables::getInstance()->setDoOverwrite(true);
  }

  lua_close(L);

  // Add executable directory and project directory to the require directory lists, for use in segment processing
  addRequireDirectory("");

  // Open timestamps file
  std::map<std::string, int64_t> timestamps_old;
  std::string timestamps_filename = "." + project_file_path.filename().string() + ".timestamps";
  std::ifstream timestamps_ifs(timestamps_filename, std::ios::binary);
  while (timestamps_ifs.good()) {
    std::string seg_name;
    char ch = 0;
    while (timestamps_ifs.good() && (ch = (char) timestamps_ifs.get()) != '\0') {
      seg_name += ch;
    }
    int64_t write_time;
    timestamps_ifs.read((char*) &write_time, sizeof(write_time));
    if (timestamps_ifs.good()) {
      timestamps_old[seg_name] = write_time;
    }
  }
  timestamps_ifs.close();

  // Compare timestamps file with current file timestamps and mark unchanged as processed if not process_unchanged_files
  // Overwrite the timestamps file
  std::ofstream timestamps_ofs(timestamps_filename, std::ios::binary);
  for (auto& seg : segments) {
    int64_t write_time;
    try {
      write_time = fs::last_write_time(seg.path).time_since_epoch().count();
      if (write_time == timestamps_old[seg.name]) {
        bool success = true;
        for (auto& pre : seg.prerequisites) {
          if (pre->stage != SegmentNode::GRAPH_PROCESSED) {
            success = false;
            break;
          }
        }
        if (success && !process_unchanged_files) {
          logInfo("ExecutionManager", "segment %s omitted because no changes to it or any prerequisites were made",
                  seg.name.c_str());
          seg.stage = SegmentNode::GRAPH_PROCESSED;
          auto it = std::find(initialized_segments.begin(), initialized_segments.end(), &seg);
          initialized_segments.erase(it);
        }
      }
      timestamps_ofs << seg.name << '\0';
      timestamps_ofs.write((char*) &write_time, sizeof(write_time));
    } catch (const fs::filesystem_error& e) {
      logErrorC(e.what());
    }
  }
  timestamps_ofs.close();

  // Spawn worker threads
  for (unsigned int i = 0; i < num_threads; ++i) {
    threads.emplace_back(&ExecutionManager::threadFunction, this);
  }
  for (auto& th : threads) {
    th.join();
  }
  std::ofstream variables_ofs(variables_filename, std::ios::binary);
  SharedLuaVariables::getInstance()->serialize(variables_ofs);
  variables_ofs.close();
}

void ExecutionManager::threadFunction() {
  int failed_count = 0;
  while (true) {
    // Find if some segment hasn't been built yet (execute segment lua file)
    if (auto seg = findSegmentToBuild()) {
      seg->buildGraph();
      failed_count = 0;
      // Find if some segment can be processed (execute instructions for the graph)
    } else if (auto seg = findSegmentToProcess()) {
      seg->processGraph();
      failed_count = 0;
      // Exit if everything is completed or something failed
    } else if (areAllSegmentsFinished()) {
      break;
      // Sleep for a while if no job is available at this moment
    } else {
      failed_count++;
      if (failed_count > 10) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        failed_count = 0;
      }
    }
  }
}

ExecutionManager::SegmentNode* ExecutionManager::findSegmentToBuild() {
  // Return a suitable segment to build - grab one from initialized segments list
  std::lock_guard<std::mutex> lock(segments_mutex);
  if (!initialized_segments.empty()) {
    SegmentNode* found = initialized_segments.front();
    if (found->canBuildGraph()) {
      initialized_segments.pop_front();
      return found;
    } else {
      return nullptr;
    }
  }
  return nullptr;
}

ExecutionManager::SegmentNode* ExecutionManager::findSegmentToProcess() {
  // Iterate over segments list and return the first segment that is built and can be processed
  // and has the least amount of threads, while also respecting maximum segments processed at once
  std::lock_guard<std::mutex> lock(segments_mutex);
  if (running_segments.size() < max_processing_segments) {
    SegmentNode* found = nullptr;
    for (auto& seg : built_segments) {
      if (seg->canProcessGraph()) {
        found = seg;
        break;
      }
    }
    if (found) {
      built_segments.erase(found);
      running_segments[found] = 1;
      return found;
    }
  }
  auto min_found = std::min_element(running_segments.begin(), running_segments.end(),
                                    [](auto v1, auto v2) {
                                      if (v1.first->stage != SegmentNode::GRAPH_BUILT) {
                                        return false;
                                      }
                                      if (v2.first->stage != SegmentNode::GRAPH_BUILT) {
                                        return true;
                                      }
                                      return v1.second < v2.second;
                                    });
  if (min_found != running_segments.end() && min_found->first->stage == SegmentNode::GRAPH_BUILT) {
    running_segments[min_found->first]++;
    return min_found->first;
  }
  return nullptr;
}

bool ExecutionManager::areAllSegmentsFinished() {
  // Check if everything is finished or failed
  std::lock_guard<std::mutex> lock(segments_mutex);
  for (auto& seg : segments) {
    if (!seg.isFinished()) {
      return false;
    }
  }
  return true;
}

ExecutionManager::SegmentNode::SegmentNode(ExecutionManager& manager, std::string name, std::string path,
                                           std::vector<SegmentNode*> prerequisites)
    : manager(manager), name(std::move(name)), path(std::move(path)), prerequisites(std::move(prerequisites)),
      process_graph(new ProcessGraph()) {}


bool ExecutionManager::SegmentNode::canBuildGraph() {
  for (auto& seg : prerequisites) {
    if (seg->stage == SegmentNode::INITIAL) {
      return false;
    }
  }
  return true;
}

bool ExecutionManager::SegmentNode::canProcessGraph() {
  // Check if the graph has been built and if all prerequisites have been completed
  // If any prerequisite fails, mark this segment as failed
  if (stage == GRAPH_BUILT) {
    for (auto& pre : prerequisites) {
      if (pre->stage == FAILED) {
        stage = FAILED;
        logError("ExecutionManager", "Segment %s failed because prerequisite %s failed", name.c_str(),
                 pre->name.c_str());
        break;
      }
    }
    for (auto& pre : prerequisites) {
      if (pre->stage != GRAPH_PROCESSED) {
        return false;
      }
    }
    return true;
  }
  return false;
}

bool ExecutionManager::SegmentNode::isFinished() {
  // Whether nothing can be done (has finished or failed)
  return stage == GRAPH_PROCESSED || stage == FAILED;
}

/*
static int wrap_exceptions(lua_State* L, lua_CFunction f) {
  try {
    return f(L);  // Call wrapped function and return result.
  } catch (const char* s) {  // Catch and convert exceptions.
    lua_pushstring(L, s);
  } catch (std::exception& e) {
    lua_pushstring(L, e.what());
  } catch (...) {
    lua_pushliteral(L, "caught (...)");
  }
  return lua_error(L);  // Rethrow as a Lua error.
}*/


int ExecutionManager::SegmentNode::lua_print(lua_State* L) {
  int n = lua_gettop(L);
  int i;
  lua_getglobal(L, "tostring");
  for (i=1; i<=n; i++) {
    const char *s;
    lua_pushvalue(L, -1);
    lua_pushvalue(L, i);
    lua_call(L, 1, 1);
    s = lua_tostring(L, -1);
    if (s == NULL)
      return luaL_error(L, LUA_QL("tostring") " must return a string to "
                           LUA_QL("print"));
    if (i>1) {
      (*thread_print_output) << "\t";
    }
    (*thread_print_output) << s;
    lua_pop(L, 1);
  }
  (*thread_print_output) << "\n";
  return 0;
}

thread_local std::ostream* ExecutionManager::SegmentNode::thread_print_output;

void ExecutionManager::SegmentNode::buildGraph() {
  auto time1 = timeutil::now();
  lua_State* L = luaL_newstate();
  luaL_openlibs(L);

  //Output for this segment
  std::stringstream output;
  thread_print_output = &output;

  //Set custom print function
  lua_pushcfunction(L, lua_print);
  lua_setglobal(L, "print");

  // Custom traceback function
  lua_pushcfunction(L, lua_traceback);

  // Setting thread local pointer to process graph
  lua_segment_processing::thread_graph = process_graph.get();

  // Load the segment processing library or set ffi flag, depending on use_ffi
  if (manager.use_ffi) {
    lua_pushboolean(L, true);
    lua_setglobal(L, "luamusgen_use_ffi");
  } else {
    lua_segment_processing::load(L);
  }

  // Fixing paths - enabling require to use all directories defined in the project file
  // and also executable and project directories
  lua_pushstring(L, manager.require_paths.c_str());
  lua_setglobal(L, "require_paths");

  const char* path_fix = "package.path = package.path .. require_paths require_paths = nil";

  // Load shared variables utility
  if (SharedLuaVariables::getInstance()->load(L) != LUA_OK) {
    logError("ExecutionManager", "Unexpected error while loading shared lua variables: %s",
             std::string(lua_tostring(L, -1), lua_strlen(L, -1)).c_str());
    stage = FAILED;

    // Execute the path fix
  } else if (luaL_loadstring(L, path_fix) != LUA_OK) {
    logError("ExecutionManager", "Unexpected error while loading path fix: %s",
             std::string(lua_tostring(L, -1), lua_strlen(L, -1)).c_str());
    stage = FAILED;
  } else if (lua_pcall(L, 0, 0, lua_gettop(L) - 1) != LUA_OK) {
    logError("ExecutionManager", "Unexpected error while fixing path: %s",
             std::string(lua_tostring(L, -1), lua_strlen(L, -1)).c_str());
    stage = FAILED;

    // Load the segment file
  } else if (luaL_loadfile(L, path.c_str()) != LUA_OK) {
    logError("ExecutionManager", "Cannot open segment file %s:\n%s", name.c_str(),
             std::string(lua_tostring(L, -1), lua_strlen(L, -1)).c_str());
    stage = FAILED;
  } else if (lua_pcall(L, 0, 0, lua_gettop(L) - 1) != LUA_OK) {
    // Lua file print output
    std::string print_header;
    std::string print_str = output.str();
    if (!print_str.empty()) {
      print_header = "\nLua output:\n";
    }
    logError("ExecutionManager", "Error while loading segment file %s:\n%s%s%s", name.c_str(),
             std::string(lua_tostring(L, -1), lua_strlen(L, -1)).c_str(), print_header.c_str(), print_str.c_str());
    stage = FAILED;

    // Success
  } else {
    stage = GRAPH_BUILT;

    // Optional graph print
    std::stringstream graph_output;
    if (manager.print_graph) {
      graph_output << '\n';
      process_graph->printGraph(graph_output);
    }

    // Lua file print output
    std::string print_header;
    std::string print_str = output.str();
    if (!print_str.empty()) {
      print_header = "\nLua output:\n";
    }

    // Print success info
    logInfo("ExecutionManager", "analyzed segment %s, time: %s%s%s%s", name.c_str(),
            timeutil::timeFormat(timeutil::duration(time1)).c_str(), print_header.c_str(), print_str.c_str(),
            graph_output.str().c_str());

    // Add the graph to list of built segments
    std::lock_guard<std::mutex> lock(manager.segments_mutex);
    manager.built_segments.insert(this);
  }

  lua_close(L);
}

void ExecutionManager::SegmentNode::processGraph() {
  // Start measuring time of segment execution if it's the first thread to start executing it
  {
    std::lock_guard<std::mutex> lock(time_measure_mutex);
    if (!time_measure_started) {
      time_measure_started = true;
      time_measure_start = timeutil::now();
    }
  }

  //Do the graph execution
  bool success = true;
  if (!process_graph->graphFinished()) {
    success = process_graph->execute();
  }

  // If graph processing was successful, check if it's finished and print timing information
  std::lock_guard<std::mutex> lock1(manager.segments_mutex);
  if (success) {
    if (process_graph->graphFinished()) {
      stage = GRAPH_PROCESSED;
      std::lock_guard<std::mutex> lock(time_measure_mutex);
      if (!time_measure_finished) {
        time_measure_finished = true;
        logInfo("ExecutionManager", "processed segment %s, time: %s", name.c_str(),
                timeutil::timeFormat(timeutil::duration(time_measure_start)).c_str());
      }
    }

    // If graph processing was unsuccessful, mark it as failed and print timing information
  } else {
    stage = FAILED;
    std::lock_guard<std::mutex> lock(time_measure_mutex);
    if (!time_measure_finished) {
      time_measure_finished = true;
      logInfo("ExecutionManager", "failed to process segment %s, time: ", name.c_str(),
              timeutil::timeFormat(timeutil::duration(time_measure_start)).c_str());
    }
  }

  // If processing has been finished and there is no other thread in the segment,
  // deallocate (unique_ptr::reset) the process graph in order to free memory
  manager.running_segments[this]--;
  if (manager.running_segments[this] == 0 && isFinished()) {
    process_graph.reset();
    manager.running_segments.erase(this);
  }
}
