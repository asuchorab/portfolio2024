//
// Created by rutio on 11.07.18.
//

#ifndef PXNM_RANDOM_H
#define PXNM_RANDOM_H


#include <random>
#include <chrono>

template <class RandomEngine, class seed_t = uint_fast32_t>
class RandomWrapper {
public:
  RandomWrapper(): seed_v() {
    auto t1 = std::chrono::high_resolution_clock::now();
    auto d1 = t1.time_since_epoch();
    seed_t s = (seed_t) (std::chrono::duration_cast<std::chrono::seconds>(d1).count()
               * std::chrono::duration_cast<std::chrono::nanoseconds>(d1).count());
    auto t2 = std::chrono::high_resolution_clock::now();
    auto d2 = t2 - t1;
    s = (seed_t) (s * std::chrono::duration_cast<std::chrono::nanoseconds>(d2).count());
    seed_v = s;
    engine.seed(s);
    eng_max_d_inv =  1.0 / ((double) engine.max() + 1.0);
    eng_max_f_inv = (float) eng_max_d_inv;
  }
  explicit RandomWrapper(seed_t seed): seed_v(seed) {
    engine.seed(seed);
    eng_max_d_inv = 1.0 / ((double) engine.max() + 1.0);
    eng_max_f_inv = (float) eng_max_d_inv;
  }
  seed_t getSeed() {
    return seed_v;
  }
  void seed(seed_t seed) {
    this->seed_v = seed;
    engine.seed(seed);
  }
  double randDouble() {
    return engine() * eng_max_d_inv;
  }
  double randDouble(double range) {
    return range * engine() * eng_max_d_inv;
  }
  double randDouble(double minimum, double maximum) {
    return minimum + (maximum - minimum) * engine() * eng_max_d_inv;
  }
  float randFloat() {
    return engine() * eng_max_f_inv;
  }
  float randFloat(float range) {
    return range * engine() * eng_max_f_inv;
  }
  float randFloat(float minimum, float maximum) {
    return minimum + (maximum - minimum) * engine() * eng_max_f_inv;
  }
  template<class int_t>
  int_t randInt(int_t range) {
    return engine() % range;
  }
  template<class int_t>
  int_t randInt(int_t minimum, int_t maximum) {
    return minimum + (engine() % (maximum - minimum));
  }
private:
  RandomEngine engine;
  seed_t seed_v;
  double eng_max_d_inv;
  float eng_max_f_inv;
};

typedef RandomWrapper<std::minstd_rand> RandomDefault;

#endif //PXNM_RANDOM_H
