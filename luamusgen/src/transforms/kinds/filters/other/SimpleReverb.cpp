//
// Created by rutio on 2019-07-31.
//

#include "SimpleReverb.h"
#include <array>
#include <parameters/ParDeduce.hpp>

SimpleReverb::SimpleReverb(double rel_length) : rel_length(rel_length) {}

template<class T1, class T2, class T3, class T4>
struct SimpleReverb::FunctorStereo {
  static void call(const T1& room_size, const T2& damping, const T3& wet_dry, const T4& space_width, double rel_length,
                   double sample_rate, int64_t length, double* bufL, double* bufR) {

    int64_t combDelaySamplesL[8] = {1617, 1557, 1491, 1422, 1356, 1277, 1188, 1116};
    int64_t combDelaySamplesR[8];
    int64_t allpassDelaySamplesL[4] = {225, 556, 441, 341};
    int64_t allpassDelaySamplesR[4];
    for (int64_t i = 0; i < 8; i++) {
      combDelaySamplesL[i] = (int64_t) ((double) combDelaySamplesL[i] * rel_length);
      combDelaySamplesR[i] = (int64_t) ((double) combDelaySamplesR[i] * rel_length);
    }
    for (int64_t i = 0; i < 4; i++) {
      allpassDelaySamplesL[i] = (int64_t) ((double) allpassDelaySamplesL[i] * rel_length);
      allpassDelaySamplesR[i] = (int64_t) ((double) allpassDelaySamplesR[i] * rel_length);
    }
    const int64_t stereoSpread = 23;
    const double fixedGain = 0.015;
    const double allpassCoefficent = 0.5;

    for (int64_t i = 0; i < 8; ++i) {
      combDelaySamplesR[i] = combDelaySamplesL[i] + stereoSpread;
    }
    for (int64_t i = 0; i < 4; ++i) {
      allpassDelaySamplesR[i] = allpassDelaySamplesL[i] + stereoSpread;
    }
    //damp *= scaleDamp;
    //room_size = (room_size * scaleRoom) + offsetRoom;

    //double ampWet = scaleWet * dry_wet;
    //double ampDry = scaleDry * (1.0-dry_wet);
    //ampWet /= (ampWet + ampDry);
    //ampDry /= (ampWet + ampDry);

    double damp, damp_comp, amp_dry, amp_wet, amp_wet1, amp_wet2;

    if (ParData::IsConst(damping)) {
      damp = ParData::Value(damping);
      damp_comp = 1 - damp;
    }

    if (ParData::IsConst(wet_dry)) {
      amp_wet = ParData::Value(wet_dry);
      amp_dry = 1 - amp_wet;

      if (ParData::IsConst(space_width)) {
        amp_wet1 = amp_wet * (0.5 + 0.5 * ParData::Value(space_width));
        amp_wet2 = amp_wet * (0.5 - 0.5 * ParData::Value(space_width));
      }
    }

    //COMB BUFFERS
    int64_t combDelayBufferSize = 0;
    for (int64_t i = 0; i < 8; i++) {
      if (combDelaySamplesL[i] > combDelayBufferSize)
        combDelayBufferSize = combDelaySamplesL[i];
      if (combDelaySamplesR[i] > combDelayBufferSize)
        combDelayBufferSize = combDelaySamplesR[i];
    }

//    not my code for the reverb algorithm but had to fix variable size arrays
//    double combDelayBufferL[8][combDelayBufferSize];
//    double combDelayBufferR[8][combDelayBufferSize];
    std::vector<double> combDelayBuffers;
    combDelayBuffers.resize(2 * 8 * combDelayBufferSize);
    double* combDelayBufferL[8];
    double* combDelayBufferR[8];
    double lowpassBufferL[8];
    double lowpassBufferR[8];
    for (int64_t i = 0; i < 8; i++) {
//      for (int64_t j = 0; j < combDelayBufferSize; j++) {
//        combDelayBufferL[i][j] = 0;
//        combDelayBufferR[i][j] = 0;
//      }
      combDelayBufferL[i] = combDelayBuffers.data() + i * combDelayBufferSize;
      combDelayBufferR[i] = combDelayBuffers.data() + (i + 8) * combDelayBufferSize;
      lowpassBufferL[i] = 0;
      lowpassBufferR[i] = 0;
    }
    int64_t cbuffL, cbuffR;
    double xLR1 = 0;
    //ALLPASS BUFFERS
    int64_t allpassDelayBufferSize = 0;
    for (int i = 0; i < 4; i++) {
      if (allpassDelaySamplesL[i] > allpassDelayBufferSize)
        allpassDelayBufferSize = allpassDelaySamplesL[i];
      if (allpassDelaySamplesR[i] > allpassDelayBufferSize)
        allpassDelayBufferSize = allpassDelaySamplesR[i];
    }

//    not my code for the reverb algorithm but had to fix variable size arrays
//    double allpassDelayBufferL[4][allpassDelayBufferSize];
//    double allpassDelayBufferR[4][allpassDelayBufferSize];
    std::vector<double> allpassDelayBuffers;
    allpassDelayBuffers.resize(2 * 4 * allpassDelayBufferSize);
    double* allpassDelayBufferL[4];
    double* allpassDelayBufferR[4];

    for (int64_t i = 0; i < 4; i++) {
//      for (int64_t j = 0; j < allpassDelayBufferSize; j++) {
//        allpassDelayBufferL[i][j] = 0;
//        allpassDelayBufferR[i][j] = 0;
//      }
      allpassDelayBufferL[i] = allpassDelayBuffers.data() + i * allpassDelayBufferSize;
      allpassDelayBufferR[i] = allpassDelayBuffers.data() + (i + 4) * allpassDelayBufferSize;
    }
    for (int64_t i = 0; i < length; ++i) {
      xLR1 = (bufL[i] + bufR[i]) * fixedGain;
      if (ParData::IsArray(damping)) {
        damp = ParData::Value(damping, i);
        damp_comp = 1 - damp;
      }
      if (ParData::IsArray(wet_dry)) {
        amp_wet = ParData::Value(wet_dry, i);
        amp_dry = 1 - amp_wet;
        amp_wet1 = amp_wet * (0.5 + 0.5 * ParData::Value(space_width, i));
        amp_wet2 = amp_wet * (0.5 - 0.5 * ParData::Value(space_width, i));
      } else if (ParData::IsArray(space_width)) {
        amp_wet1 = amp_wet * (0.5 + 0.5 * ParData::Value(space_width, i));
        amp_wet2 = amp_wet * (0.5 - 0.5 * ParData::Value(space_width, i));
      }
      double wetL = 0;
      double wetR = 0;
      for (int64_t j = 0; j < 8; ++j) {
        cbuffL = i % combDelaySamplesL[j];
        cbuffR = i % combDelaySamplesR[j];
        wetL += combDelayBufferL[j][cbuffL];
        wetR += combDelayBufferR[j][cbuffR];
        lowpassBufferL[j] = combDelayBufferL[j][cbuffL] * damp_comp + lowpassBufferL[j] * damp;
        lowpassBufferR[j] = combDelayBufferR[j][cbuffR] * damp_comp + lowpassBufferR[j] * damp;
        double lowpass_weight = ParData::Value(room_size, i);
        combDelayBufferL[j][cbuffL] = xLR1 + lowpassBufferL[j] * lowpass_weight;
        combDelayBufferR[j][cbuffR] = xLR1 + lowpassBufferR[j] * lowpass_weight;
      }
      for (int64_t j = 0; j < 4; ++j) {
        cbuffL = i % allpassDelaySamplesL[j];
        cbuffR = i % allpassDelaySamplesR[j];
        double x0L = wetL;
        double x0R = wetR;
        wetL = allpassCoefficent * allpassDelayBufferL[j][cbuffL] - x0L;
        wetR = allpassCoefficent * allpassDelayBufferR[j][cbuffR] - x0R;
        allpassDelayBufferL[j][cbuffL] = x0L + allpassCoefficent * allpassDelayBufferL[j][cbuffL];
        allpassDelayBufferR[j][cbuffR] = x0R + allpassCoefficent * allpassDelayBufferR[j][cbuffR];
      }
      bufL[i] = amp_dry * bufL[i] + amp_wet1 * wetL + amp_wet2 * wetR;
      bufR[i] = amp_dry * bufR[i] + amp_wet1 * wetR + amp_wet2 * wetL;
    }
  }
};

void SimpleReverb::applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                                      const std::vector<arg_t>& arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(4, "room size, damping, wet/dry, space width");
  ParDeduce<FunctorStereo>::call<4>(*arguments[0], *arguments[1], *arguments[2], *arguments[3], rel_length, sample_rate,
                                    length, bufL->data_pointer, bufR->data_pointer);
}
