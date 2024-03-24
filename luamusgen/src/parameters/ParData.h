//
// Created by rutio on 23.02.19.
//

#ifndef LUAMUSGEN_DPAR_H
#define LUAMUSGEN_DPAR_H

#include <cstddef>
#include <type_traits>
#include <cmath>
#include <memory>

class ParDataConst;

/**
 * Base parameter data class, stores parameter data either as single value
 * (ParDataConst) or any form of non-constant. Non-constant parameter can
 * remember its data as its fields and can be computed to array form.
 * Non-constant data can be either ParDataArray (owned pointer) or ParDataView
 * (not owned pointer). The weird inheritance mixed with template and union
 * model ensures high performance low level access, either as a constant value
 * (ddata.value) or array (ddata.data), which is handled by ParValue function,
 * checked at compile time.
 */
class ParData {
public:

  /**
   * Get value of parameter at sample i, works for compile-time polymorphism
   * and not for runtime polymorphism, see ParDeduce.
   */
  template <typename ParT>
  static double Value(const ParT& par, int64_t i = 0) {
    if (std::is_same<ParDataConst, ParT>::value) {
      return par.data.value;
    } else {
      return par.data.array[i];
    }
  }

  /**
   * If parameter is const, works for compile-time polymorphism and not for
   * runtime polymorphism, see ParDeduce.
   */
  template <typename ParT>
  constexpr static bool IsConst(const ParT& par) {
    return std::is_same<ParDataConst, ParT>::value;
  }

  /**
   * If parameter is an array or in other words, isn't const, works for
   * compile-time polymorphism and not for runtime polymorphism, see ParDeduce.
   */
  template <typename ParT>
  constexpr static bool IsArray(const ParT& par) {
    return !std::is_same<ParDataConst, ParT>::value;
  }

  virtual ~ParData() = default;

  int64_t offset;
  int64_t length;
  union {
    double value;
    double* array = nullptr;
  } data;
};

#include "ParDataConst.h"

#endif //LUAMUSGEN_DPAR_H
