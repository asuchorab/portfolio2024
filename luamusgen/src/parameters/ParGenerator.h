//
// Created by rutio on 31.03.19.
//

#ifndef LUAMUSGEN_PARGENERATOR_H
#define LUAMUSGEN_PARGENERATOR_H


#include <memory>
#include "ParData.h"
#include <util/Logger.h>

/**
 * Base class for a parameter generator. An object of this interface represents
 * a certain mathematical function regardless of time interval or sample rate.
 */
class ParGenerator {
public:

  enum MergeResultType {
    FAILURE, FULL, PARTIAL
  };

  struct MergeResult {
    MergeResult(MergeResultType type, double new_start = 0.0): type(type), new_start(new_start) {}
    MergeResultType type;
    double new_start;
  };

  /**
   * Generate parameter data (constant value of array of samples) of the
   * parameter, with given sample rate, starting at given start_point with
   * duration of length (in samples).
   * @param arguments parameter data for arguments, should be called with
   *    aproprioate length and start_point
   * @param sample_rate sample rate of result and of arguments
   * @param start_point start point (in samples) of result and arguments
   * @param length length (in samples) of result and arguments
   * @return parameter data (constant or array) of computed parameter
   */
  virtual std::unique_ptr<ParData> generate(const std::vector<const ParData*>& arguments, double sample_rate,
                                            int64_t start_point, int64_t length) = 0;

  /**
   * Compare to other to determine if it can be substituted with this
   * @param other generator to compare to
   * @param start_point relative start point (in seconds)
   * @return FAILURE if generators represent different functions
   *         FULL if provided generator can be fully substituted with this
   *           with given start_point
 *           PARTIAL if provided generator can be substituted with this but
   *           with offset (given by MergeResult::new_start)
   */
  virtual MergeResult merge(const ParGenerator& other) = 0;

#define PARAMETER_VERIFY_ARGS(n, args_str) { \
  size_t actual = arguments.size(); \
  if (length == 0) { \
    logWarningC("length is 0"); \
    return std::make_unique<ParDataConst>(0, length, start_point); \
  } \
  if (actual != n) { \
    if (n == 0) { \
      logErrorC(("wrong number of arguments: expected " + std::to_string(n) + \
                ", got " + std::to_string(actual)).c_str() ); \
    } else { \
      logErrorC(("wrong number of arguments: expected " + std::to_string(n)+ " (" + args_str + ")" \
                ", got " + std::to_string(actual)).c_str() ); \
    } \
    return std::make_unique<ParDataConst>(0, length, start_point); \
  } \
}

#define PARAMETER_VERIFY_ARGS_EQUAL_LENGTH(n, args_str) { \
  PARAMETER_VERIFY_ARGS(n, args_str); \
  for (const ParData* p : arguments) { \
    if (p->length != length) { \
      logErrorC("all arguments are expected to have the same length as the parameter"); \
      return std::make_unique<ParDataConst>(0, length, start_point); \
    } \
  } \
}
};

#endif //LUAMUSGEN_PARGENERATOR_H
