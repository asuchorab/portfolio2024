//
// Created by rutio on 06.09.19.
//

#ifndef LUAMUSGEN_SIMBREREVERSEREVERB_H
#define LUAMUSGEN_SIMBREREVERSEREVERB_H

#include <transforms/Transform.h>

class SimpleReverseReverb : public Transform {
public:
  explicit SimpleReverseReverb(double rel_length);

  void applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                          const std::vector<arg_t> &arguments) override;

private:
  double rel_length;
  template<class T1, class T2, class T3, class T4> struct FunctorStereo;
};

#endif //LUAMUSGEN_SIMBREREVERSEREVERB_H
