//
// Created by rutio on 12.08.2019.
//

#ifndef LUAMUSGEN_WAVEOSCILLATOR_H
#define LUAMUSGEN_WAVEOSCILLATOR_H

#include <transforms/Transform.h>
#include <parameters/ParDeduce.hpp>
#include <util/Oscillators.hpp>

template<class Oscillator>
class WaveOscillator : public Transform {
  template<class T1>
  struct FunctorMono {
    static void call(const T1& freq, double phase, double sample_rate, int64_t length, double* buf) {
      oscil::OscilFunctorMono<T1, Oscillator>::call(freq, phase, sample_rate, length, buf);
    }
  };

  template<class T1>
  struct FunctorStereo {
    static void call(const T1& freq, double phase, double sample_rate, int64_t length, double* bufL, double* bufR) {
      oscil::OscilFunctorStereo<T1, Oscillator>::call(freq, phase, sample_rate, length, bufL, bufR);
    }
  };
/*
  class Step : public TransformStep {
  public:
    explicit Step(double start_phase, double sample_rate): freq_conv(M_PI * 2.0 / sample_rate) {
      oscillator.set(start_phase);
    }

    void stepMono(double sample_rate, int64_t length, double* buf, const std::vector<double>& arguments,
                  const std::vector<double>& interconnected, const std::vector<double>& additional_arguments) override {
      *buf += oscillator.advance(arguments[0] * freq_conv);
      for (auto v : interconnected) {
        oscillator.advancePhase(v);
      }
    }

    void stepStereo(double sample_rate, int64_t length, double* bufL, double* bufR,
                    const std::vector<double>& arguments, const std::vector<double>& interconnected,
                    const std::vector<double>& additional_arguments) override {

    }

  private:
    Oscillator oscillator;
    double freq_conv;
  };*/

public:
  explicit WaveOscillator(double start_phase) : start_phase(start_phase) {}

  void applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                        const std::vector<arg_t>& arguments) override {
    TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(1, "frequency");
    ParDeduce<FunctorMono>::template call<1>(*arguments[0], start_phase, sample_rate, length, buf->data_pointer);
  }

  void applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                          const std::vector<arg_t>& arguments) override {
    TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(1, "frequency");
    ParDeduce<FunctorStereo>::template call<1>(*arguments[0], start_phase, sample_rate, length, bufL->data_pointer,
                                               bufR->data_pointer);
  }

private:
  double start_phase;
};

#endif //LUAMUSGEN_WAVEOSCILLATOR_H
