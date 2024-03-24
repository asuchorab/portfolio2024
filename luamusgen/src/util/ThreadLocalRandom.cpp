//
// Created by rutio on 24.02.19.
//

#include "ThreadLocalRandom.h"

thread_local RandomWrapper<std::minstd_rand> ThreadLocalRandom::rand;