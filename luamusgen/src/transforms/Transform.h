//
// Created by rutio on 26.04.19.
//

#ifndef LUAMUSGEN_TRANSFORM_H
#define LUAMUSGEN_TRANSFORM_H


#include <parameters/ParData.h>
#include <vector>
#include <util/Logger.h>
#include <buffer/Buffer.h>

/**
 * Class for all transforms, which are actions that can be made on
 */
class Transform {
public:
  enum Type {
    MONO_IN_PLACE,
    STEREO_IN_PLACE,
    MONO_TO_MONO,
    STEREO_TO_STEREO,
    MONO_TO_STEREO,
    STEREO_TO_MONO
  };

  typedef const Buffer::TransformData* buf_t;
  typedef const ParData* arg_t;

  virtual ~Transform() = default;
  void verifyAndApply(Type type, double sample_rate, int64_t length, const std::vector<buf_t>& buffers,
                      const std::vector<arg_t>& arguments);
  virtual void applyMonoInPlace(double sample_rate, int64_t length, buf_t buf, const std::vector<arg_t>& arguments);
  virtual void applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR, const std::vector<arg_t>& arguments);
  virtual void applyMonoToMono(double sample_rate, int64_t length, buf_t in, buf_t out, const std::vector<arg_t>& arguments);
  virtual void applyStereoToStereo(double sample_rate, int64_t length, buf_t inL, buf_t inR, buf_t outL, buf_t outR, const std::vector<arg_t>& arguments);
  virtual void applyMonoToStereo(double sample_rate, int64_t length, buf_t in, buf_t outL, buf_t outR, const std::vector<arg_t>& arguments);
  virtual void applyStereoToMono(double sample_rate, int64_t length, buf_t inL, buf_t inR, buf_t out, const std::vector<arg_t>& arguments);

  virtual void getStepMono();
  virtual void getStepStereo();


#define TRANSFORM_VERIFY_ARGS(n, args_str) { \
  size_t actual = arguments.size(); \
  if (length == 0) { \
    logWarningC("length is 0"); \
    return; \
  } \
  if (actual != n) { \
    if (n == 0) { \
      logErrorC(("wrong number of arguments: expected " + std::to_string(n) + \
                ", got " + std::to_string(actual)).c_str() ); \
    } else { \
      logErrorC(("wrong number of arguments: expected " + std::to_string(n)+ " (" + args_str + ")" \
                ", got " + std::to_string(actual)).c_str()); \
    } \
  return; } \
}

#define TRANSFORM_VERIFY_ARGS_EQUAL_LENGTH(n, args_str) { \
  TRANSFORM_VERIFY_ARGS(n, args_str); \
  for (const ParData* p : arguments) { \
    if (p->length != length) { \
      logErrorC("all arguments are expected to have the same length as the transform"); \
    } \
  } \
}

};


#endif //LUAMUSGEN_TRANSFORM_H
