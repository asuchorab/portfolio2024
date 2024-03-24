//
// Created by rutio on 2019-08-02.
//

#include "SpacialTest.h"
#include <parameters/ParDeduce.hpp>

template<class T1, class T2>
struct SpacialTest::FunctorStereo {
  static void call(const T1& lr, const T1& fb, double sample_rate, int64_t length, double* bufL, double* bufR) {
    if (ParData::IsConst(lr)) {
      double p = ParData::Value(lr);
      double rbias = 1.0 + p / (1.0 + std::abs(p));
      double mul = std::exp(-std::fabs(0.2 * p));
      double weightLL = (2.5 - 1.5 * rbias);
      double weightRL = 0.5 * rbias - 0.5;
      double weightRR = 1.5 * rbias - 0.5;
      double weightLR = 0.5 - 0.5 * rbias;
      for (int64_t i = 0; i < length; ++i) {
        double memL = bufL[i] * mul;
        double memR = bufR[i] * mul;
        bufL[i] = memL * weightLL + memR * weightRL;
        bufR[i] = memR * weightRR + memL * weightLR;
      }
    }
  }
};

template<class T1, class T2>
struct SpacialTest::FunctorMonoToStereo {
  static void call(const T1& lr, const T1& fb, double sample_rate, int64_t length, const double* in, double* outL, double* outR) {
    if (ParData::IsConst(lr)) {
      double p = ParData::Value(lr);
      double weightL = p >= 0 ? 1 - p : 1;
      double weightR = p <= 0 ? 1 + p : 1;
      for (int64_t i = 0; i < length; ++i) {
        outL[i] += in[i] * weightL;
        outR[i] += in[i] * weightR;
      }
    } else {
      for (int64_t i = 0; i < length; ++i) {
        double p = ParData::Value(lr, i);
        double weightL = p >= 0 ? 1 - p : 1;
        double weightR = p <= 0 ? 1 + p : 1;
        outL[i] += in[i] * weightL;
        outR[i] += in[i] * weightR;
      }
    }
  }
};

void SpacialTest::applyStereoInPlace(double sample_rate, int64_t length, Transform::buf_t bufL, Transform::buf_t bufR,
                                      const std::vector<Transform::arg_t>& arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(2, "left-right, front-back");
  ParDeduce<FunctorStereo>::call<2>(*arguments[0], *arguments[1], sample_rate, length,
                                    bufL->data_pointer, bufR->data_pointer);
}

void SpacialTest::applyMonoToStereo(double sample_rate, int64_t length, Transform::buf_t in, Transform::buf_t outL,
                                     Transform::buf_t outR, const std::vector<Transform::arg_t>& arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(2, "left-right, front-back");
  ParDeduce<FunctorMonoToStereo>::call<2>(*arguments[0], *arguments[1], sample_rate, length, in->data_pointer,
                                          outL->data_pointer, outR->data_pointer);
}

