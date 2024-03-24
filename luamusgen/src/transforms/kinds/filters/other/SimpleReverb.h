//
// Created by rutio on 2019-07-31.
//

#ifndef LUAMUSGEN_SIMPLEREVERB_H
#define LUAMUSGEN_SIMPLEREVERB_H

#include <transforms/Transform.h>

class SimpleReverb : public Transform {
public:
  explicit SimpleReverb(double rel_length);

  void applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                          const std::vector<arg_t> &arguments) override;

private:
  double rel_length;
  template<class T1, class T2, class T3, class T4> struct FunctorStereo;
};

#endif //LUAMUSGEN_SIMPLEREVERB_H
