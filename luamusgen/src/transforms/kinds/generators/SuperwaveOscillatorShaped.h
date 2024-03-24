//
// Created by rutio on 04.09.19.
//

#ifndef LUAMUSGEN_SUPERWAVEOSCILLATORSHAPED_H
#define LUAMUSGEN_SUPERWAVEOSCILLATORSHAPED_H

#include <transforms/Transform.h>
#include <parameters/ParDeduce.hpp>
#include <util/Oscillators.hpp>

template<class Oscillator>
class SuperwaveOscillatorShaped : public Transform {
  template<class T1, class T2, class T3, class T4>
  struct FunctorMono {
    static void call(const T1& freq, const T2& mix, const T3& detune, const T4& shape, double phase, uint_fast32_t seed,
                     double sample_rate, int64_t length, double* buf) {
      oscil::OscilSuperShapedFunctorMono<T1, T2, T3, T4, Oscillator>::call(freq, mix, detune, shape, phase, seed,
                                                                           sample_rate, length, buf);
    }
  };

  template<class T1, class T2, class T3, class T4>
  struct FunctorStereo {
    static void call(const T1& freq, const T2& mix, const T3& detune, const T4& shape, double phase, uint_fast32_t seed,
                     double sample_rate, int64_t length, double* bufL, double* bufR) {
      oscil::OscilSuperShapedFunctorStereo<T1, T2, T3, T4, Oscillator>::call(freq, mix, detune, shape, phase, seed,
                                                                             sample_rate, length, bufL, bufR);
    }
  };

public:
  explicit SuperwaveOscillatorShaped(double start_phase, uint_fast32_t seed) : start_phase(start_phase), seed(seed) {}

  void applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                        const std::vector<arg_t>& arguments) override {
    TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(4, "frequency, mix, detune, shape");
    ParDeduce<FunctorMono>::template call<4>(*arguments[0], *arguments[1], *arguments[2], *arguments[3], start_phase,
                                             seed, sample_rate, length, buf->data_pointer);
  }

  void applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                          const std::vector<arg_t>& arguments) override {
    TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(4, "frequency, mix, detune, shape");
    ParDeduce<FunctorStereo>::template call<4>(*arguments[0], *arguments[1], *arguments[2], *arguments[3], start_phase,
                                               seed, sample_rate, length, bufL->data_pointer, bufR->data_pointer);
  }

private:
  double start_phase;
  uint_fast32_t seed;
};

#endif //LUAMUSGEN_SUPERWAVEOSCILLATORSHAPED_H
