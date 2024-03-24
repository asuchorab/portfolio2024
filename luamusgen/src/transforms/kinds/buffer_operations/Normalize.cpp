//
// Created by rutio on 2019-07-31.
//

#include <util/MathUtil.h>
#include "Normalize.h"

Normalize::Normalize(bool allow_volume_up, bool provide_feedback)
    : allow_volume_up(allow_volume_up), provide_feedback(provide_feedback) {}

void Normalize::applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                               const std::vector<arg_t> &arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(0, "");
  double* data = buf->data_pointer;
  double abs_max = 0;
  for (int64_t i = 0; i < length; ++i) {
    abs_max = std::max(abs_max, std::fabs(data[i]));
  }
  if (allow_volume_up || abs_max > 1.0) {
    if (abs_max > 0) {
      double inv = 1.0 / abs_max;
      for (int64_t i = 0; i < length; ++i) {
        data[i] *= inv;
      }
      if (provide_feedback) {
        double dB = mathut::amp_to_dB(inv);
        logInfoC("max absolute: %f, multiplied by %f (%s%fdB)", abs_max, inv, dB >= 0 ? "+" : "", dB);
      }
    } else if (provide_feedback) {
      logInfoC("Buffer is filled with zeros, nothing was changed");
    }
  } else if (provide_feedback) {
    logInfoC("max absolute value was %f and allow_volume_up was false", abs_max);
  }
}

void Normalize::applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                                 const std::vector<arg_t> &arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(0, "");
  double* dataL = bufL->data_pointer;
  double* dataR = bufR->data_pointer;
  double abs_max = 0;
  for (int64_t i = 0; i < length; ++i) {
    abs_max = std::max(abs_max, std::fabs(dataL[i]));
    abs_max = std::max(abs_max, std::fabs(dataR[i]));
  }

  if (allow_volume_up || abs_max > 1.0) {
    if (abs_max > 0) {
      double inv = 1.0 / abs_max;
      for (int64_t i = 0; i < length; ++i) {
        dataL[i] *= inv;
        dataR[i] *= inv;
      }
      if (provide_feedback) {
        double dB = mathut::amp_to_dB(inv);
        logInfoC("max absolute: %f, multiplied by %f (%s%fdB)", abs_max, inv, dB >= 0 ? "+" : "", dB);
      }
    } else if (provide_feedback) {
      logInfoC("Buffer is filled with zeros, nothing was changed");
    }
  } else if (provide_feedback) {
    logInfoC("max absolute value was %f and allow_volume_up was false", abs_max);
  }
}
