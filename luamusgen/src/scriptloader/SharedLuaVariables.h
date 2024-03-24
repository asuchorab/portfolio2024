//
// Created by rutio on 2020-01-28.
//

#ifndef LUAMUSGEN_SHAREDLUAVARIABLES_H
#define LUAMUSGEN_SHAREDLUAVARIABLES_H

#include <string>
#include <map>
#include <mutex>
#include <lua5.1/lua.hpp>

/**
 * Utility for holding lua variables between lua instances.
 */
class SharedLuaVariables {
public:
  enum Type {
    NIL,
    BOOLEAN,
    NUMBER,
    STRING
  };

  // used to be union and worked under GNU but it was probably not good
  struct Value {
    Value(bool v): boolean(v) {}
    Value(double v): number(v) {}
    Value(std::string v): string(std::move(v)) {}
    ~Value() {};
    bool boolean;
    double number;
    std::string string;
  };

  struct LuaVariable {
    LuaVariable(): type(NIL), value(false) {}
    LuaVariable(bool v): type(BOOLEAN), value(v) {}
    LuaVariable(double v): type(NUMBER), value(v) {}
    LuaVariable(std::string v): type(STRING), value(std::move(v)) {}
    LuaVariable(const LuaVariable& other);
    LuaVariable& operator=(const LuaVariable& other);
    LuaVariable(LuaVariable&& other) noexcept;
    LuaVariable& operator=(LuaVariable&& other) noexcept;
    ~LuaVariable() { if (type == STRING) value.string.~basic_string(); }
    void output(std::ostream& os);
    Type type;
    Value value;
  };

  static LuaVariable getVariable(lua_State* L, int index);
  static void pushVariable(lua_State* L, const LuaVariable& v);
  int load(lua_State* L);
  void serialize(std::ostream& os);
  void setDoOverwrite(bool v);

  void put(const LuaVariable& key, const LuaVariable& value);
  LuaVariable get(const LuaVariable& key);

  static SharedLuaVariables* getInstance();

private:
  static SharedLuaVariables* instance;
  static int lua_shared_index(lua_State* L);
  static int lua_shared_newindex(lua_State* L);

  std::map<bool, LuaVariable> bool_keys;
  std::map<std::string, LuaVariable> string_keys;
  std::map<double, LuaVariable> number_keys;
  std::mutex bool_mutex;
  std::mutex string_mutex;
  std::mutex number_mutex;
  bool do_overwrite = true;
};


#endif //LUAMUSGEN_SHAREDLUAVARIABLES_H
