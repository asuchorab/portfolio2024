//
// Created by rutio on 2019-07-31.
//

#ifndef LUAMUSGEN_SOFTCLIPCOMPRESSOR_H
#define LUAMUSGEN_SOFTCLIPCOMPRESSOR_H

#include <transforms/Transform.h>

class SoftClipCompressor : public Transform {
public:
  void applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                        const std::vector<arg_t> &arguments) override;

  void applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                          const std::vector<arg_t> &arguments) override;

  template<class T1, class T2, class T3> struct FunctorMono;
  template<class T1, class T2, class T3> struct FunctorStereo;
};

#endif //LUAMUSGEN_SOFTCLIPCOMPRESSOR_H
