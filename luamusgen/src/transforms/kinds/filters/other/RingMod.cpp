//
// Created by rutio on 2019-07-31.
//

#include "RingMod.h"
#include <parameters/ParDeduce.hpp>

void RingMod::applyMonoToMono(double sample_rate, int64_t length, buf_t in, buf_t out,
                                 const std::vector<arg_t> &arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(0, "");
  double* data_in = in->data_pointer;
  double* data_out = out->data_pointer;
  for (int64_t i = 0; i < length; ++i) {
    data_out[i] *= data_in[i];
  }
}

void
RingMod::applyStereoToStereo(double sample_rate, int64_t length, buf_t inL, buf_t inR,
                                buf_t outL, buf_t outR, const std::vector<arg_t> &arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(0, "");
  double* data_inL = inL->data_pointer;
  double* data_inR = inR->data_pointer;
  double* data_outL = outL->data_pointer;
  double* data_outR = outR->data_pointer;
  for (int64_t i = 0; i < length; ++i) {
    data_outL[i] *= data_inL[i];
    data_outR[i] *= data_inR[i];
  }
}
