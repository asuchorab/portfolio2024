//
// Created by rutio on 01.03.19.
//

#ifndef LUAMUSGEN_BUTTERWORTHBANDSTOP_H
#define LUAMUSGEN_BUTTERWORTHBANDSTOP_H

#include <transforms/Transform.h>

class ButterworthBandStop : public Transform {
public:
  explicit ButterworthBandStop(int64_t iterations);

  void applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                        const std::vector<arg_t>& arguments) override;

  void applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                          const std::vector<arg_t>& arguments) override;

private:
  int64_t iterations;

  template<class T1, class T2, class T3, class T4>
  struct FunctorMono;
  template<class T1, class T2, class T3, class T4>
  struct FunctorStereo;
};

#endif //LUAMUSGEN_BUTTERWORTHBANDSTOP_H
