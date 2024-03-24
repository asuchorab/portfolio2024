//
// Created by rutio on 27.07.19.
//

#ifndef LUAMUSGEN_PARCONCAT_H
#define LUAMUSGEN_PARCONCAT_H

#include <parameters/Parameter.h>

class ParConcat : public ParGenerator {

  std::unique_ptr<ParData> generate(const std::vector<const ParData*>& arguments, double sample_rate,
                                    int64_t start_point, int64_t length) override;

  MergeResult merge(const ParGenerator& other) override;
};


#endif //LUAMUSGEN_PARCONCAT_H
