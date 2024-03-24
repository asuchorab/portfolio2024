//
// Created by rutio on 24.02.19.
//

#ifndef LUAMUSGEN_WHITENOISE_H
#define LUAMUSGEN_WHITENOISE_H

#include <transforms/Transform.h>
#include <util/RandomWrapper.hpp>

class WhiteNoise : public Transform {
public:
  explicit WhiteNoise(uint_fast32_t seed);

  void applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                        const std::vector<arg_t>& arguments) override;

  void applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                          const std::vector<arg_t>& arguments) override;

private:
  RandomWrapper<std::mt19937> random;
};


#endif //LUAMUSGEN_WHITENOISE_H
