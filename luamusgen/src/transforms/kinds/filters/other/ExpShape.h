//
// Created by rutio on 2019-08-01.
//

#ifndef LUAMUSGEN_EXPSHAPE_H
#define LUAMUSGEN_EXPSHAPE_H

#include <transforms/Transform.h>

class ExpShape : public Transform {
public:
  void applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                        const std::vector<arg_t> &arguments) override;

  void applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                          const std::vector<arg_t> &arguments) override;

  template<class T1> struct FunctorMono;
  template<class T1> struct FunctorStereo;
};

#endif //LUAMUSGEN_EXPSHAPE_H
