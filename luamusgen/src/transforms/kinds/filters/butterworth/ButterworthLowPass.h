//
// Created by rutio on 25.02.19.
//

#ifndef LUAMUSGEN_BUTTERWORTHLOWPASS_H
#define LUAMUSGEN_BUTTERWORTHLOWPASS_H

#include <transforms/Transform.h>

class ButterworthLowPass : public Transform {
public:
  explicit ButterworthLowPass(int64_t iterations);

  void applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                        const std::vector<arg_t>& arguments) override;

  void applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                          const std::vector<arg_t>& arguments) override;

private:
  int64_t iterations;

  template<class T1, class T2, class T3>
  struct FunctorMono;
  template<class T1, class T2, class T3>
  struct FunctorStereo;
};

#endif //LUAMUSGEN_BUTTERWORTHLOWPASS_H
