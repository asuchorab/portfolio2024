//
// Created by rutio on 2019-07-28.
//

#ifndef LUAMUSGEN_PARLIMIT_H
#define LUAMUSGEN_PARLIMIT_H

#include <parameters/ParGenerator.h>

class ParLimit : public ParGenerator {
public:
  std::unique_ptr<ParData> generate(const std::vector<const ParData*>& arguments, double sample_rate,
                                    int64_t start_point, int64_t length) override;

  MergeResult merge(const ParGenerator& other) override;

private:
  template<class T1> struct Functor;
};

#endif //LUAMUSGEN_PARLIMIT_H
