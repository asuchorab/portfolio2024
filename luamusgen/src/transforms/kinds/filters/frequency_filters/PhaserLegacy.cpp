//
// Created by rutio on 2019-08-01.
//

#include "PhaserLegacy.h"
#include <parameters/ParDeduce.hpp>

PhaserLegacy::PhaserLegacy(int64_t stages) : stages(stages) {
  if (stages < 2) {
    logWarningC("%d stages phaser requested, which doesn't do anything");
  }
}

template<class T1, class T2, class T3>
struct PhaserLegacy::FunctorMono {
  static void call(const T1& coefficient, const T2& feedback, const T3& wetDry, int64_t stages,
                   double sample_rate, int64_t length, double* buf) {
    double w = 0;
    std::vector<double> x0(static_cast<size_t>(stages));
    std::vector<double> x1(static_cast<size_t>(stages));
    std::vector<double> y1(static_cast<size_t>(stages));

    double weight1, weight2;

    if (ParData::IsConst(wetDry)) {
      weight2 = ParData::Value(wetDry) * 0.5;
      weight1 = 1.0 - weight2;
    }

    for (int64_t i = 0; i < length; ++i) {
      w = buf[i] + w * ParData::Value(feedback, i);
      for (int64_t j = 0; j < stages; ++j) {
        x0[j] = w;
        w = x1[j] + ParData::Value(coefficient, i) * y1[j] - ParData::Value(coefficient, i) * w;
        x1[j] = x0[j];
        y1[j] = w;
      }
      if (ParData::IsArray(wetDry)) {
        weight2 = ParData::Value(wetDry, i) * 0.5;
        weight1 = 1.0 - weight2;
      }
      buf[i] = buf[i] * weight1 + w * weight2;
    }
  }
};

template<class T1, class T2, class T3>
struct PhaserLegacy::FunctorStereo {
  static void call(const T1& coefficient, const T2& feedback, const T3& wetDry, int64_t stages,
                   double sample_rate, int64_t length, double* bufL, double* bufR) {
    double wL = 0;
    std::vector<double> x0L(static_cast<size_t>(stages));
    std::vector<double> x1L(static_cast<size_t>(stages));
    std::vector<double> y1L(static_cast<size_t>(stages));
    double wR = 0;
    std::vector<double> x0R(static_cast<size_t>(stages));
    std::vector<double> x1R(static_cast<size_t>(stages));
    std::vector<double> y1R(static_cast<size_t>(stages));

    double weight1, weight2;

    if (ParData::IsConst(wetDry)) {
      weight2 = ParData::Value(wetDry) * 0.5;
      weight1 = 1.0 - weight2;
    }

    for (int64_t i = 0; i < length; ++i) {
      wL = bufL[i] + wL * ParData::Value(feedback, i);
      for (int64_t j = 0; j < stages; ++j) {
        x0L[j] = wL;
        wL = x1L[j] + ParData::Value(coefficient, i) * y1L[j] - ParData::Value(coefficient, i) * wL;
        x1L[j] = x0L[j];
        y1L[j] = wL;
      }
      wR = bufR[i] + wR * ParData::Value(feedback, i);
      for (int64_t j = 0; j < stages; ++j) {
        x0R[j] = wR;
        wR = x1R[j] + ParData::Value(coefficient, i) * y1R[j] - ParData::Value(coefficient, i) * wR;
        x1R[j] = x0R[j];
        y1R[j] = wR;
      }
      if (ParData::IsArray(wetDry)) {
        weight2 = ParData::Value(wetDry, i) * 0.5;
        weight1 = 1.0 - weight2;
      }
      bufL[i] = bufL[i] * weight1 + wL * weight2;
      bufR[i] = bufR[i] * weight1 + wR * weight2;
    }
  }
};

void PhaserLegacy::applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                                    const std::vector<arg_t>& arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(3, "coefficient, feedback, wet/dry");
  ParDeduce<FunctorMono>::call<3>(*arguments[0], *arguments[1], *arguments[2], stages, sample_rate, length,
                                  buf->data_pointer);
}

void PhaserLegacy::applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                                      const std::vector<arg_t>& arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(3, "coefficient, feedback, wet/dry");
  ParDeduce<FunctorStereo>::call<3>(*arguments[0], *arguments[1], *arguments[2], stages, sample_rate, length,
                                    bufL->data_pointer, bufR->data_pointer);
}
