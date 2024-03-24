//
// Created by rutio on 12.08.2019.
//

#ifndef LUAMUSGEN_WaveOscillatorShapedSHAPED_H
#define LUAMUSGEN_WaveOscillatorShapedSHAPED_H

#include <transforms/Transform.h>
#include <parameters/ParDeduce.hpp>
#include <util/Oscillators.hpp>

template<class Oscillator>
class WaveOscillatorShaped : public Transform {
  template<class T1, class T2>
  struct FunctorMono {
    static void call(const T1& freq, const T2& shape, double phase, double sample_rate, int64_t length, double* buf) {
      oscil::OscilShapedFunctorMono<T1, T2, Oscillator>::call(freq, shape, phase, sample_rate, length, buf);
    }
  };

  template<class T1, class T2>
  struct FunctorStereo {
    static void call(const T1& freq, const T2& shape, double phase, double sample_rate, int64_t length, double* bufL,
                     double* bufR) {
      oscil::OscilShapedFunctorStereo<T1, T2, Oscillator>::call(freq, shape, phase, sample_rate, length, bufL, bufR);
    }
  };

public:
  explicit WaveOscillatorShaped(double start_phase) : start_phase(start_phase) {}

  void applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                        const std::vector<arg_t>& arguments) override {
    TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(2, "frequency, shape");
    ParDeduce<FunctorMono>::template call<2>(*arguments[0], *arguments[1], start_phase, sample_rate, length, buf->data_pointer);
  }

  void applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                          const std::vector<arg_t>& arguments) override {
    TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(2, "frequency, shape");
    ParDeduce<FunctorStereo>::template call<2>(*arguments[0], *arguments[1], start_phase, sample_rate, length,
                                      bufL->data_pointer, bufR->data_pointer);
  }

private:
  double start_phase;
};

#endif //LUAMUSGEN_WaveOscillatorShapedSHAPED_H
