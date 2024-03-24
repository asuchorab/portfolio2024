//
// Created by rutio on 12.08.2019.
//

#ifndef LUAMUSGEN_FLANGERSELFOSCILLATION_H
#define LUAMUSGEN_FLANGERSELFOSCILLATION_H

#include <transforms/Transform.h>

class FlangerSelfOscillation : public Transform {
public:
  explicit FlangerSelfOscillation(int64_t buffer_length);

  void applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                        const std::vector<arg_t>& arguments) override;

  void applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                          const std::vector<arg_t>& arguments) override;

private:
  size_t buffer_length;

  template<class T1, class T2, class T3>
  struct FunctorMono;
  template<class T1, class T2, class T3>
  struct FunctorStereo;
};

#endif //LUAMUSGEN_FLANGERSELFOSCILLATION_H
