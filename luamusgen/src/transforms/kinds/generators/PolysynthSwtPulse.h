//
// Created by rutio on 01.09.19.
//

#ifndef LUAMUSGEN_POLYSYNTH_H
#define LUAMUSGEN_POLYSYNTH_H

#include <transforms/Transform.h>

class PolysynthSwtPulse : public Transform {
public:
  PolysynthSwtPulse(double start_phase_swt, double start_phase_pulse);
  void applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                        const std::vector<arg_t>& arguments) override;
  void applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                          const std::vector<arg_t>& arguments) override;
private:
  double start_phase_swt;
  double start_phase_pulse;
  template<class T1, class T2, class T3, class T4, class T5, class T6>
  struct FunctorMono;
  template<class T1, class T2, class T3, class T4, class T5, class T6>
  struct FunctorStereo;
};


#endif //LUAMUSGEN_POLYSYNTH_H
