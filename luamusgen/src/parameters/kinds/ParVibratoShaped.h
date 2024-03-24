//
// Created by rutio on 13.08.2019.
//

#ifndef LUAMUSGEN_PARVIBRATOSHAPED_H
#define LUAMUSGEN_PARVIBRATOSHAPED_H

#include <parameters/ParGenerator.h>
#include <parameters/ParDeduce.hpp>
#include <util/Oscillators.hpp>
#include <util/MathUtil.h>
#include <parameters/ParDataView.h>

template<class Oscillator>
class ParVibratoShaped : public ParGenerator {
  template<class T1, class T2, class T3, class T4>
  struct Functor {
    static std::unique_ptr<ParData>
    call(const T1& freq, const T2& shape, const T3& base, const T4& exp_amp, double start_phase,
         double sample_rate, int64_t start_point, int64_t length) {
      auto result = std::make_unique<ParDataArray>(length);
      double* out = result->data.array;
      double freq_conv = M_PI * 2.0 / sample_rate;

      int64_t freq_offset = 0;
      if (ParData::IsConst(freq)) {
        start_phase += start_point * ParData::Value(freq) * freq_conv;
      } else if (start_point > 0) {
        auto& freq_data = dynamic_cast<const ParDataView&>(freq);
        if (freq_data.offset != 0 && freq_data.length != start_point + length) {
          logError("ParVibrato::generate", "freq data is misplaced, add argument with touch_zero = true");
          return std::make_unique<ParDataConst>(ParData::Value(base));
        }
        for (int64_t i = 0; i < start_point; ++i) {
          start_phase += ParData::Value(freq, i) * freq_conv;
        }
        freq_offset = start_point;
      } else if (start_point < 0) {
        auto& freq_data = dynamic_cast<const ParDataView&>(freq);
        int64_t freq_end_point = freq_data.offset + freq_data.length;
        if (freq_data.offset != start_point && freq_end_point != std::max((int64_t) 0, start_point + length)) {
          logError("ParVibrato::generate", "freq data is misplaced, add argument with touch_zero = true");
          return std::make_unique<ParDataConst>(ParData::Value(base));
        }
        int64_t freq_pre_length = 0 - start_point;
        for (int64_t i = 0; i < freq_pre_length; ++i) {
          start_phase -= ParData::Value(freq, i) * freq_conv;
        }
      }

      Oscillator oscillator;
      oscillator.set(start_phase);
      if (ParData::IsConst(freq)) {
        double phase_diff = ParData::Value(freq) * freq_conv;
        for (int64_t i = 0; i < length; ++i) {
          out[i] = ParData::Value(base, i) *
                   std::pow(ParData::Value(exp_amp, i), oscillator.advance(phase_diff, ParData::Value(shape, i)));
        }
      } else {
        for (int64_t i = 0; i < length; ++i) {
          double phase_diff = ParData::Value(freq, freq_offset + i) * freq_conv;
          out[i] = ParData::Value(base, i) *
                   std::pow(ParData::Value(exp_amp, i), oscillator.advance(phase_diff, ParData::Value(shape, i)));
        }
      }
      return result;
    }
  };

public:
  explicit ParVibratoShaped(double start_phase) : start_phase(start_phase) {}

  std::unique_ptr<ParData> generate(const std::vector<const ParData*>& arguments, double sample_rate,
                                    int64_t start_point, int64_t length) override {
    PARAMETER_VERIFY_ARGS(4, "frequency, shape, base, exp_amp");
    if (arguments[1]->length != length) {
      logErrorC("shape (arg #2) is expected to have the same length as the parameter");
      return std::make_unique<ParDataConst>(0, length, start_point);
    }
    if (arguments[2]->length != length) {
      logErrorC("base (arg #3) is expected to have the same length as the parameter");
      return std::make_unique<ParDataConst>(0, length, start_point);
    }
    if (arguments[3]->length != length) {
      logErrorC("exp_amp (arg #4) is expected to have the same length as the parameter");
      return std::make_unique<ParDataConst>(0, length, start_point);
    }
    if (arguments[1]->offset != start_point) {
      logErrorC("shape (arg #2) is expected to have the same start point as the parameter");
      return std::make_unique<ParDataConst>(0, length, start_point);
    }
    if (arguments[2]->offset != start_point) {
      logErrorC("base (arg #3) is expected to have the same start point as the parameter");
      return std::make_unique<ParDataConst>(0, length, start_point);
    }
    if (arguments[3]->offset != start_point) {
      logErrorC("exp_amp (arg #4) is expected to have the same start point as the parameter");
      return std::make_unique<ParDataConst>(0, length, start_point);
    }
    return ParDeduce<Functor>::template callParData<4>(*arguments[0], *arguments[1], *arguments[2], *arguments[3],
                                                       start_phase, sample_rate, start_point, length);
  }

  MergeResult merge(const ParGenerator& other) override {
    if (auto p = dynamic_cast<const ParVibratoShaped*>(&other)) {
      if (mathut::equals(start_phase, p->start_phase)) {
        return FULL;
      }
    }
    return FAILURE;
  }

private:
  double start_phase;
};

#endif //LUAMUSGEN_PARVIBRATOSHAPED_H
