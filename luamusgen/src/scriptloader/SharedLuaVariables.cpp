//
// Created by rutio on 2020-01-28.
//

#include <util/Logger.h>
#include <sstream>
#include <iomanip>
#include "SharedLuaVariables.h"

SharedLuaVariables::LuaVariable::LuaVariable(const SharedLuaVariables::LuaVariable& other)
    : type(other.type),
      value(other.type == NIL ? Value(false)
                              : other.type == BOOLEAN ? Value(other.value.boolean)
                                                      : other.type == NUMBER ? Value(other.value.number)
                                                                             : Value(other.value.string)) {}

SharedLuaVariables::LuaVariable&
SharedLuaVariables::LuaVariable::operator=(const SharedLuaVariables::LuaVariable& other) {
  type = other.type;
  if (other.type == NIL) {
    value.boolean = false;
  } else if (other.type == BOOLEAN) {
    value.boolean = other.value.boolean;
  } else if (other.type == NUMBER) {
    value.number = other.value.number;
  } else {
    new(&value.string) std::string(other.value.string);
  }
  return *this;
}

SharedLuaVariables::LuaVariable::LuaVariable(SharedLuaVariables::LuaVariable&& other) noexcept
    : type(other.type),
      value(other.type == NIL ? Value(false)
                              : other.type == BOOLEAN ? Value(other.value.boolean)
                                                      : other.type == NUMBER ? Value(other.value.number)
                                                                             : Value(std::move(other.value.string))) {}

SharedLuaVariables::LuaVariable&
SharedLuaVariables::LuaVariable::operator=(SharedLuaVariables::LuaVariable&& other) noexcept {
  type = other.type;
  if (other.type == NIL) {
    value.boolean = false;
  } else if (other.type == BOOLEAN) {
    value.boolean = other.value.boolean;
  } else if (other.type == NUMBER) {
    value.number = other.value.number;
  } else {
    new(&value.string) std::string(std::move(other.value.string));
  }
  return *this;
}

void SharedLuaVariables::LuaVariable::output(std::ostream& os) {
  if (type == STRING) {
    std::string end_prefix = "]";
    std::string start_prefix = "[";
    while (value.string.find(end_prefix + "]") != std::string::npos) {
      end_prefix += "=";
      start_prefix += "=";
    }
    os << start_prefix << "[" << value.string << end_prefix << "]";
  } else if (type == NUMBER) {
    os << value.number;
  } else if (type == BOOLEAN) {
    os << (value.boolean ? "true" : "false");
  } else {
    os << "nil";
  }
}

SharedLuaVariables::LuaVariable SharedLuaVariables::getVariable(lua_State* L, int index) {
  int type = lua_type(L, index);
  if (type == LUA_TSTRING) {
    return {static_cast<std::string>(lua_tostring(L, index))};
  } else if (type == LUA_TNUMBER) {
    return {static_cast<double>(lua_tonumber(L, index))};
  } else if (type == LUA_TBOOLEAN) {
    return {static_cast<bool>(lua_toboolean(L, index))};
  } else if (type == LUA_TNIL) {
    return {};
  } else {
    luaL_error(L, "only nil, boolean, number and string keys and values are supported");
    return {};
  }
}

void SharedLuaVariables::pushVariable(lua_State* L, const SharedLuaVariables::LuaVariable& v) {
  if (v.type == STRING) {
    lua_pushstring(L, v.value.string.c_str());
  } else if (v.type == NUMBER) {
    lua_pushnumber(L, v.value.number);
  } else if (v.type == BOOLEAN) {
    lua_pushboolean(L, v.value.boolean);
  } else {
    lua_pushnil(L);
  }
}

void SharedLuaVariables::put(const SharedLuaVariables::LuaVariable& key, const SharedLuaVariables::LuaVariable& value) {
  if (key.type == STRING) {
    std::lock_guard<std::mutex> lock(string_mutex);
    if (!do_overwrite) {
      if (value.type != NIL) {
        string_keys.try_emplace(key.value.string, value);
      }
    } else {
      if (value.type == NIL) {
        string_keys.erase(key.value.string);
      } else {
        string_keys[key.value.string] = value;
      }
    }
  } else if (key.type == NUMBER) {
    std::lock_guard<std::mutex> lock(number_mutex);
    if (!do_overwrite) {
      if (value.type != NIL) {
        number_keys.try_emplace(key.value.number, value);
      }
    } else {
      if (value.type == NIL) {
        number_keys.erase(key.value.number);
      } else {
        number_keys[key.value.number] = value;
      }
    }
  } else if (key.type == BOOLEAN) {
    std::lock_guard<std::mutex> lock(bool_mutex);
    if (!do_overwrite) {
      if (value.type != NIL) {
        bool_keys.try_emplace(key.value.boolean, value);
      }
    } else {
      if (value.type == NIL) {
        bool_keys.erase(key.value.boolean);
      } else {
        bool_keys[key.value.boolean] = value;
      }
    }
  } else {
    throw std::runtime_error("put at nil key requested");
  }
}

SharedLuaVariables::LuaVariable SharedLuaVariables::get(const SharedLuaVariables::LuaVariable& key) {
  if (key.type == STRING) {
    std::lock_guard<std::mutex> lock(string_mutex);
    auto it = string_keys.find(key.value.string);
    if (it != string_keys.end()) {
      return it->second;
    } else {
      return LuaVariable();
    }
  } else if (key.type == NUMBER) {
    std::lock_guard<std::mutex> lock(number_mutex);
    auto it = number_keys.find(key.value.number);
    if (it != number_keys.end()) {
      return it->second;
    } else {
      return LuaVariable();
    }
  } else if (key.type == BOOLEAN) {
    std::lock_guard<std::mutex> lock(bool_mutex);
    auto it = bool_keys.find(key.value.boolean);
    if (it != bool_keys.end()) {
      return it->second;
    } else {
      return LuaVariable();
    }
  } else {
    throw std::runtime_error("get at nil key requested");
  }
}

SharedLuaVariables* SharedLuaVariables::instance = nullptr;

int SharedLuaVariables::load(lua_State* L) {
  const char* shared_load = "shared = {} setmetatable(shared, {__index = shared_index, __newindex = shared_newindex,"
                            "__metatable = 'protected'}) shared_index = nil shared_newindex = nil";
  lua_pushcfunction(L, lua_shared_index);
  lua_setglobal(L, "shared_index");
  lua_pushcfunction(L, lua_shared_newindex);
  lua_setglobal(L, "shared_newindex");
  int err = luaL_loadstring(L, shared_load);
  if (err != LUA_OK) {
    return err;
  }
  err = lua_pcall(L, 0, 0, lua_gettop(L) - 1);
  if (err != LUA_OK) {
    return err;
  }
  return LUA_OK;
}

void SharedLuaVariables::serialize(std::ostream& os) {
  if (!instance) {
    return;
  }
  os << std::setprecision(std::numeric_limits<double>::max_digits10);
  for (auto& pair : instance->bool_keys) {
    os << "shared[" << (pair.first ? "true" : "false") << "] = ";
    pair.second.output(os);
    os << "\n";
  }
  for (auto& pair : instance->number_keys) {
    os << "shared[" << pair.first << "] = ";
    pair.second.output(os);
    os << "\n";
  }
  for (auto& pair : instance->string_keys) {
    os << "shared[ ";
    LuaVariable(pair.first).output(os);
    os << " ] = ";
    pair.second.output(os);
    os << "\n";
  }
}

int SharedLuaVariables::lua_shared_index(lua_State* L) {
  try {
    auto key = SharedLuaVariables::getVariable(L, 2);
    auto value = instance->get(key);
    SharedLuaVariables::pushVariable(L, value);
  } catch (const std::exception& e) {
    luaL_error(L, e.what());
  }
  return 1;
}

int SharedLuaVariables::lua_shared_newindex(lua_State* L) {
  try {
    auto key = SharedLuaVariables::getVariable(L, 2);
    auto value = SharedLuaVariables::getVariable(L, 3);
    instance->put(key, value);
  } catch (const std::exception& e) {
    luaL_error(L, e.what());
  }
  return 0;
}

SharedLuaVariables* SharedLuaVariables::getInstance() {
  if (!instance) {
    instance = new SharedLuaVariables();
  }
  return instance;
}

void SharedLuaVariables::setDoOverwrite(bool v) {
  do_overwrite = v;
}
