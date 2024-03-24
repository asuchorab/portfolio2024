//
// Created by rutio on 21.07.19.
//

#include "ConstantSignal.h"
#include <parameters/ParDeduce.hpp>

template<class T1>
struct ConstantSignal::FunctorMono {
  static void call(const T1& value, double sample_rate, int64_t length, double* buf) {
    if (ParData::IsConst(value)) {
      double val = ParData::Value(value);
      for (int64_t i = 0; i < length; ++i) {
        buf[i] += val;
      }
    } else {
      for (int64_t i = 0; i < length; ++i) {
        double val = ParData::Value(value, i);
        buf[i] += val;
      }
    }
  }
};

template<class T1>
struct ConstantSignal::FunctorStereo {
  static void call(const T1& value, double sample_rate, int64_t length, double* bufL, double* bufR) {
    if (ParData::IsConst(value)) {
      double val = ParData::Value(value);
      for (int64_t i = 0; i < length; ++i) {
        bufL[i] += val;
        bufR[i] += val;
      }
    } else {
      for (int64_t i = 0; i < length; ++i) {
        double val = ParData::Value(value, i);
        bufL[i] += val;
        bufR[i] += val;
      }
    }
  }
};

void ConstantSignal::applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                                      const std::vector<arg_t>& arguments) {
  size_t actual = arguments.size();
  if (length == 0) {
    logWarningC("length is 0");
    return;
  }
  if (actual > 1) {
    logErrorC(("wrong number of arguments: expected 0 or 1 (value), got " + std::to_string(actual)).c_str());
    return;
  }
  if (actual == 0) {
    double* buf_ptr = buf->data_pointer;
    for (int64_t i = 0; i < length; ++i) {
      buf_ptr[i] += 1;
    }
  } else {
    ParDeduce<FunctorMono>::call<1>(*arguments[0], sample_rate, length, buf->data_pointer);
  }
}

void ConstantSignal::applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                                        const std::vector<arg_t>& arguments) {
  size_t actual = arguments.size();
  if (length == 0) {
    logWarningC("length is 0");
    return;
  }
  if (actual > 1) {
    logErrorC(("wrong number of arguments: expected 0 or 1 (value), got " + std::to_string(actual)).c_str());
    return;
  }
  if (actual == 0) {
    double* bufL_ptr = bufL->data_pointer;
    double* bufR_ptr = bufR->data_pointer;
    for (int64_t i = 0; i < length; ++i) {
      bufL_ptr[i] += 1;
      bufR_ptr[i] += 1;
    }
  } else {
    ParDeduce<FunctorStereo>::call<1>(*arguments[0], sample_rate, length, bufL->data_pointer, bufR->data_pointer);
  }
}
