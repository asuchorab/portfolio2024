//
// Created by rutio on 31.03.19.
//

#ifndef LUAMUSGEN_PARAMETER_H
#define LUAMUSGEN_PARAMETER_H


#include <memory>
#include <vector>
#include <mutex>
#include "ParGenerator.h"
#include "ParData.h"

/**
 * Structure that represents a parameter with particular sample rate and time
 * interval. It can either contain a parameter generator or point to another
 * parameter and use its data with an offset. You can also merge two parameters
 * to save computation time and memory (one of them only points to the other
 * one, possibly with an offset). Before parameter can be used, it has to be
 * computed (this prepares the data array/value).
 */
class Parameter {
public:
  /**
   * Parameter that has its own generator.
   */
  Parameter(std::unique_ptr<ParGenerator> generator, double sample_rate);

  /**
   * Parameter that points to another parameter with given offset.
   */
  Parameter(Parameter* view_target, double start_point);

  /**
   * Register time interval for computation. It guarantees that after
   * computing, parameter data from start_sample (using getData function)
   * will contain at least length samples.
   */
  void registerInterval(int64_t start_sample, int64_t length);

  /**
   * Attempt to merge with other. Its results are similar to
   * ParGenerator::merge
   * @return FAILURE if either this or other do not have generators (are views)
   *           or generators represent different functions
   *         PARTIAL if both generators represent the same function but with
   *           an offset, as a result, this will be transformed into a view
   *           pointing at other with an offset
   *         FULL if both generators represent the same function without any
   *           offset, in this case this will be transformed into a view
   *           pointing at other with offset = 0, additionally it means that
   *           this can be destroyed and other can be used instead
   */
  ParGenerator::MergeResultType merge(Parameter& other);

  /**
   * Compute the parameter, if the parameter has a generator, it uses the
   * generator to get a sample array of the parameter, if it's a view, it gets
   * view from its target at its offset.
   * @param arguments parameter data of arguments, they should start at the
   * right offset
   */
  void compute(const std::vector<const ParData*>& arguments);

  /**
   * Returns view to its data starting at start_sample.
   */
  std::unique_ptr<ParData> getData(int64_t start_sample, int64_t length) const;

  /**
   * Returns its sample rate.
   */
  double getSampleRate();

  /**
   * Returns its value if it has a constant value or qnan if it doesn't.
   */
  std::pair<bool, double> getConstValue();

  /**
   * Returns registered interval length in samples
   */
  int64_t getLengthSamples();

  /**
   * Returns starting point of registered time interval, in samples
   */
  int64_t getOffset();

  /**
   * Returns whether the parameter points to another
   */
  bool isView();

  /**
   * Returns type of the parameter generator/view
   */
  std::string getType();

protected:
  std::unique_ptr<ParData> data;
  std::unique_ptr<ParGenerator> generator;
  Parameter* view_target;
  double sample_rate;
  int64_t offset = std::numeric_limits<int64_t>::max();
  int64_t end_offset = 0;
};


#endif //LUAMUSGEN_PARAMETER_H
