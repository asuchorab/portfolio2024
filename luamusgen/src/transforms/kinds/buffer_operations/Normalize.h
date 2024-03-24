//
// Created by rutio on 2019-07-31.
//

#ifndef LUAMUSGEN_NORMALIZE_H
#define LUAMUSGEN_NORMALIZE_H

#include <transforms/Transform.h>

class Normalize : public Transform {
public:
  explicit Normalize(bool allow_volume_up = false, bool provide_feedback = false);

  void applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                        const std::vector<arg_t> &arguments) override;

  void applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                          const std::vector<arg_t> &arguments) override;

private:
  bool allow_volume_up;
  bool provide_feedback;
};

#endif //LUAMUSGEN_NORMALIZE_H
