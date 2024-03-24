//
// Created by rutio on 2019-07-31.
//

#ifndef LUAMUSGEN_LIMITER_H
#define LUAMUSGEN_LIMITER_H

#include <transforms/Transform.h>

class Limiter : public Transform {
public:
  void applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                        const std::vector<arg_t> &arguments) override;

  void applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                          const std::vector<arg_t> &arguments) override;

  template<class T1> struct FunctorMono;
  template<class T1> struct FunctorStereo;
};

#endif //LUAMUSGEN_LIMITER_H
