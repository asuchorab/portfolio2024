//
// Created by rutio on 13.08.2019.
//

#ifndef LUAMUSGEN_MOOGHIGHPASS_H
#define LUAMUSGEN_MOOGHIGHPASS_H

#include <transforms/Transform.h>
#include <parameters/ParDeduce.hpp>
#include <util/MoogFilters.hpp>

template<class Saturation>
class MoogHighPass : public Transform {
public:
  template<class T1, class T2, class T3, class T4>
  struct FunctorMono {
    static void call(const T1& frequency, const T2& resonance, const T3& wetDry, const T4& inner_multiplier,
                     double sample_rate, int64_t length, double* buf) {
      moog::HighPassMono<Saturation> filter(sample_rate);

      if (ParData::IsConst(frequency)) {
        filter.updateFrequency(ParData::Value(frequency));
        if (ParData::IsConst(resonance)) {
          filter.updateResonance(ParData::Value(resonance));
        }
      }
      if (ParData::IsConst(wetDry)) {
        filter.updateWetDry(ParData::Value(wetDry));
      }

      for (int64_t i = 0; i < length; ++i) {
        if (ParData::IsArray(frequency)) {
          filter.updateFrequency(ParData::Value(frequency, i));
          filter.updateResonance(ParData::Value(resonance, i));
        } else if (ParData::IsArray(resonance)) {
          filter.updateResonance(ParData::Value(resonance, i));
        }
        if (ParData::IsArray(wetDry)) {
          filter.updateWetDry(ParData::Value(wetDry, i));
        }
        filter.advance(buf[i], ParData::Value(inner_multiplier, i));
      }
    }
  };

  template<class T1, class T2, class T3, class T4>
  struct FunctorStereo {
    static void call(const T1& frequency, const T2& resonance, const T3& wetDry, const T4& inner_multiplier,
                     double sample_rate, int64_t length, double* bufL, double* bufR) {
      moog::HighPassStereo<Saturation> filter(sample_rate);

      if (ParData::IsConst(frequency)) {
        filter.updateFrequency(ParData::Value(frequency));
        if (ParData::IsConst(resonance)) {
          filter.updateResonance(ParData::Value(resonance));
        }
      }
      if (ParData::IsConst(wetDry)) {
        filter.updateWetDry(ParData::Value(wetDry));
      }

      for (int64_t i = 0; i < length; ++i) {
        if (ParData::IsArray(frequency)) {
          filter.updateFrequency(ParData::Value(frequency, i));
          filter.updateResonance(ParData::Value(resonance, i));
        } else if (ParData::IsArray(resonance)) {
          filter.updateResonance(ParData::Value(resonance, i));
        }
        if (ParData::IsArray(wetDry)) {
          filter.updateWetDry(ParData::Value(wetDry, i));
        }
        filter.advance(bufL[i], bufR[i], ParData::Value(inner_multiplier, i));
      }
    }
  };

  void applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                        const std::vector<arg_t>& arguments) override {
    TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(4, "freq, resonance, wet/dry, inner multiplier");
    ParDeduce<FunctorMono>::template call<4>(*arguments[0], *arguments[1], *arguments[2], *arguments[3], sample_rate,
                                             length, buf->data_pointer);
  }


  void applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                          const std::vector<arg_t>& arguments) override {
    TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(4, "freq, resonance, wet/dry, inner multiplier");
    ParDeduce<FunctorStereo>::template call<4>(*arguments[0], *arguments[1], *arguments[2], *arguments[3], sample_rate,
                                               length, bufL->data_pointer, bufR->data_pointer);
  }

};

#endif //LUAMUSGEN_MOOGHIGHPASS_H
