//
// Created by rutio on 28.02.19.
//

#include "ChangeVolume.h"
#include <parameters/ParDeduce.hpp>

template<class T1>
struct ChangeVolume::FunctorMono {
  static void call(const T1& vol, double sample_rate, int64_t length, double* buf) {
    for (int64_t i = 0; i < length; ++i) {
      double val = ParData::Value(vol, i);
      buf[i] *= val;
    }
  }
};

template<class T1>
struct ChangeVolume::FunctorStereo {
  static void call(const T1& vol, double sample_rate, int64_t length, double* bufL, double* bufR) {
    for (int64_t i = 0; i < length; ++i) {
      double val = ParData::Value(vol, i);
      bufL[i] *= val;
      bufR[i] *= val;
    }
  }
};

void ChangeVolume::applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                                    const std::vector<arg_t> &arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(1, "multiplier");
  ParDeduce<FunctorMono>::call<1>(*arguments[0], sample_rate, length, buf->data_pointer);
}

void ChangeVolume::applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                                      const std::vector<arg_t> &arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(1, "multiplier");
  ParDeduce<FunctorStereo>::call<1>(*arguments[0], sample_rate, length, bufL->data_pointer, bufR->data_pointer);
}
