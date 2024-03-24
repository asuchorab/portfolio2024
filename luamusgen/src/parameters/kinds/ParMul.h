//
// Created by rutio on 28.02.19.
//

#ifndef LUAMUSGEN_PARMUL_H
#define LUAMUSGEN_PARMUL_H

#include <parameters/Parameter.h>

class ParMul : public ParGenerator {
public:
  std::unique_ptr<ParData> generate(const std::vector<const ParData*>& arguments, double sample_rate,
                                    int64_t start_point, int64_t length) override;

  MergeResult merge(const ParGenerator& other) override;

private:
  template<class T1, class T2> struct Functor;
};

#endif //LUAMUSGEN_PARMUL_H
