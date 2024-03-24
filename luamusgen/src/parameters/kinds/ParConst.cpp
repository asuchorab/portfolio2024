//
// Created by rutio on 2019-05-15.
//

#include "ParConst.h"
#include <util/ClassUtil.h>
#include <util/MathUtil.h>


ParConst::ParConst(double value) : value(value) {
  if (!std::isfinite(value)) {
    logWarningC("value is not finite");
  }
}

std::unique_ptr<ParData>
ParConst::generate(const std::vector<const ParData*>& arguments, double sample_rate, int64_t start_point,
                   int64_t length) {
  return std::make_unique<ParDataConst>(value);
}

ParGenerator::MergeResult ParConst::merge(const ParGenerator& other) {
  if (classutil::compareIfCast<ParConst>(other,
      [this] (const ParConst& other) {
        return mathut::equals(value, other.value);
      })) {
    return FULL;
  } else {
    return FAILURE;
  }
}

double ParConst::getValue() const {
  return value;
}
