//
// Created by rutio on 2019-07-31.
//

#include "SaturationTanh.h"
#include <cmath>

void SaturationTanh::applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                                    const std::vector<arg_t> &arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(0, "");
  double* data = buf->data_pointer;
  for (int64_t i = 0; i < length; ++i) {
    data[i] = std::tanh(data[i]);
  }
}

void SaturationTanh::applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                                      const std::vector<arg_t> &arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(0, "");
  double* dataL = bufL->data_pointer;
  double* dataR = bufR->data_pointer;
  for (int64_t i = 0; i < length; ++i) {
    dataL[i] = std::tanh(dataL[i]);
    dataR[i] = std::tanh(dataR[i]);
  }
}

