//
// Created by rutio on 01.03.19.
//

#ifndef LUAMUSGEN_PARLINEAREXP_H
#define LUAMUSGEN_PARLINEAREXP_H

#include <parameters/ParGenerator.h>

class ParLinearExp : public ParGenerator {
public:
  ParLinearExp(double v1, double v2);

  std::unique_ptr<ParData> generate(const std::vector<const ParData*>& arguments, double sample_rate,
                                    int64_t start_point, int64_t length) override;

  MergeResult merge(const ParGenerator& other) override;

private:
  double v1;
  double v2;

  template<class T1> struct Functor;
};

#endif //LUAMUSGEN_PARLINEAREXP_H
