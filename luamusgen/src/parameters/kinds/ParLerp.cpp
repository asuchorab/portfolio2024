//
// Created by rutio on 23.02.19.
//

#include "ParLerp.h"
#include <parameters/ParDeduce.hpp>
#include <util/MathUtil.h>

template<class T1, class T2>
struct ParLerp::Functor {
  static std::unique_ptr<ParData> call(const T1& first, const T2& second, double full_first_time, double full_second_time,
                   double sample_rate, int64_t start_point, int64_t length) {
    if (ParData::IsConst(first) && ParData::IsConst(second)) {
      double val1 = ParData::Value(first);
      double val2 = ParData::Value(second);
      if (mathut::equals(val1, val2)) {
        return std::make_unique<ParDataConst>(val1);
      }
    }
    auto result = std::make_unique<ParDataArray>(length);
    double* out = result->data.array;

    double line_eq_a = 1.0 / (full_second_time - full_first_time);
    double line_eq_b = -full_first_time/line_eq_a;
    double start_weight = line_eq_a * start_point / sample_rate + line_eq_b;
    double d_weight = line_eq_a * length / sample_rate;
    for (int64_t i = 0; i < length; ++i) {
      double weight = start_weight + d_weight * i / length;
      double inv_weight = 1.0 - weight;
      out[i] = ParData::Value(first, i) * inv_weight + ParData::Value(second, i) * weight;
    }
    return result;
  }
};

ParLerp::ParLerp(double fullFirstTime, double fullSecondTime)
    : full_first_time(fullFirstTime), full_second_time(fullSecondTime) {
  if (mathut::equals(fullFirstTime, fullSecondTime)) {
    logWarningC("first time point is equal the second time point");
    full_second_time += 1.0;
  }
}

std::unique_ptr<ParData>
ParLerp::generate(const std::vector<const ParData*>& arguments, double sample_rate, int64_t start_point,
                    int64_t length) {
  PARAMETER_VERIFY_ARGS_EQUAL_LENGTH(2, "first, second");
  return ParDeduce<Functor>::callParData<2>(*arguments[0], *arguments[1], full_first_time, full_second_time,
      sample_rate, start_point, length);
}

ParGenerator::MergeResult ParLerp::merge(const ParGenerator& other) {
  if (auto p = dynamic_cast<const ParLerp*>(&other)) {
    double diff1 = full_first_time - p->full_first_time;
    double diff2 = full_second_time - p->full_second_time;
    if (mathut::equals(diff1, diff2)) {
      if (mathut::equals(diff1, 0)) {
        return FULL;
      } else {
        return ParGenerator::MergeResult(PARTIAL, diff1);
      }
    }
  }
  return FAILURE;
}

