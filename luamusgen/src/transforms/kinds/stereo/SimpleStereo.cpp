//
// Created by rutio on 2019-08-01.
//

#include "SimpleStereo.h"
#include <parameters/ParDeduce.hpp>

template<class T1>
struct SimpleStereo::FunctorStereo {
  static void call(const T1& pan, double sample_rate, int64_t length, double* bufL, double* bufR) {
    if (ParData::IsConst(pan)) {
      double p = ParData::Value(pan);
      double weightL = p >= 0 ? 1 - p : 1;
      double weightR = p <= 0 ? 1 + p : 1;
      for (int64_t i = 0; i < length; ++i) {
        bufL[i] *= weightL;
        bufR[i] *= weightR;
      }
    } else {
      for (int64_t i = 0; i < length; ++i) {
        double p = ParData::Value(pan, i);
        double weightL = p >= 0 ? 1 - p : 1;
        double weightR = p <= 0 ? 1 + p : 1;
        bufL[i] *= weightL;
        bufR[i] *= weightR;
      }
    }
  }
};

template<class T1>
struct SimpleStereo::FunctorStereoToStereo {
  static void call(const T1& pan, double sample_rate, int64_t length, const double* inL, const double* inR,
                   double* outL, double* outR) {
    if (ParData::IsConst(pan)) {
      double p = ParData::Value(pan);
      double weightL = p >= 0 ? 1 - p : 1;
      double weightR = p <= 0 ? 1 + p : 1;
      for (int64_t i = 0; i < length; ++i) {
        outL[i] += inL[i] * weightL;
        outR[i] += inR[i] * weightR;
      }
    } else {
      for (int64_t i = 0; i < length; ++i) {
        double p = ParData::Value(pan, i);
        double weightL = p >= 0 ? 1 - p : 1;
        double weightR = p <= 0 ? 1 + p : 1;
        outL[i] += inL[i] * weightL;
        outR[i] += inR[i] * weightR;
      }
    }
  }
};

template<class T1>
struct SimpleStereo::FunctorMonoToStereo {
  static void call(const T1& pan, double sample_rate, int64_t length, const double* in, double* outL, double* outR) {
    if (ParData::IsConst(pan)) {
      double p = ParData::Value(pan);
      double weightL = p >= 0 ? 1 - p : 1;
      double weightR = p <= 0 ? 1 + p : 1;
      for (int64_t i = 0; i < length; ++i) {
        outL[i] += in[i] * weightL;
        outR[i] += in[i] * weightR;
      }
    } else {
      for (int64_t i = 0; i < length; ++i) {
        double p = ParData::Value(pan, i);
        double weightL = p >= 0 ? 1 - p : 1;
        double weightR = p <= 0 ? 1 + p : 1;
        outL[i] += in[i] * weightL;
        outR[i] += in[i] * weightR;
      }
    }
  }
};

void SimpleStereo::applyStereoInPlace(double sample_rate, int64_t length, Transform::buf_t bufL, Transform::buf_t bufR,
                                      const std::vector<Transform::arg_t>& arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(1, "pan");
  ParDeduce<FunctorStereo>::call<1>(*arguments[0], sample_rate, length, bufL->data_pointer, bufR->data_pointer);
}

void SimpleStereo::applyStereoToStereo(double sample_rate, int64_t length, buf_t inL, buf_t inR, buf_t outL,
                                       buf_t outR, const std::vector<arg_t>& arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(1, "pan");
  ParDeduce<FunctorStereoToStereo>::call<1>(*arguments[0], sample_rate, length, inL->data_pointer, inR->data_pointer,
                                            outL->data_pointer, outR->data_pointer);
}

void SimpleStereo::applyMonoToStereo(double sample_rate, int64_t length, Transform::buf_t in, Transform::buf_t outL,
                                     Transform::buf_t outR, const std::vector<Transform::arg_t>& arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(1, "pan");
  ParDeduce<FunctorMonoToStereo>::call<1>(*arguments[0], sample_rate, length, in->data_pointer,
                                          outL->data_pointer, outR->data_pointer);
}
