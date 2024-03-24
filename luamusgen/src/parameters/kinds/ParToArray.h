//
// Created by rutio on 2019-07-27.
//

#ifndef LUAMUSGEN_PARTOARRAY_H
#define LUAMUSGEN_PARTOARRAY_H

#include <parameters/ParGenerator.h>

class ParToArray : public ParGenerator {
public:
  std::unique_ptr<ParData> generate(const std::vector<const ParData*>& arguments, double sample_rate,
                                    int64_t start_point, int64_t length) override;

  MergeResult merge(const ParGenerator& other) override;
};


#endif //LUAMUSGEN_PARTOARRAY_H
