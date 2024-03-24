//
// Created by rutio on 24.02.19.
//

#ifndef LUAMUSGEN_COPYBUFFER_H
#define LUAMUSGEN_COPYBUFFER_H

#include <transforms/Transform.h>

class CopyBuffer : public Transform {
public:
  void applyMonoToMono(double sample_rate, int64_t length, buf_t in, buf_t out,
                       const std::vector<arg_t>& arguments) override;

  void applyStereoToStereo(double sample_rate, int64_t length, buf_t inL, buf_t inR,
                           buf_t outL, buf_t outR, const std::vector<arg_t>& arguments) override;

  void applyMonoToStereo(double sample_rate, int64_t length, buf_t in, buf_t outL, buf_t outR,
                         const std::vector<arg_t>& arguments) override;

  void applyStereoToMono(double sample_rate, int64_t length, buf_t inL, buf_t inR, buf_t out,
                         const std::vector<arg_t>& arguments) override;

  template<class T1> struct FunctorMono;
  template<class T1> struct FunctorStereo;
  template<class T1> struct FunctorMonoToStereo;
  template<class T1> struct FunctorStereoToMono;
};

#endif //LUAMUSGEN_COPYBUFFER_H
