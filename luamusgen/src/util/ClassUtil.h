//
// Created by rutio on 25.02.19.
//

#ifndef LUAMUSGEN_CLASSUTIL_H
#define LUAMUSGEN_CLASSUTIL_H

#include <typeinfo>
#include <functional>

namespace classutil {

  template <class To, class From>
  bool canCast(const From& obj) {
    try {
      auto p = dynamic_cast<const To&>(obj);
      return true;
    } catch (const std::bad_cast& e) {
      return false;
    }
  }

  template <class To, class From>
  bool compareIfCast(const From& obj, std::function<bool(const To&)> func) {
    try {
      auto p = dynamic_cast<const To&>(obj);
      if (&obj == &p) {
        return true;
      }
      return func(p);
    } catch (const std::bad_cast&) {
      return false;
    }
  }


  template <class RetType, RetType on_failure, class To, class From>
  RetType returnIfCast(const From& obj, std::function<RetType(const To&)> func) {
    try {
      auto p = dynamic_cast<const To&>(obj);
      return func(p);
    } catch (const std::bad_cast&) {
      return on_failure;
    }
  }

}

#endif //LUAMUSGEN_CLASSUTIL_H
