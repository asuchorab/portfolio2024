//
// Created by rutio on 23.02.19.
//

#ifndef LUAMUSGEN_DPARCONST_H
#define LUAMUSGEN_DPARCONST_H

#include "ParData.h"
#include <any>
#include <vector>
#include <algorithm>

/**
 * Refer to ParData.
 */
class ParDataConst final : public ParData {
public:
  ParDataConst(double val, int64_t samples = 1, int64_t start_sample = 0) {
    offset = start_sample;
    length = samples;
    data.value = val;
  }
  ParDataConst(const ParData& par) { *this = dynamic_cast<const ParDataConst&>(par); };
  ~ParDataConst() final = default;
};

#endif //LUAMUSGEN_DPARCONST_H
