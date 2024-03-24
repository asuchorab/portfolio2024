//
// Created by rutio on 2019-08-02.
//

#include "LegacyStereo.h"
#include <parameters/ParDeduce.hpp>

template<class T1>
struct LegacyStereo::FunctorStereo {
  static void call(const T1& pan, double sample_rate, int64_t length, double* bufL, double* bufR) {
    if (ParData::IsConst(pan)) {
      double p = ParData::Value(pan);
      double rbias = 1.0 + p / (1.0 + std::abs(p));
      double weightLL = 2.5 - 1.5 * rbias;
      double weightRL = 0.5 * rbias - 0.5;
      double weightRR = 1.5 * rbias - 0.5;
      double weightLR = 0.5 - 0.5 * rbias;
      for (int64_t i = 0; i < length; ++i) {
        double memL = bufL[i];
        double memR = bufR[i];
        bufL[i] = memL * weightLL + memR * weightRL;
        bufR[i] = memR * weightRR + memL * weightLR;
      }
    } else {
      for (int64_t i = 0; i < length; ++i) {
        double p = ParData::Value(pan, i);
        double rbias = 1.0 + p / (1.0 + std::abs(p));
        double weightLL = 2.5 - 1.5 * rbias;
        double weightRL = 0.5 * rbias - 0.5;
        double weightRR = 1.5 * rbias - 0.5;
        double weightLR = 0.5 - 0.5 * rbias;
        double memL = bufL[i];
        double memR = bufR[i];
        bufL[i] = memL * weightLL + memR * weightRL;
        bufR[i] = memR * weightRR + memL * weightLR;
      }
    }
  }
};

template<class T1>
struct LegacyStereo::FunctorStereoToStereo {
  static void call(const T1& pan, double sample_rate, int64_t length, const double* inL, const double* inR,
                   double* outL, double* outR) {
    if (ParData::IsConst(pan)) {
      double p = ParData::Value(pan);
      double rbias = 1.0 + p / (1.0 + std::abs(p));
      double weightLL = 2.5 - 1.5 * rbias;
      double weightRL = 0.5 * rbias - 0.5;
      double weightRR = 1.5 * rbias - 0.5;
      double weightLR = 0.5 - 0.5 * rbias;
      for (int64_t i = 0; i < length; ++i) {
        outL[i] += inL[i] * weightLL + inR[i] * weightRL;
        outR[i] += inR[i] * weightRR + inL[i] * weightLR;
      }
    } else {
      for (int64_t i = 0; i < length; ++i) {
        double p = ParData::Value(pan, i);
        double rbias = 1.0 + p / (1.0 + std::abs(p));
        double weightLL = 2.5 - 1.5 * rbias;
        double weightRL = 0.5 * rbias - 0.5;
        double weightRR = 1.5 * rbias - 0.5;
        double weightLR = 0.5 - 0.5 * rbias;
        outL[i] += inL[i] * weightLL + inR[i] * weightRL;
        outR[i] += inR[i] * weightRR + inL[i] * weightLR;
      }
    }
  }
};

template<class T1>
struct LegacyStereo::FunctorMonoToStereo {
  static void call(const T1& pan, double sample_rate, int64_t length, const double* in, double* outL, double* outR) {
    if (ParData::IsConst(pan)) {
      double p = ParData::Value(pan);
      double rbias = 1.0 + p / (1.0 + std::abs(p));
      double lbias = 2.0 - 1.0 * rbias;
      for (int64_t i = 0; i < length; ++i) {
        outL[i] += in[i] * lbias;
        outR[i] += in[i] * rbias;
      }
    } else {
      for (int64_t i = 0; i < length; ++i) {
        double p = ParData::Value(pan, i);
        double rbias = 1.0 + p / (1.0 + std::abs(p));
        double lbias = 2.0 - 1.0 * rbias;
        outL[i] += in[i] * lbias;
        outR[i] += in[i] * rbias;
      }
    }
  }
};

void LegacyStereo::applyStereoInPlace(double sample_rate, int64_t length, Transform::buf_t bufL, Transform::buf_t bufR,
                                      const std::vector<Transform::arg_t>& arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(1, "pan");
  ParDeduce<FunctorStereo>::call<1>(*arguments[0], sample_rate, length, bufL->data_pointer, bufR->data_pointer);
}

void LegacyStereo::applyStereoToStereo(double sample_rate, int64_t length, buf_t inL, buf_t inR, buf_t outL,
                                       buf_t outR, const std::vector<arg_t>& arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(1, "pan");
  ParDeduce<FunctorStereoToStereo>::call<1>(*arguments[0], sample_rate, length, inL->data_pointer, inR->data_pointer,
                                            outL->data_pointer, outR->data_pointer);
}

void LegacyStereo::applyMonoToStereo(double sample_rate, int64_t length, Transform::buf_t in, Transform::buf_t outL,
                                     Transform::buf_t outR, const std::vector<Transform::arg_t>& arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(1, "pan");
  ParDeduce<FunctorMonoToStereo>::call<1>(*arguments[0], sample_rate, length, in->data_pointer,
                                          outL->data_pointer, outR->data_pointer);
}
