//
// Created by rutio on 2019-08-02.
//

#ifndef LUAMUSGEN_LEGACYSTEREO_H
#define LUAMUSGEN_LEGACYSTEREO_H

#include <transforms/Transform.h>

class LegacyStereo : public Transform {
public:
  void applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                          const std::vector<arg_t>& arguments) override;

  void applyStereoToStereo(double sample_rate, int64_t length, buf_t inL, buf_t inR,
                           buf_t outL, buf_t outR, const std::vector<arg_t>& arguments) override;

  void applyMonoToStereo(double sample_rate, int64_t length, buf_t in, buf_t outL, buf_t outR,
                         const std::vector<arg_t>& arguments) override;

  template<class T1> struct FunctorStereo;
  template<class T1> struct FunctorStereoToStereo;
  template<class T1> struct FunctorMonoToStereo;
};

#endif //LUAMUSGEN_LEGACYSTEREO_H
