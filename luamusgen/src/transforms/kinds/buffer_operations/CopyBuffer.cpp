//
// Created by rutio on 24.02.19.
//

#include "CopyBuffer.h"
#include <parameters/ParDeduce.hpp>
#include <util/MathUtil.h>

template<class T1>
struct CopyBuffer::FunctorMono {
  static void call(const T1& vol, double sample_rate, int64_t length, const double* in, double* out) {
    if (ParData::IsConst(vol) && mathut::equals(ParData::Value(vol), 1.0)) {
      for (int64_t i = 0; i < length; ++i) {
        out[i] += in[i];
      }
    } else {
      for (int64_t i = 0; i < length; ++i) {
        out[i] += in[i] * ParData::Value(vol, i);
      }
    }
  }
};

template<class T1>
struct CopyBuffer::FunctorStereo {
  static void call(const T1& vol, double sample_rate, int64_t length, const double* inL, const double* inR,
                   double* outL, double* outR) {
    if (ParData::IsConst(vol) && mathut::equals(ParData::Value(vol), 1.0)) {
      for (int64_t i = 0; i < length; ++i) {
        outL[i] += inL[i];
        outR[i] += inR[i];
      }
    } else {
      for (int64_t i = 0; i < length; ++i) {
        outL[i] += inL[i] * ParData::Value(vol, i);
        outR[i] += inR[i] * ParData::Value(vol, i);
      }
    }
  }
};

template<class T1>
struct CopyBuffer::FunctorMonoToStereo {
  static void call(const T1& vol, double sample_rate, int64_t length, const double* in, double* outL, double* outR) {
    if (ParData::IsConst(vol) && mathut::equals(ParData::Value(vol), 1.0)) {
      for (int64_t i = 0; i < length; ++i) {
        outL[i] += in[i];
        outR[i] += in[i];
      }
    } else {
      for (int64_t i = 0; i < length; ++i) {
        outL[i] += in[i] * ParData::Value(vol, i);
        outR[i] += in[i] * ParData::Value(vol, i);
      }
    }
  }
};

template<class T1>
struct CopyBuffer::FunctorStereoToMono {
  static void call(const T1& vol, double sample_rate, int64_t length, const double* inL, const double* inR,
                   double* out) {
    if (ParData::IsConst(vol) && mathut::equals(ParData::Value(vol), 1.0)) {
      for (int64_t i = 0; i < length; ++i) {
        out[i] += 0.5 * (inL[i] + inR[i]);
      }
    } else {
      for (int64_t i = 0; i < length; ++i) {
        out[i] += 0.5 * (inL[i] + inR[i]) * ParData::Value(vol, i);
      }
    }
  }
};

void CopyBuffer::applyMonoToMono(double sample_rate, int64_t length, buf_t in, buf_t out,
                                 const std::vector<arg_t>& arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(1, "multiplier");
  ParDeduce<FunctorMono>::call<1>(*arguments[0], sample_rate, length, in->data_pointer, out->data_pointer);
}

void CopyBuffer::applyStereoToStereo(double sample_rate, int64_t length, buf_t inL, buf_t inR, buf_t outL, buf_t outR,
                                     const std::vector<arg_t>& arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(1, "multiplier");
  ParDeduce<FunctorStereo>::call<1>(*arguments[0], sample_rate, length, inL->data_pointer, inR->data_pointer,
                                    outL->data_pointer, outR->data_pointer);
}

void CopyBuffer::applyMonoToStereo(double sample_rate, int64_t length, Transform::buf_t in, Transform::buf_t outL,
                                   Transform::buf_t outR, const std::vector<Transform::arg_t>& arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(1, "multiplier");
  ParDeduce<FunctorMonoToStereo>::call<1>(*arguments[0], sample_rate, length, in->data_pointer, outL->data_pointer,
                                          outR->data_pointer);
}

void CopyBuffer::applyStereoToMono(double sample_rate, int64_t length, Transform::buf_t inL, Transform::buf_t inR,
                                   Transform::buf_t out, const std::vector<Transform::arg_t>& arguments) {
  TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(1, "multiplier");
  ParDeduce<FunctorStereoToMono>::call<1>(*arguments[0], sample_rate, length, inL->data_pointer, inR->data_pointer,
                                          out->data_pointer);
}
