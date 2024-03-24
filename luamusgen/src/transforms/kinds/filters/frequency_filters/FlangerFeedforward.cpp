//
// Created by rutio on 11.08.2019.
//

#include "FlangerFeedforward.h"
#include <parameters/ParDeduce.hpp>

FlangerFeedforward::FlangerFeedforward(int64_t buffer_length) : buffer_length(buffer_length) {
  if (buffer_length <= 0) {
    logWarningC("%d buffer length, must positive (set to 44100)");
    buffer_length = 44100;
  }
}

template<class T1, class T2>
struct FlangerFeedforward::FunctorMono {
  static void call(const T1& frequency, const T2& wetDry, int64_t buffer_length,
                   double sample_rate, int64_t length, double* buf) {
    std::vector<double> buffer(static_cast<size_t>(buffer_length));
    double weight1;
    double weight2;
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

      buffer[buffer_pos] = buf[i];
      int64_t index1 = buffer_pos + period_int;
      while (index1 >= buffer_length) {
        index1 -= buffer_length;
      }
      int64_t index2 = index1 + 1;
      if (index2 >= buffer_length) {
        index2 -= buffer_length;
      }
      buf[i] += ParData::Value(wetDry, i) * (buffer[index1] * weight1 + buffer[index2] * weight2);

      if(buffer_pos > 0) {
        buffer_pos--;
      } else {
        buffer_pos = buffer_length - 1;
      }
    }
  }
};

template<class T1, class T2>
struct FlangerFeedforward::FunctorStereo {
  static void call(const T1& frequency, const T2& wetDry, int64_t buffer_length,
                   double sample_rate, int64_t length, double* bufL, double* bufR) {
    std::vector<double> bufferL(static_cast<size_t>(buffer_length));
    std::vector<double> bufferR(static_cast<size_t>(buffer_length));
    double weight1;
    double weight2;
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

      bufferL[buffer_pos] = bufL[i];
      bufferR[buffer_pos] = bufR[i];
      int64_t index1 = buffer_pos + period_int;
      while (index1 >= buffer_length) {
        index1 -= buffer_length;
      }
      int64_t index2 = index1 + 1;
      if (index2 >= buffer_length) {
        index2 -= buffer_length;
      }
      bufL[i] += ParData::Value(wetDry, i) * (bufferL[index1] * weight1 + bufferL[index2] * weight2);
      bufR[i] += ParData::Value(wetDry, i) * (bufferR[index1] * weight1 + bufferR[index2] * weight2);

      if(buffer_pos > 0) {
        buffer_pos--;
      } else {
        buffer_pos = buffer_length - 1;
      }
    }
  }
};

void FlangerFeedforward::applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                                    const std::vector<arg_t>& arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(2, "frequency, wet/dry");
  ParDeduce<FunctorMono>::call<2>(*arguments[0], *arguments[1], buffer_length, sample_rate, length,
                                  buf->data_pointer);
}

void FlangerFeedforward::applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                                      const std::vector<arg_t>& arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(2, "frequency, wet/dry");
  ParDeduce<FunctorStereo>::call<2>(*arguments[0], *arguments[1], buffer_length, sample_rate, length,
                                    bufL->data_pointer, bufR->data_pointer);
}
