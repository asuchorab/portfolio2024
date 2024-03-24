//
// Created by rutio on 2019-07-28.
//

#include "ParLimit.h"
#include <parameters/ParDeduce.hpp>
#include <util/MathUtil.h>

template<class T1>
struct ParLimit::Functor {
  static std::unique_ptr<ParData> call(const T1& par, double sample_rate, int64_t start_point, int64_t length) {
    if (ParData::IsConst(par)) {
      return std::make_unique<ParDataConst>(mathut::limit(ParData::Value(par)));
    }
    auto result = std::make_unique<ParDataArray>(length);
    double* out = result->data.array;

    for (int64_t i = 0; i < length; ++i) {
      out[i] = mathut::limit(ParData::Value(par, i));
    }
    return result;
  }
};

std::unique_ptr<ParData>
ParLimit::generate(const std::vector<const ParData*>& arguments, double sample_rate, int64_t start_point,
                  int64_t length) {
  PARAMETER_VERIFY_ARGS_EQUAL_LENGTH(1, "parameter");
  return ParDeduce<Functor>::callParData<1>(*arguments[0], sample_rate, start_point, length);
}

ParGenerator::MergeResult ParLimit::merge(const ParGenerator& other) {
  if (auto p = dynamic_cast<const ParLimit*>(&other)) {
    return FULL;
  }
  return FAILURE;
}