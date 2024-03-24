//
// Created by rutio on 21.07.19.
//

#ifndef LUAMUSGEN_CONSTANTSIGNAL_H
#define LUAMUSGEN_CONSTANTSIGNAL_H

#include <transforms/Transform.h>

class ConstantSignal : public Transform {
public:
  void applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                        const std::vector<arg_t>& arguments) override;

  void applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                          const std::vector<arg_t>& arguments) override;

  template<class T1>
  struct FunctorMono;
  template<class T1>
  struct FunctorStereo;
};

#endif //LUAMUSGEN_CONSTANTSIGNAL_H
