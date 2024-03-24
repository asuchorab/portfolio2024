//
// Created by rutio on 2019-07-31.
//

#ifndef LUAMUSGEN_RINGMOD_H
#define LUAMUSGEN_RINGMOD_H

#include <transforms/Transform.h>

class RingMod : public Transform {
public:
  void applyMonoToMono(double sample_rate, int64_t length, buf_t in, buf_t out,
                       const std::vector<arg_t>& arguments) override;

  void applyStereoToStereo(double sample_rate, int64_t length, buf_t inL, buf_t inR,
                           buf_t outL, buf_t outR, const std::vector<arg_t>& arguments) override;
};

#endif //LUAMUSGEN_RINGMOD_H
