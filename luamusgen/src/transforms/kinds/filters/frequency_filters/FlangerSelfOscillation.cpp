//
// Created by rutio on 12.08.2019.
//

#include "FlangerSelfOscillation.h"
#include <parameters/ParDeduce.hpp>

FlangerSelfOscillation::FlangerSelfOscillation(int64_t buffer_length) : buffer_length(buffer_length) {
  if (buffer_length <= 0) {
    logWarningC("%d buffer length, must positive (set to 44100)");
    buffer_length = 44100;
  }
}

template<class T1, class T2, class T3>
struct FlangerSelfOscillation::FunctorMono {
  static void call(const T1& frequency, const T2& oscil_length, const T3& correction, int64_t buffer_length,
                   double sample_rate, int64_t length, double* buf) {
    std::vector<double> buffer(static_cast<size_t>(buffer_length));
    double weight1;
    double weight2;
    double inv_sample_rate = 1.0/sample_rate;
    int64_t period_int;

    if (ParData::IsConst(frequency)) {
      if (ParData::Value(frequency) < 0) {
        weight2 = -sample_rate / ParData::Value(frequency);
      } else {
        weight2 = sample_rate / ParData::Value(frequency);
      }
      period_int = weight2;
      weight2 -= period_int;
      weight1 = 1.0 - weight2;
    }

    int64_t buffer_pos = 0;
    for (int64_t i = 0; i < length; ++i) {
      if (ParData::IsArray(frequency)) {
        if (ParData::Value(frequency, i) < 0) {
          weight2 = -sample_rate / ParData::Value(frequency, i);
        } else {
          weight2 = sample_rate / ParData::Value(frequency, i);
        }
        period_int = weight2;
        weight2 -= period_int;
        weight1 = 1.0 - weight2;
      }

      int64_t index1 = buffer_pos + period_int;
      while (index1 >= buffer_length) {
        index1 -= buffer_length;
      }
      int64_t index2 = index1 + 1;
      if (index2 >= buffer_length) {
        index2 -= buffer_length;
      }

      double coeff;
      if(ParData::Value(oscil_length, i) > 0.0) {
        coeff = 1 - (inv_sample_rate/ParData::Value(oscil_length, i)) * pow(sample_rate/ParData::Value(frequency, i), ParData::Value(correction, i));
      } else if(ParData::Value(oscil_length, i) < 0.0) {
        coeff = -1 - (inv_sample_rate/ParData::Value(oscil_length, i)) * pow(sample_rate/ParData::Value(frequency, i), ParData::Value(correction, i));
      } else {
        coeff = 0.0;
      }

      buf[i] += coeff * (buffer[index1] * weight1 + buffer[index2] * weight2);
      buffer[buffer_pos] = buf[i];

      if(buffer_pos > 0) {
        buffer_pos--;
      } else {
        buffer_pos = buffer_length - 1;
      }
    }
  }
};

template<class T1, class T2, class T3>
struct FlangerSelfOscillation::FunctorStereo {
  static void call(const T1& frequency, const T2& oscil_length, const T3& correction, int64_t buffer_length,
                   double sample_rate, int64_t length, double* bufL, double* bufR) {
    std::vector<double> bufferL(static_cast<size_t>(buffer_length));
    std::vector<double> bufferR(static_cast<size_t>(buffer_length));
    double weight1;
    double weight2;
    double inv_sample_rate = 1.0/sample_rate;
    int64_t period_int;

    if (ParData::IsConst(frequency)) {
      if (ParData::Value(frequency) < 0) {
        weight2 = -sample_rate / ParData::Value(frequency);
      } else {
        weight2 = sample_rate / ParData::Value(frequency);
      }
      period_int = weight2;
      weight2 -= period_int;
      weight1 = 1.0 - weight2;
    }

    int64_t buffer_pos = 0;
    for (int64_t i = 0; i < length; ++i) {
      if (ParData::IsArray(frequency)) {
        if (ParData::Value(frequency, i) < 0) {
          weight2 = -sample_rate / ParData::Value(frequency, i);
        } else {
          weight2 = sample_rate / ParData::Value(frequency, i);
        }
        period_int = (int64_t) weight2;
        weight2 -= (double) period_int;
        weight1 = 1.0 - weight2;
      }

      int64_t index1 = buffer_pos + period_int;
      while (index1 >= buffer_length) {
        index1 -= buffer_length;
      }
      int64_t index2 = index1 + 1;
      if (index2 >= buffer_length) {
        index2 -= buffer_length;
      }

      double coeff;
      if(ParData::Value(oscil_length, i) > 0.0) {
        coeff = 1 - (inv_sample_rate/ParData::Value(oscil_length, i)) * pow(sample_rate/ParData::Value(frequency, i), ParData::Value(correction, i));
      } else if(ParData::Value(oscil_length, i) < 0.0) {
        coeff = -1 - (inv_sample_rate/ParData::Value(oscil_length, i)) * pow(sample_rate/ParData::Value(frequency, i), ParData::Value(correction, i));
      } else {
        coeff = 0.0;
      }

      bufL[i] += coeff * (bufferL[index1] * weight1 + bufferL[index2] * weight2);
      bufR[i] += coeff * (bufferR[index1] * weight1 + bufferR[index2] * weight2);
      bufferL[buffer_pos] = bufL[i];
      bufferR[buffer_pos] = bufR[i];

      if(buffer_pos > 0) {
        buffer_pos--;
      } else {
        buffer_pos = buffer_length - 1;
      }
    }
  }
};

void FlangerSelfOscillation::applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                                       const std::vector<arg_t>& arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(3, "freq, oscil_length, correction");
  ParDeduce<FunctorMono>::call<3>(*arguments[0], *arguments[1], *arguments[2], buffer_length, sample_rate, length,
                                  buf->data_pointer);
}

void FlangerSelfOscillation::applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                                         const std::vector<arg_t>& arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(3, "freq, oscil_length, correction");
  ParDeduce<FunctorStereo>::call<3>(*arguments[0], *arguments[1], *arguments[2], buffer_length, sample_rate, length,
                                    bufL->data_pointer, bufR->data_pointer);
}
