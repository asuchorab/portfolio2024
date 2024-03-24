//
// Created by rutio on 23.02.19.
//

#ifndef LUAMUSGEN_DPARLINEAR_H
#define LUAMUSGEN_DPARLINEAR_H

#include <parameters/ParGenerator.h>

class ParLerp : public ParGenerator {
public:
  ParLerp(double fullFirstTime, double fullSecondTime);

  std::unique_ptr<ParData> generate(const std::vector<const ParData*>& arguments, double sample_rate,
                                    int64_t start_point, int64_t length) override;

  MergeResult merge(const ParGenerator& other) override;

private:
  double full_first_time;
  double full_second_time;

  template<class T1, class T2> struct Functor;
};

#endif //LUAMUSGEN_DPARLINEAR_H
