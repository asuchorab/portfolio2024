//
// Created by rutio on 12.08.2019.
//

#ifndef LUAMUSGEN_FLANGERFEEDBACK_H
#define LUAMUSGEN_FLANGERFEEDBACK_H

#include <transforms/Transform.h>

class FlangerFeedback : public Transform {
public:
  explicit FlangerFeedback(int64_t buffer_length);

  void applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                        const std::vector<arg_t>& arguments) override;

  void applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                          const std::vector<arg_t>& arguments) override;

private:
  size_t buffer_length;

  template<class T1, class T2>
  struct FunctorMono;
  template<class T1, class T2>
  struct FunctorStereo;
};

#endif //LUAMUSGEN_FLANGERFEEDBACK_H
