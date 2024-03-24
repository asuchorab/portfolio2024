//
// Created by rutio on 13.08.2019.
//

#include "lua_segment_register.h"
#include <LuaBridge/LuaBridge.h>

ExecutionManager* lua_segment_register::global_execution_manager = nullptr;

void lua_segment_register::load(lua_State* L) {
  luabridge::getGlobalNamespace(L)
      .addCFunction("loadSegment", loadSegment)
      .addCFunction("setThreadCount", setThreadCount)
      .addCFunction("setSegmentsDirectory", setSegmentsDirectory)
      .addCFunction("setWorkingDirectory", setWorkingDirectory)
      .addCFunction("addRequireDirectory", addRequireDirectory)
      .addCFunction("setPrintGraph", setPrintGraph)
      .addCFunction("setProcessUnchangedFiles", setProcessUnchangedFiles)
      .addCFunction("setUseFFI", setUseFFI)
      .addCFunction("setMaxProcessedSegments", setMaxProcessedSegments)
      .addCFunction("setLoadVariables", setLoadVariables);
}

int lua_segment_register::loadSegment(lua_State* L) {
  if (!global_execution_manager) {
    logErrorF("internal error");
    return 0;
  }
  std::string path = luaL_checkstring(L, 1);
  std::vector<std::string> prerequisites;
  if (lua_gettop(L) > 1) {
    luaL_checktype(L, 2, LUA_TTABLE);
    int len = (int) lua_objlen(L, 2);
    for (int i = 1; i <= len; ++i) {
      lua_rawgeti(L, 2, i);
      if (lua_isstring(L, -1)) {
        std::string temp(lua_tostring(L, -1));
        auto it = std::find(prerequisites.begin(), prerequisites.end(), temp);
        if (it != prerequisites.end()) {
          logWarningF("prerequisite %s of %s added twice", temp.c_str(), path.c_str());
        } else {
          prerequisites.emplace_back(temp);
        }
      }
    }
  }
  try {
    global_execution_manager->addSegment(path, prerequisites);
  } catch (const std::invalid_argument& e) {
    luaL_error(L, e.what());
  }
  return 0;
}

int lua_segment_register::setThreadCount(lua_State* L) {
  if (!global_execution_manager) {
    logError("setThreadCount", "internal error");
    return 0;
  }
  int value = (int) luaL_checkinteger(L, 1);
  if (value > 256) {
    logErrorF("requested %d threads, bad idea", value);
  } else {
    global_execution_manager->setNumThreads(value);
  }
  return 0;
}

int lua_segment_register::setSegmentsDirectory(lua_State* L) {
  if (!global_execution_manager) {
    logErrorF("internal error");
    return 0;
  }
  std::string path = luaL_checkstring(L, 1);
  global_execution_manager->setSegmentsDirectory(path);
  return 0;
}

int lua_segment_register::setWorkingDirectory(lua_State* L) {
  if (!global_execution_manager) {
    logErrorF("internal error");
    return 0;
  }
  std::string path = luaL_checkstring(L, 1);
  global_execution_manager->setWorkingDirectory(path);
  return 0;
}

int lua_segment_register::addRequireDirectory(lua_State* L) {
  if (!global_execution_manager) {
    logErrorF("internal error");
    return 0;
  }
  std::string path = luaL_checkstring(L, 1);
  global_execution_manager->addRequireDirectory(path);
  return 0;
}

int lua_segment_register::setPrintGraph(lua_State* L) {
  if (!global_execution_manager) {
    logErrorF("internal error");
    return 0;
  }
  if (lua_isboolean(L, 1)) {
    bool value = lua_toboolean(L, 1);
    global_execution_manager->setPrintGraph(value);
  } else {
    lua_pushstring(L, "bad argument #1 to 'setPrintGraph' (boolean expected)");
    lua_error(L);
  }
  return 0;
}

int lua_segment_register::setProcessUnchangedFiles(lua_State* L) {
  if (!global_execution_manager) {
    logErrorF("internal error");
    return 0;
  }
  if (lua_isboolean(L, 1)) {
    bool value = lua_toboolean(L, 1);
    global_execution_manager->setProcessUnchangedFiles(value);
  } else {
    lua_pushstring(L, "bad argument #1 to 'setProcessUnchangedFiles' (boolean expected)");
    lua_error(L);
  }
  return 0;
}

int lua_segment_register::setUseFFI(lua_State* L) {
  if (!global_execution_manager) {
    logErrorF("internal error");
    return 0;
  }
  if (lua_isboolean(L, 1)) {
    bool value = lua_toboolean(L, 1);
    global_execution_manager->setUseFFI(value);
  } else {
    lua_pushstring(L, "bad argument #1 to 'setUseFFI' (boolean expected)");
    lua_error(L);
  }
  return 0;
}

int lua_segment_register::setMaxProcessedSegments(lua_State* L) {
  if (!global_execution_manager) {
    logError("setThreadCount", "internal error");
    return 0;
  }
  int value = (int) luaL_checkinteger(L, 1);
  global_execution_manager->setMaxProcessedSegments(value);
  return 0;
}

int lua_segment_register::setLoadVariables(lua_State* L) {
  if (!global_execution_manager) {
    logErrorF("internal error");
    return 0;
  }
  if (lua_isboolean(L, 1)) {
    bool value = lua_toboolean(L, 1);
    global_execution_manager->setLoadVariables(value);
  } else {
    lua_pushstring(L, "bad argument #1 to 'setUseFFI' (boolean expected)");
    lua_error(L);
  }
  return 0;
}
