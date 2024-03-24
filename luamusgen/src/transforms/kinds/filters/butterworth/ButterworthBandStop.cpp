//
// Created by rutio on 01.03.19.
//

#include "ButterworthBandStop.h"
#include <parameters/ParDeduce.hpp>
#include <util/ButterworthFilters.hpp>


ButterworthBandStop::ButterworthBandStop(int64_t iterations)
    : iterations(iterations) {
  if (iterations <= 0) {
    logWarningC("iterations is nonpositive, setting iterations to 1");
  }
}

template<class T1, class T2, class T3, class T4>
struct ButterworthBandStop::FunctorMono {
  static void
  call(const T1& frequency1, const T2& frequency2, const T3& resonance, const T4& wetDry, int64_t iterations,
       double sample_rate, int64_t length, double* buf) {
    butterworth::BandStopMono filter(sample_rate, iterations);

    if (ParData::IsConst(frequency1) && ParData::IsConst(frequency2)) {
      filter.updateFrequency(ParData::Value(frequency1), ParData::Value(frequency2));
    }
    if (ParData::IsConst(wetDry)) {
      filter.updateWetDry(ParData::Value(wetDry));
    }

    for (int64_t i = 0; i < length; ++i) {
      if (ParData::IsArray(frequency1) || ParData::IsArray(frequency2)) {
        filter.updateFrequency(ParData::Value(frequency1, i), ParData::Value(frequency2, i));
      }
      if (ParData::IsArray(wetDry)) {
        filter.updateWetDry(ParData::Value(wetDry, i));
      }
      filter.advance(buf[i], ParData::Value(resonance, i));
    }
  }
};


template<class T1, class T2, class T3, class T4>
struct ButterworthBandStop::FunctorStereo {
  static void
  call(const T1& frequency1, const T2& frequency2, const T3& resonance, const T4& wetDry, int64_t iterations,
       double sample_rate, int64_t length, double* bufL, double* bufR) {
    butterworth::BandStopStereo filter(sample_rate, iterations);

    if (ParData::IsConst(frequency1) && ParData::IsConst(frequency2)) {
      filter.updateFrequency(ParData::Value(frequency1), ParData::Value(frequency2));
    }
    if (ParData::IsConst(wetDry)) {
      filter.updateWetDry(ParData::Value(wetDry));
    }

    for (int64_t i = 0; i < length; ++i) {
      if (ParData::IsArray(frequency1) || ParData::IsArray(frequency2)) {
        filter.updateFrequency(ParData::Value(frequency1, i), ParData::Value(frequency2, i));
      }
      if (ParData::IsArray(wetDry)) {
        filter.updateWetDry(ParData::Value(wetDry, i));
      }
      filter.advance(bufL[i], bufR[i], ParData::Value(resonance, i));
    }
  }
};

void ButterworthBandStop::applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                                           const std::vector<arg_t>& arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(4, "freq1, freq2, resonance, wet/dry");
  ParDeduce<FunctorMono>::call<4>(*arguments[0], *arguments[1], *arguments[2], *arguments[3], iterations, sample_rate,
                                  length, buf->data_pointer);
}

void ButterworthBandStop::applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                                             const std::vector<arg_t>& arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(4, "freq1, freq2, resonance, wet/dry");
  ParDeduce<FunctorStereo>::call<4>(*arguments[0], *arguments[1], *arguments[2], *arguments[3], iterations, sample_rate,
                                    length, bufL->data_pointer, bufR->data_pointer);
}