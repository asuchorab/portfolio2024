//
// Created by rutio on 2019-07-31.
//

#ifndef LUAMUSGEN_SATURATIONTANH_H
#define LUAMUSGEN_SATURATIONTANH_H

#include <transforms/Transform.h>

class SaturationTanh : public Transform {
public:
  void applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                        const std::vector<arg_t> &arguments) override;

  void applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                          const std::vector<arg_t> &arguments) override;
};

#endif //LUAMUSGEN_SATURATIONTANH_H
