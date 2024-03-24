//
// Created by rutio on 2019-08-01.
//

#include "ExpShape.h"
#include <parameters/ParDeduce.hpp>
#include <util/MathUtil.h>

template<class T1>
struct ExpShape::FunctorMono {
  static void call(const T1& vol, double sample_rate, int64_t length, double* buf) {
    for (int64_t i = 0; i < length; ++i) {
      double val = ParData::Value(vol, i);
      buf[i] = mathut::pseudoExponential(buf[i], val);
    }
  }
};

template<class T1>
struct ExpShape::FunctorStereo {
  static void call(const T1& vol, double sample_rate, int64_t length, double* bufL, double* bufR) {
    for (int64_t i = 0; i < length; ++i) {
      double val = ParData::Value(vol, i);
      bufL[i] = mathut::pseudoExponential(bufL[i], val);
      bufR[i] = mathut::pseudoExponential(bufR[i], val);
    }
  }
};

void ExpShape::applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                                    const std::vector<arg_t> &arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(1, "exponent");
  ParDeduce<FunctorMono>::call<1>(*arguments[0], sample_rate, length, buf->data_pointer);
}

void ExpShape::applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                                      const std::vector<arg_t> &arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(1, "exponent");
  ParDeduce<FunctorStereo>::call<1>(*arguments[0], sample_rate, length, bufL->data_pointer, bufR->data_pointer);
}

