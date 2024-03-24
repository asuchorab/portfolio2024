//
// Created by rutio on 2019-07-27.
//

#include <parameters/ParDataArray.h>
#include <cstring>
#include "ParToArray.h"

std::unique_ptr<ParData>
ParToArray::generate(const std::vector<const ParData*>& arguments, double sample_rate, int64_t start_point,
                     int64_t length) {
  PARAMETER_VERIFY_ARGS_EQUAL_LENGTH(1, "parameter");
  if (auto p = dynamic_cast<const ParDataConst*>(arguments[0])) {
    double val = p->data.value;
    auto result = std::make_unique<ParDataArray>(length);
    double* out = result->data.array;
    for (int64_t i = 0; i < length; ++i) {
      out[i] = val;
    }
    return result;
  } else {
    logWarningC("The parameter is already in array form, the transform is pointless");
    auto result = std::make_unique<ParDataArray>(length);
    memcpy(result->data.array, arguments[0]->data.array, length * sizeof (double));
    return result;
  }
}

ParGenerator::MergeResult ParToArray::merge(const ParGenerator& other) {
  if (auto p = dynamic_cast<const ParToArray*>(&other)) {
    return FULL;
  }
  return FAILURE;
}
