//
// Created by rutio on 2019-07-28.
//

#include <parameters/ParDataArray.h>
#include <cstring>
#include <parameters/ParData.h>
#include "ParRepeat.h"

std::unique_ptr<ParData>
ParRepeat::generate(const std::vector<const ParData*>& arguments, double sample_rate, int64_t start_point,
                     int64_t length) {
  PARAMETER_VERIFY_ARGS(1, "parameter");
  if (auto p = dynamic_cast<const ParDataConst*>(arguments[0])) {
    return std::make_unique<ParDataConst>(p->data.value, length);
  } else {
    if (arguments[0]->length == 0) {
      logErrorC("parameter has no length, nothing to repeat")
      return std::make_unique<ParDataConst>(0, length);
    }

    auto result = std::make_unique<ParDataArray>(length);
    double* out = result->data.array;
    int64_t remaining_length = length;
    double* arg_data = arguments[0]->data.array;
    int64_t arg_length = arguments[0]->length;
    if (start_point < 0) {
      start_point = start_point - (start_point / arg_length - 1) * arg_length;
    }
    while (remaining_length > 0) {
      int64_t fill_length = std::min(arg_length, remaining_length);
      if (start_point > 0) {
        int64_t original_fill_length = fill_length;
        fill_length -= start_point;
        memcpy(out, arg_data + start_point, fill_length * sizeof(double));
        start_point -= original_fill_length;
      } else {
        memcpy(out, arg_data, fill_length * sizeof(double));
      }
      remaining_length -= fill_length;
      out += fill_length;
    }
    return result;
  }
}

ParGenerator::MergeResult ParRepeat::merge(const ParGenerator& other) {
  if (auto p = dynamic_cast<const ParRepeat*>(&other)) {
    return FULL;
  }
  return FAILURE;
}
