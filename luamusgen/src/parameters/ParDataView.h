//
// Created by rutio on 31.03.19.
//

#ifndef LUAMUSGEN_PARVIEW_H
#define LUAMUSGEN_PARVIEW_H

#include "ParData.h"

/**
 * Refer to ParData.
 */
class ParDataView : public ParData  {
public:
  ParDataView(double* ptr, int64_t samples, int64_t start_sample = 0) {
    offset =  start_sample;
    length = samples;
    data.array = ptr;
  }
};


#endif //LUAMUSGEN_PARVIEW_H
