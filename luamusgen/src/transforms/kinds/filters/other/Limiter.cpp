//
// Created by rutio on 2019-07-31.
//

#include "Limiter.h"
#include <parameters/ParDeduce.hpp>
#include <util/MathUtil.h>

template<class T1>
struct Limiter::FunctorMono {
  static void call(const T1& limit, double sample_rate, int64_t length, double* buf) {
    for (int64_t i = 0; i < length; ++i) {
      buf[i] = mathut::limit(buf[i], ParData::Value(limit, i));
    }
  }
};

template<class T1>
struct Limiter::FunctorStereo {
  static void call(const T1& limit, double sample_rate, int64_t length, double* bufL, double* bufR) {
    for (int64_t i = 0; i < length; ++i) {
      bufL[i] = mathut::limit(bufL[i], ParData::Value(limit, i));
      bufR[i] = mathut::limit(bufR[i], ParData::Value(limit, i));
    }
  }
};

void Limiter::applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                                      const std::vector<arg_t> &arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(1, "limit");
  ParDeduce<FunctorMono>::call<1>(*arguments[0], sample_rate, length, buf->data_pointer);
}

void Limiter::applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                                        const std::vector<arg_t> &arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(1, "limit");
  ParDeduce<FunctorStereo>::call<1>(*arguments[0], sample_rate, length, bufL->data_pointer, bufR->data_pointer);
}
