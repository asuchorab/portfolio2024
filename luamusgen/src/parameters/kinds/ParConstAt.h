//
// Created by rutio on 31.08.19.
//

#ifndef LUAMUSGEN_PARAT_H
#define LUAMUSGEN_PARAT_H

#include <parameters/ParGenerator.h>

class ParConstAt : public ParGenerator {
public:
  std::unique_ptr<ParData> generate(const std::vector<const ParData*>& arguments, double sample_rate,
                                    int64_t start_point, int64_t length) override;

  MergeResult merge(const ParGenerator& other) override;
};


#endif //LUAMUSGEN_PARAT_H
