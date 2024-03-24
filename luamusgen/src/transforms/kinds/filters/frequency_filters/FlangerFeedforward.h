//
// Created by rutio on 11.08.2019.
//

#ifndef LUAMUSGEN_FLANGERFEEDFORWARD_H
#define LUAMUSGEN_FLANGERFEEDFORWARD_H

#include <transforms/Transform.h>

class FlangerFeedforward : public Transform {
public:
  explicit FlangerFeedforward(int64_t buffer_length);

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

#endif //LUAMUSGEN_FLANGERFEEDFORWARD_H
