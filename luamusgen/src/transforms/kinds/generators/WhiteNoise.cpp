//
// Created by rutio on 24.02.19.
//

#include <util/ThreadLocalRandom.h>
#include "WhiteNoise.h"

WhiteNoise::WhiteNoise(uint_fast32_t seed) {
  random.seed(seed);
}

void WhiteNoise::applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                                  const std::vector<arg_t>& arguments) {
  double* buf_arr = buf->data_pointer;
  for (int64_t i = 0; i < length; ++i) {
    buf_arr[i] += random.randDouble(-1, 1);
  }
}

void WhiteNoise::applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                                    const std::vector<arg_t>& arguments) {
  double* bufL_arr = bufL->data_pointer;
  double* bufR_arr = bufR->data_pointer;
  for (int64_t i = 0; i < length; ++i) {
    bufL_arr[i] += random.randDouble(-1, 1);
    bufR_arr[i] += random.randDouble(-1, 1);
  }
}
