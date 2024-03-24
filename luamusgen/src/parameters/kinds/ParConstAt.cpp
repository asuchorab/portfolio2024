//
// Created by rutio on 31.08.19.
//

#include <util/MathUtil.h>
#include <parameters/ParData.h>
#include "ParConstAt.h"

std::unique_ptr<ParData> ParConstAt::generate(const std::vector<const ParData*>& arguments, double sample_rate,
                                              int64_t start_point, int64_t length) {
  PARAMETER_VERIFY_ARGS(1, "parameter");
  if (auto p = dynamic_cast<const ParDataConst*>(arguments[0])) {
    return std::make_unique<ParDataConst>(p->data.value, length);
  } else {
    if (length > 0) {
      return std::make_unique<ParDataConst>(arguments[0]->data.array[0], length);
    } else {
      logErrorC("length is 0");
      return std::make_unique<ParDataConst>(0, length);
    }
  }
}

ParGenerator::MergeResult ParConstAt::merge(const ParGenerator& other) {
  return FAILURE;
}
