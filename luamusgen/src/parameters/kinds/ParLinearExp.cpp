//
// Created by rutio on 01.03.19.
//

#include "ParLinearExp.h"
#include <parameters/ParDeduce.hpp>
#include <util/MathUtil.h>

template<class T1>
struct ParLinearExp::Functor {
  static std::unique_ptr<ParData> call(const T1& par, double v1, double v2,
                                       double sample_rate, int64_t start_point, int64_t length) {
    double mul_coeff = (std::log(v2/v1)) / (v2 - v1);
    double add_coeff = std::log(v1) - v1*mul_coeff;
    if (ParData::IsConst(par)) {
      return std::make_unique<ParDataConst>(std::exp(ParData::Value(par) * mul_coeff + add_coeff));
    }
    auto result = std::make_unique<ParDataArray>(length);
    double* out = result->data.array;

    for (int64_t i = 0; i < length; ++i) {
      out[i] = std::exp(ParData::Value(par, i) * mul_coeff + add_coeff);
    }
    return result;
  }
};

ParLinearExp::ParLinearExp(double v1, double v2)
    : v1(v1), v2(v2) {
  if (v1 <= 0) {
    logWarningC("both values must be positive (v1 = %f)", this->v1);
    this->v1 = 1;
  }
  if (v2 <= 0) {
    logWarningC("both values must be positive (v2 = %f)", this->v1);
    this->v2 = 1;
  }
  if (mathut::equals(v1, v2)) {
    logWarningC("first value is equal the second value (v1 = v2 = %f)", this->v1);
    this->v2 = v1*2;
  }
}

std::unique_ptr<ParData>
ParLinearExp::generate(const std::vector<const ParData*>& arguments, double sample_rate, int64_t start_point,
                  int64_t length) {
  PARAMETER_VERIFY_ARGS_EQUAL_LENGTH(1, "parameter");
  return ParDeduce<Functor>::callParData<1>(*arguments[0], v1, v2, sample_rate, start_point, length);
}

ParGenerator::MergeResult ParLinearExp::merge(const ParGenerator& other) {
  if (auto p = dynamic_cast<const ParLinearExp*>(&other)) {
    if (mathut::equals(v1, p->v1) && mathut::equals(v2, p->v2)) {
      return FULL;
    }
    if (mathut::equals(v1, p->v2) && mathut::equals(v2, p->v1)) {
      return FULL;
    }
  }
  return FAILURE;
}