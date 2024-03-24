//
// Created by rutio on 19.08.19.
//

#ifndef LUAMUSGEN_PARSUB_H
#define LUAMUSGEN_PARSUB_H

#include <parameters/Parameter.h>

class ParSub : public ParGenerator {
public:
  std::unique_ptr<ParData> generate(const std::vector<const ParData*>& arguments, double sample_rate,
                                    int64_t start_point, int64_t length) override;

  MergeResult merge(const ParGenerator& other) override;

private:
  template<class T1, class T2> struct Functor;
};

#endif //LUAMUSGEN_PARSUB_H
