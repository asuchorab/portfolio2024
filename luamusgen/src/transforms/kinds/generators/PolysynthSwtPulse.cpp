//
// Created by rutio on 01.09.19.
//

#include "PolysynthSwtPulse.h"
#include <parameters/ParDeduce.hpp>
#include <util/Oscillators.hpp>

template<class T1, class T2, class T3, class T4, class T5, class T6>
struct PolysynthSwtPulse::FunctorMono {
  static void call(const T1& freq_swt, const T2& freq_pulse, const T3& freq_mod_swt, const T4& freq_mod_pulse,
                   const T5& swt_pulse_bias, const T6& pulse_shape, double start_phase_swt, double start_phase_pulse,
                   double sample_rate, int64_t length, double* buf) {
    oscil::Swt osc1;
    osc1.set(start_phase_swt);
    oscil::Pulse osc2;
    osc2.set(start_phase_swt);
    double freq_conv = M_PI * 2.0 / sample_rate;

    double mem1 = 0, mem2 = 0;
    for (int64_t i = 0; i < length; ++i) {
      double dphase1 = ParData::Value(freq_swt, i) * freq_conv;
      double dphase2 = ParData::Value(freq_pulse, i) * freq_conv;
      double weight2 = ParData::Value(swt_pulse_bias, i);
      double weight1 = 1 - weight2;
      double temp = osc1.advance(dphase1 * (1.0 + ParData::Value(freq_mod_swt, i) * mem2));
      mem2 = osc2.advance(dphase2 * (1.0 + ParData::Value(freq_mod_pulse, i) * mem1), ParData::Value(pulse_shape, i));
      buf[i] += temp * weight1 + mem2 * weight2;
      mem1 = temp;
    }
  }
};

template<class T1, class T2, class T3, class T4, class T5, class T6>
struct PolysynthSwtPulse::FunctorStereo {
  static void call(const T1& freq_swt, const T2& freq_pulse, const T3& freq_mod_swt, const T4& freq_mod_pulse,
                   const T5& swt_pulse_bias, const T6& pulse_shape, double start_phase_swt, double start_phase_pulse,
                   double sample_rate, int64_t length, double* bufL, double* bufR) {
    oscil::Swt osc1;
    osc1.set(start_phase_swt);
    oscil::Pulse osc2;
    osc2.set(start_phase_swt);
    double freq_conv = M_PI * 2.0 / sample_rate;

    double mem1 = 0, mem2 = 0;
    for (int64_t i = 0; i < length; ++i) {
      double dphase1 = ParData::Value(freq_swt, i) * freq_conv;
      double dphase2 = ParData::Value(freq_pulse, i) * freq_conv;
      double weight2 = ParData::Value(swt_pulse_bias, i);
      double weight1 = 1 - weight2;
      double temp = osc1.advance(dphase1 * (1.0 + ParData::Value(freq_mod_swt, i) * mem2));
      mem2 = osc2.advance(dphase2 * (1.0 + ParData::Value(freq_mod_pulse, i) * mem1), ParData::Value(pulse_shape, i));
      double result = temp * weight1 + mem2 * weight2;
      bufL[i] += result;
      bufR[i] += result;
      mem1 = temp;
    }
  }
};

PolysynthSwtPulse::PolysynthSwtPulse(double start_phase_swt, double start_phase_pulse)
    : start_phase_swt(start_phase_swt), start_phase_pulse(start_phase_pulse) {}

void PolysynthSwtPulse::applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                                         const std::vector<arg_t>& arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(6,
                                     "freq_swt, freq_pulse, freq_mod_swt, freq_mod_pulse, swt_pulse_bias, pulse_shape");
  ParDeduce<FunctorMono>::call<6>(*arguments[0], *arguments[1], *arguments[2], *arguments[3], *arguments[4],
                                  *arguments[5], start_phase_swt, start_phase_pulse, sample_rate, length,
                                  buf->data_pointer);
}

void PolysynthSwtPulse::applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                                           const std::vector<arg_t>& arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(6,
                                     "freq_swt, freq_pulse, freq_mod_swt, freq_mod_pulse, swt_pulse_bias, pulse_shape");
  ParDeduce<FunctorStereo>::call<6>(*arguments[0], *arguments[1], *arguments[2], *arguments[3], *arguments[4],
                                    *arguments[5], start_phase_swt, start_phase_pulse, sample_rate, length,
                                    bufL->data_pointer, bufR->data_pointer);
}