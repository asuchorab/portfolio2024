//
// Created by rutio on 2019-07-31.
//

#include "SoftClipCompressor.h"
#include <util/MathUtil.h>
#include <parameters/ParDeduce.hpp>

template<class T1, class T2, class T3>
struct SoftClipCompressor::FunctorMono {
  static void call(const T1& linear_threshold, const T2& linear_amp, const T3& tanh_wet_dry,
                   double sample_rate, int64_t length, double* buf) {
    double threshold, threshold_comp, wet_dry, wet_dry_comp;
    if (ParData::IsConst(linear_threshold)) {
      threshold = ParData::Value(linear_threshold);
      threshold_comp = 1.0 - threshold;
    }
    if (ParData::IsConst(tanh_wet_dry)) {
      wet_dry = ParData::Value(tanh_wet_dry);
      wet_dry_comp = 1.0 - wet_dry;
    }
    for (int64_t i = 0; i < length; ++i) {
      if (!mathut::equals(buf[i], 0.0)) {
        if (ParData::IsArray(linear_threshold)) {
          threshold = ParData::Value(linear_threshold);
          threshold_comp = 1.0 - threshold;
        }
        if (ParData::IsArray(tanh_wet_dry)) {
          wet_dry = ParData::Value(tanh_wet_dry);
          wet_dry_comp = 1.0 - wet_dry;
        }
        if (buf[i] > threshold || buf[i] < -threshold) {
          double temp;
          if (buf[i] > 0.0) {
            temp = 1.0;
          } else {
            temp = -1.0;
          }
          double weight1 = (std::exp(std::tanh(std::log(std::fabs(buf[i]) - threshold)) * 0.5 + 0.5) - 1.0) *
                           (1.0 / (M_E - 1.0));
          double weight2 = 1.0 - weight1;
          double temp2 = buf[i] - temp * threshold;
          buf[i] = threshold_comp * wet_dry *
                   (weight1 * std::tanh(temp2) + weight2 * std::tanh(temp2 / threshold_comp))
                   + wet_dry_comp * temp2 + temp * threshold;
        }
        buf[i] *= ParData::Value(linear_amp);
      }
    }
  }
};

template<class T1, class T2, class T3>
struct SoftClipCompressor::FunctorStereo {
  static void call(const T1& linear_threshold, const T2& linear_amp, const T3& tanh_wet_dry,
                   double sample_rate, int64_t length, double* bufL, double* bufR) {
    double threshold, threshold_comp, wet_dry, wet_dry_comp;
    if (ParData::IsConst(linear_threshold)) {
      threshold = ParData::Value(linear_threshold);
      threshold_comp = 1.0 - threshold;
    }
    if (ParData::IsConst(tanh_wet_dry)) {
      wet_dry = ParData::Value(tanh_wet_dry);
      wet_dry_comp = 1.0 - wet_dry;
    }
    for (int64_t i = 0; i < length; ++i) {
      if (ParData::IsArray(linear_threshold)) {
        threshold = ParData::Value(linear_threshold);
        threshold_comp = 1.0 - threshold;
      }
      if (ParData::IsArray(tanh_wet_dry)) {
        wet_dry = ParData::Value(tanh_wet_dry);
        wet_dry_comp = 1.0 - wet_dry;
      }
      if (!mathut::equals(bufL[i], 0.0)) {
        if (bufL[i] > threshold || bufL[i] < -threshold) {
          double temp;
          if (bufL[i] > 0.0) {
            temp = 1.0;
          } else {
            temp = -1.0;
          }
          double weight1 = (std::exp(std::tanh(std::log(std::fabs(bufL[i]) - threshold)) * 0.5 + 0.5) - 1.0) *
                           (1.0 / (M_E - 1.0));
          double weight2 = 1.0 - weight1;
          double temp2 = bufL[i] - temp * threshold;
          bufL[i] = threshold_comp * wet_dry *
                     (weight1 * std::tanh(temp2) + weight2 * std::tanh(temp2 / threshold_comp))
                     + wet_dry_comp * temp2 + temp * threshold;
        }
        bufL[i] *= ParData::Value(linear_amp);;
      }

      if (!mathut::equals(bufR[i], 0.0)) {
        if (bufR[i] > threshold || bufR[i] < -threshold) {
          double temp;
          if (bufR[i] > 0.0) {
            temp = 1.0;
          } else {
            temp = -1.0;
          }
          double weight1 = (std::exp(std::tanh(std::log(std::fabs(bufR[i]) - threshold)) * 0.5 + 0.5) - 1.0) *
                           (1.0 / (M_E - 1.0));
          double weight2 = 1.0 - weight1;
          double temp2 = bufR[i] - temp * threshold;
          bufR[i] = threshold_comp * wet_dry *
                     (weight1 * std::tanh(temp2) + weight2 * std::tanh(temp2 / threshold_comp))
                     + wet_dry_comp * temp2 + temp * threshold;
        }
        bufR[i] *= ParData::Value(linear_amp);;
      }
    }
  }
};

void SoftClipCompressor::applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                                          const std::vector<arg_t>& arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(3, "linear_threshold, linear_amp, tanh_wet_dry");
  ParDeduce<FunctorMono>::call<3>(*arguments[0], *arguments[1], *arguments[2], sample_rate, length,
                                  buf->data_pointer);
}

void SoftClipCompressor::applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                                            const std::vector<arg_t>& arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(3, "linear_threshold, linear_amp, tanh_wet_dry");
  ParDeduce<FunctorStereo>::call<3>(*arguments[0], *arguments[1], *arguments[2], sample_rate, length,
                                    bufL->data_pointer, bufR->data_pointer);
  
}
