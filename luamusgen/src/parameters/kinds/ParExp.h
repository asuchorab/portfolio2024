//
// Created by rutio on 01.03.19.
//

#ifndef LUAMUSGEN_PARPSEUDOEXPONENTIAL_H
#define LUAMUSGEN_PARPSEUDOEXPONENTIAL_H

#include <parameters/Parameter.h>

class ParExp : public ParGenerator {
public:
  std::unique_ptr<ParData> generate(const std::vector<const ParData*>& arguments, double sample_rate,
                                    int64_t start_point, int64_t length) override;

  MergeResult merge(const ParGenerator& other) override;

private:
  template<class T1, class T2> struct Functor;
};

#endif //LUAMUSGEN_PARPSEUDOEXPONENTIAL_H
