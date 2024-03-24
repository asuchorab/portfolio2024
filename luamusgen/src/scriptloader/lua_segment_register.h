//
// Created by rutio on 13.08.2019.
//

#ifndef LUAMUSGEN_LUA_SEGMENT_REGISTER_H
#define LUAMUSGEN_LUA_SEGMENT_REGISTER_H

#include <lua5.1/lua.hpp>
#include "ExecutionManager.h"

/**
 * This file contains function callable from the lua project file
 */
namespace lua_segment_register {

  extern ExecutionManager* global_execution_manager;

  /**
   * Push the functions below to the lua state
   */
  void load(lua_State* L);

  /**
   * Load a lua segment file
   * arg1: string - filename of the segment
   * arg2 (optional): table - table containing string filenames of its prerequisites, in integer keys 1, 2, 3...
   */
  int loadSegment(lua_State* L);

  /**
   * Set number of worker threads, more than 256 is not allowed
   * arg1: int - number of threads, 0 or less - auto detect
   */
  int setThreadCount(lua_State* L);

  /**
   * Set directory where the program looks for segment lua files, can be absolute or relative from the project directory
   * arg1: string - directory
   */
  int setSegmentsDirectory(lua_State* L);

  /**
   * Set directory where the program runs during segment processing and does file I/O (BufferToWAV, BufferToRaw, etc.)
   * can be absolute or relative from the project directory
   * arg1: string - directory
   */
  int setWorkingDirectory(lua_State* L);

  /**
   * Add a directory for lua require function. Require will work with "dir/?.lua" and "dir/?/init.lua".
   * arg1: string - directory
   */
  int addRequireDirectory(lua_State* L);

  /**
   * Set whether the program will print out the graph structure after analyzing a segment file
   * arg1: bool
   */
  int setPrintGraph(lua_State* L);

  /**
   * Set whether the program will process files that haven't changed since last run
   * arg1: bool
   */
  int setProcessUnchangedFiles(lua_State* L);

  /**
   * Set whether the lua library uses ffi during segment processing (luajit only).
   */
  int setUseFFI(lua_State* L);

  /**
   * Set maximum number of segments processed at the same time, lower values reduce memory consumption
   * but may decrease performance due to synchronization between threads
   * arg1: integer - number of segments, 0 or less - unlimited
   */
  int setMaxProcessedSegments(lua_State* L);

  /**
   * Set whether the application will load saved variables in
   * arg1: bool
   */
  int setLoadVariables(lua_State* L);
}

#endif //LUAMUSGEN_LUA_SEGMENT_REGISTER_H
