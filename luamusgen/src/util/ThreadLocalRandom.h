//
// Created by rutio on 24.02.19.
//

#ifndef LUAMUSGEN_THREADLOCALRANDOM_H
#define LUAMUSGEN_THREADLOCALRANDOM_H


#include "RandomWrapper.hpp"

class ThreadLocalRandom {
public:
  static thread_local RandomWrapper<std::minstd_rand> rand;
};


#endif //LUAMUSGEN_THREADLOCALRANDOM_H
