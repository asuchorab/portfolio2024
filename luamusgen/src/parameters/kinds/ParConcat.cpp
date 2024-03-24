//
// Created by rutio on 27.07.19.
//

#include <sstream>
#include <cstring>
#include <parameters/ParDataArray.h>
#include "ParConcat.h"

std::unique_ptr<ParData>
ParConcat::generate(const std::vector<const ParData*>& arguments, double sample_rate, int64_t start_point,
                    int64_t length) {
  size_t args_size = arguments.size();
  if (args_size == 0) {
    logErrorC("wrong number of arguments: expected at least 1");
    return std::make_unique<ParDataConst>(0, length);
  }
  std::vector<std::pair<const ParData*, bool>> usable_arguments;
  //std::vector<std::pair<size_t, const ParDataConst*>> const_arguments;
  std::vector<std::pair<size_t, const ParData*>> bad_arguments;

  for (size_t i = 0; i < args_size; ++i) {
    auto arg = arguments[i];
    if (auto p = dynamic_cast<const ParDataConst*>(arg)) {
      usable_arguments.emplace_back(arg, true);
    } else {
      if (arg->length > 0) {
        usable_arguments.emplace_back(arg, false);
      } else {
        bad_arguments.emplace_back(i, arg);
      }
    }
  }

  std::stringstream warn;
  std::stringstream err;

  if (!bad_arguments.empty()) {
    warn << "received array-type arguments with no length: ";
    bool first = true;
    for (auto arg : bad_arguments) {
      if (first) {
        first = false;
      } else {
        warn << ", ";
      }
      warn << arg.first;
    }
    logWarningC(warn.str().c_str());
    warn.str("");
  }

  //size_t const_args_count = const_arguments.size();
  if (usable_arguments.empty()) {
    /*if (const_args_count == 0) {
      logErrorC("There are no positive length array arguments or const arguments");
      return std::make_unique<ParDataConst>(0, length);
    } else if (const_args_count == 1) {
      return std::make_unique<ParDataConst>(*const_arguments[1].second);
    } else {
      logErrorC("There are no positive length array arguments and there is more than one const argument");
      return std::make_unique<ParDataConst>(0, length);
    }*/
    logErrorC("There are no positive length array arguments or const arguments");
    return std::make_unique<ParDataConst>(0, length);
  } else {
    /*
    if (!const_arguments.empty()) {
      warn << "received constant arguments alongside valid array arguments: ";
      bool first = true;
      for (auto arg : bad_arguments) {
        if (first) {
          first = false;
        } else {
          warn << ", ";
        }
        warn << arg.first;
      }
      warn << "; as constant arguments have no length, they cannot be simply concatenated with other arguments, "
              "use ParToArray to convert them to an array with known size";
      logWarningC(warn.str());
    }*/

    auto result = std::make_unique<ParDataArray>(length);
    double* out = result->data.array;
    int64_t remaining_length = length;
    if (start_point < 0) {
      double fill_val;
      if (usable_arguments[0].second) {
        fill_val = usable_arguments[0].first->data.value;
      } else {
        fill_val = usable_arguments[0].first->data.array[0];
      }
      int64_t fill_length = std::min(-start_point, length);
      for (int64_t i = 0; i < fill_length; ++i) {
        out[i] = fill_val;
      }
      remaining_length -= fill_length;
      out += fill_length;
      start_point = 0;
    }
    size_t curr_arg = 0;
    size_t arg_num = usable_arguments.size();
    while (remaining_length > 0) {
      if (curr_arg < arg_num) {
        //double* arg_data = usable_arguments[curr_arg].first->data.array;
        int64_t fill_length = std::min(usable_arguments[curr_arg].first->length, remaining_length);
        if (start_point > 0) {
          int64_t original_fill_length = fill_length;
          fill_length -= start_point;
          if (fill_length >= 0) {
            //for (int64_t i = 0; i < fill_length; ++i) {
            //  double value = arg_data[start_point + 1];
            //  out[i] = value;
            //}
            if (usable_arguments[curr_arg].second) {
              double val = usable_arguments[curr_arg].first->data.value;
              for (int64_t i = 0; i < fill_length; ++i) {
                out[i] = val;
              }
            } else {
              memcpy(out, usable_arguments[curr_arg].first->data.array + start_point, fill_length * sizeof(double));
            }
          }
          start_point -= original_fill_length;
        } else {
          if (usable_arguments[curr_arg].second) {
            double val = usable_arguments[curr_arg].first->data.value;
            for (int64_t i = 0; i < fill_length; ++i) {
              out[i] = val;
            }
          } else {
            memcpy(out, usable_arguments[curr_arg].first->data.array, fill_length * sizeof(double));
          }
        }
        if (fill_length > 0) {
          out += fill_length;
          remaining_length -= fill_length;
        }
        curr_arg++;
      } else {
        double fill_val = out[-1];
        for (int64_t i = 0; i < remaining_length; ++i) {
          out[i] = fill_val;
        }
        remaining_length = 0;
      }
    }
    return result;
  }
}

ParGenerator::MergeResult ParConcat::merge(const ParGenerator& other) {
  if (auto p = dynamic_cast<const ParConcat*>(&other)) {
    return FULL;
  }
  return FAILURE;
}
