//
// Created by rutio on 2019-07-28.
//

#ifndef LUAMUSGEN_PARREPEAT_H
#define LUAMUSGEN_PARREPEAT_H

#include <parameters/ParGenerator.h>

class ParRepeat : public ParGenerator {
public:
  std::unique_ptr<ParData> generate(const std::vector<const ParData*>& arguments, double sample_rate,
                                    int64_t start_point, int64_t length) override;

  MergeResult merge(const ParGenerator& other) override;
};


#endif //LUAMUSGEN_PARREPEAT_H
