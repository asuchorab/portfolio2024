//
// Created by rutio on 01.03.19.
//

#include "ParExp.h"
#include <parameters/ParDeduce.hpp>
#include <util/MathUtil.h>

template<class T1, class T2>
struct ParExp::Functor {
  static std::unique_ptr<ParData> call(const T1& first, const T2& second,
                                       double sample_rate, int64_t start_point, int64_t length) {
    if (ParData::IsConst(first) && ParData::IsConst(second)) {
      return std::make_unique<ParDataConst>(mathut::pseudoExponential(ParData::Value(first), ParData::Value(second)));
    }
    auto result = std::make_unique<ParDataArray>(length);
    double* out = result->data.array;
    for (int64_t i = 0; i < length; ++i) {
      out[i] = mathut::pseudoExponential(ParData::Value(first, i), ParData::Value(second, i));
    }
    return result;
  }
};

std::unique_ptr<ParData>
ParExp::generate(const std::vector<const ParData*>& arguments, double sample_rate, int64_t start_point,
                 int64_t length) {
  PARAMETER_VERIFY_ARGS_EQUAL_LENGTH(2, "first, second");
  return ParDeduce<Functor>::callParData<2>(*arguments[0], *arguments[1], sample_rate, start_point, length);
}

ParGenerator::MergeResult ParExp::merge(const ParGenerator& other) {
  if (auto p = dynamic_cast<const ParExp*>(&other)) {
    return FULL;
  }
  return FAILURE;
}
