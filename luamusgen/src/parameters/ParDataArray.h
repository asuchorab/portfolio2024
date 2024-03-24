//
// Created by rutio on 23.02.19.
//

#ifndef LUAMUSGEN_DPARARRAY_H
#define LUAMUSGEN_DPARARRAY_H

#include "ParData.h"

/**
 * Refer to ParData.
 */
class ParDataArray : public ParData {
public:
  explicit ParDataArray(int64_t samples, int64_t start_sample = 0) {
    offset = start_sample;
    length = samples;
    data.array = new double[samples];
  }
  ParDataArray(const ParDataArray& other) = delete;
  ParDataArray& operator=(const ParDataArray& other) = delete;
  ParDataArray(ParDataArray&& other) noexcept {
    offset = other.offset;
    length = other.length;
    data.array = other.data.array;
    other.data.array = nullptr;
  }
  ParDataArray& operator=(ParDataArray&& other) noexcept {
    offset = other.offset;
    length = other.length;
    data.array = other.data.array;
    other.data.array = nullptr;
    return *this;
  }
  ~ParDataArray() override { delete[] data.array; };

};

#endif //LUAMUSGEN_DPARARRAY_H
