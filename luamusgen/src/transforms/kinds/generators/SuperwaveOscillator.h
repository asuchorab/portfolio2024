//
// Created by rutio on 04.09.19.
//

#ifndef LUAMUSGEN_SUPERWAVE_H
#define LUAMUSGEN_SUPERWAVE_H

#include <transforms/Transform.h>
#include <parameters/ParDeduce.hpp>
#include <util/Oscillators.hpp>

template<class Oscillator>
class SuperwaveOscillator : public Transform {
  template<class T1, class T2, class T3>
  struct FunctorMono {
    static void call(const T1& freq, const T2& mix, const T3& detune, double phase, uint_fast32_t seed,
                     double sample_rate, int64_t length, double* buf) {
      oscil::OscilSuperFunctorMono<T1, T2, T3, Oscillator>::call(freq, mix, detune, phase, seed,
                                                                 sample_rate, length, buf);
    }
  };

  template<class T1, class T2, class T3>
  struct FunctorStereo {
    static void call(const T1& freq, const T2& mix, const T3& detune, double phase, uint_fast32_t seed,
                     double sample_rate, int64_t length, double* bufL, double* bufR) {
      oscil::OscilSuperFunctorStereo<T1, T2, T3, Oscillator>::call(freq, mix, detune, phase, seed,
                                                                   sample_rate, length, bufL, bufR);
    }
  };

public:
  explicit SuperwaveOscillator(double start_phase, uint_fast32_t seed) : start_phase(start_phase), seed(seed) {}

  void applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                        const std::vector<arg_t>& arguments) override {
    TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(3, "frequency, mix, detune");
    ParDeduce<FunctorMono>::template call<3>(*arguments[0], *arguments[1], *arguments[2], start_phase, seed,
                                             sample_rate, length, buf->data_pointer);
  }

  void applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                          const std::vector<arg_t>& arguments) override {
    TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(3, "frequency, mix, detune");
    ParDeduce<FunctorStereo>::template call<3>(*arguments[0], *arguments[1], *arguments[2], start_phase, seed,
                                               sample_rate, length, bufL->data_pointer, bufR->data_pointer);
  }

private:
  double start_phase;
  uint_fast32_t seed;
};

#endif //LUAMUSGEN_SUPERWAVE_H
