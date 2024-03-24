//
// Created by rutio on 26.02.19.
//

#include "ButterworthHighPass.h"
#include <parameters/ParDeduce.hpp>
#include <util/ButterworthFilters.hpp>


ButterworthHighPass::ButterworthHighPass(int64_t iterations)
    : iterations(iterations) {
  if (iterations <= 0) {
    logWarningC("iterations is nonpositive, setting iterations to 1");
  }
}

template<class T1, class T2, class T3>
struct ButterworthHighPass::FunctorMono {
  static void call(const T1& frequency, const T2& resonance, const T3& wetDry, int64_t iterations,
                   double sample_rate, int64_t length, double* buf) {
    butterworth::HighPassMono filter(sample_rate, iterations);

    if (ParData::IsConst(frequency)) {
      filter.updateFrequency(ParData::Value(frequency));
    }
    if (ParData::IsConst(wetDry)) {
      filter.updateWetDry(ParData::Value(wetDry));
    }

    for (int64_t i = 0; i < length; ++i) {
      if (ParData::IsArray(frequency)) {
        filter.updateFrequency(ParData::Value(frequency, i));
      }
      if (ParData::IsArray(wetDry)) {
        filter.updateWetDry(ParData::Value(wetDry, i));
      }
      filter.advance(buf[i], ParData::Value(resonance, i));
    }
  }
};


template<class T1, class T2, class T3>
struct ButterworthHighPass::FunctorStereo {
  static void call(const T1& frequency, const T2& resonance, const T3& wetDry, int64_t iterations,
                   double sample_rate, int64_t length, double* bufL, double* bufR) {
    butterworth::HighPassStereo filter(sample_rate, iterations);

    if (ParData::IsConst(frequency)) {
      filter.updateFrequency(ParData::Value(frequency));
    }
    if (ParData::IsConst(wetDry)) {
      filter.updateWetDry(ParData::Value(wetDry));
    }

    for (int64_t i = 0; i < length; ++i) {
      if (ParData::IsArray(frequency)) {
        filter.updateFrequency(ParData::Value(frequency, i));
      }
      if (ParData::IsArray(wetDry)) {
        filter.updateWetDry(ParData::Value(wetDry, i));
      }
      filter.advance(bufL[i], bufR[i], ParData::Value(resonance, i));
    }
  }
};

void ButterworthHighPass::applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                                           const std::vector<arg_t>& arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(3, "freq, resonance, wet/dry");
  ParDeduce<FunctorMono>::call<3>(*arguments[0], *arguments[1], *arguments[2], iterations, sample_rate, length,
                                  buf->data_pointer);
}

void ButterworthHighPass::applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                                             const std::vector<arg_t>& arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(3, "freq, resonance, wet/dry");
  ParDeduce<FunctorStereo>::call<3>(*arguments[0], *arguments[1], *arguments[2], iterations, sample_rate, length,
                                    bufL->data_pointer, bufR->data_pointer);
}
