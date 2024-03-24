//
// Created by rutio on 2019-07-30.
//

#ifndef LUAMUSGEN_OSCILLATORS_HPP
#define LUAMUSGEN_OSCILLATORS_HPP

#include <cmath>
#include <random>
#include <parameters/ParDeduce.hpp>
#include "RandomWrapper.hpp"

namespace oscil {

  struct Sin {
    inline void set(double new_phase) {
      phase = new_phase - M_PI * 2 * std::floor(new_phase / (M_PI * 2));
      if (phase >= M_PI) {
        phase -= M_PI * 2;
      }
    }

    inline double advance(double dphase) {
      double value = std::sin(phase);
      phase += dphase;
      while (phase >= M_PI) {
        phase -= M_PI * 2;
      }
      return value;
    }

    inline void advancePhase(double dphase) {
      phase += dphase;
      while (phase >= M_PI) {
        phase -= M_PI * 2;
      }
    }

    double phase;
  };

  inline double sqr(double phase) {
    return phase >= 0 ? 1 : -1;
  }

  struct Sqr {
    inline void set(double new_phase) {
      phase = new_phase - M_PI * 2 * std::floor(new_phase / (M_PI * 2));
      if (phase >= M_PI) {
        phase -= M_PI * 2;
      }
    }

    inline double advance(double dphase) {
      double value = sqr(phase);
      phase += dphase;
      while (phase >= M_PI) {
        phase -= M_PI * 2;
      }
      return value;
    }

    inline void advancePhase(double dphase) {
      phase += dphase;
      while (phase >= M_PI) {
        phase -= M_PI * 2;
      }
    }

    double phase;
  };

  inline double swt(double phase) {
    return phase * (1.0 / M_PI);
  }

  struct Swt {
    inline void set(double new_phase) {
      phase = new_phase - M_PI * 2 * std::floor(new_phase / (M_PI * 2));
      if (phase >= M_PI) {
        phase -= M_PI * 2;
      }
    }

    inline double advance(double dphase) {
      double value = swt(phase);
      phase += dphase;
      while (phase >= M_PI) {
        phase -= M_PI * 2;
      }
      return value;
    }

    inline void advancePhase(double dphase) {
      phase += dphase;
      while (phase >= M_PI) {
        phase -= M_PI * 2;
      }
    }

    double phase;
  };

  inline double tri(double phase) {
    if (phase < -M_PI_2) {
      return -2.0 - phase * (2.0 / M_PI);
    } else if (phase > M_PI_2) {
      return 2.0 - phase * (2.0 / M_PI);
    } else {
      return phase * (2.0 / M_PI);
    }
  }

  struct Tri {
    inline void set(double new_phase) {
      phase = new_phase - M_PI * 2 * std::floor(new_phase / (M_PI * 2));
      if (phase >= M_PI) {
        phase -= M_PI * 2;
      }
    }

    inline double advance(double dphase) {
      double value = tri(phase);
      phase += dphase;
      while (phase >= M_PI) {
        phase -= M_PI * 2;
      }
      return value;
    }

    inline void advancePhase(double dphase) {
      phase += dphase;
      while (phase >= M_PI) {
        phase -= M_PI * 2;
      }
    }

    double phase;
  };

  inline double sqrtri(double phase) {
    if (phase >= 0) {
      return 1.0 - phase * (0.5 / M_PI);
    } else {
      return phase * (0.5 / M_PI) - 0.5;
    }
  }

  struct Sqrtri {
    inline void set(double new_phase) {
      phase = new_phase - M_PI * 2 * std::floor(new_phase / (M_PI * 2));
      if (phase >= M_PI) {
        phase -= M_PI * 2;
      }
    }

    inline double advance(double dphase) {
      double value = sqrtri(phase);
      phase += dphase;
      while (phase >= M_PI) {
        phase -= M_PI * 2;
      }
      return value;
    }

    inline void advancePhase(double dphase) {
      phase += dphase;
      while (phase >= M_PI) {
        phase -= M_PI * 2;
      }
    }

    double phase;
  };

  inline double pulse(double phase, double shape) {
    return phase < shape * M_PI ? 1.0 : -1.0;
  }

  struct Pulse {
    inline void set(double new_phase) {
      phase = new_phase - M_PI * 2 * std::floor(new_phase / (M_PI * 2));
    }

    inline double advance(double dphase, double shape) {
      double value = pulse(phase, shape);
      phase += dphase;
      while (phase >= M_PI * 2) {
        phase -= M_PI * 2;
      }
      return value;
    }

    inline void advancePhase(double dphase) {
      phase += dphase;
      while (phase >= M_PI * 2) {
        phase -= M_PI * 2;
      }
    }

    double phase;
  };

  template<class FreqType, class Oscillator>
  struct OscilFunctorMono {
    inline static void call(const FreqType& freq, double phase, double sample_rate, int64_t length, double* buf) {
      double freq_conv = M_PI * 2.0 / sample_rate;
      Oscillator oscillator;
      oscillator.set(phase);
      if (ParData::IsConst(freq)) {
        double phase_diff = ParData::Value(freq) * freq_conv;
        for (int64_t i = 0; i < length; ++i) {
          buf[i] += oscillator.advance(phase_diff);
        }
      } else {
        for (int64_t i = 0; i < length; ++i) {
          double phase_diff = ParData::Value(freq, i) * freq_conv;
          buf[i] += oscillator.advance(phase_diff);
        }
      }
    }
  };

  template<class FreqType, class Oscillator>
  struct OscilFunctorStereo {
    inline static void
    call(const FreqType& freq, double phase, double sample_rate, int64_t length, double* bufL, double* bufR) {
      double freq_conv = M_PI * 2.0 / sample_rate;
      Oscillator oscillator;
      oscillator.set(phase);
      if (ParData::IsConst(freq)) {
        double phase_diff = ParData::Value(freq) * freq_conv;
        for (int64_t i = 0; i < length; ++i) {
          double value = oscillator.advance(phase_diff);
          bufL[i] += value;
          bufR[i] += value;
        }
      } else {
        for (int64_t i = 0; i < length; ++i) {
          double phase_diff = ParData::Value(freq, i) * freq_conv;
          double value = oscillator.advance(phase_diff);
          bufL[i] += value;
          bufR[i] += value;
        }
      }
    }
  };

  template<class FreqType, class ShapeType, class Oscillator>
  struct OscilShapedFunctorMono {
    inline static void
    call(const FreqType& freq, const ShapeType& shape, double phase, double sample_rate, int64_t length, double* buf) {
      double freq_conv = M_PI * 2.0 / sample_rate;
      Oscillator oscillator;
      oscillator.set(phase);
      if (ParData::IsConst(freq)) {
        double phase_diff = ParData::Value(freq) * freq_conv;
        for (int64_t i = 0; i < length; ++i) {
          buf[i] += oscillator.advance(phase_diff, ParData::Value(shape, i));
        }
      } else {
        for (int64_t i = 0; i < length; ++i) {
          double phase_diff = ParData::Value(freq, i) * freq_conv;
          buf[i] += oscillator.advance(phase_diff, ParData::Value(shape, i));
        }
      }
    }
  };

  template<class FreqType, class ShapeType, class Oscillator>
  struct OscilShapedFunctorStereo {
    inline static void
    call(const FreqType& freq, const ShapeType& shape, double phase, double sample_rate, int64_t length, double* bufL,
         double* bufR) {
      double freq_conv = M_PI * 2.0 / sample_rate;
      Oscillator oscillator;
      oscillator.set(phase);
      if (ParData::IsConst(freq)) {
        double phase_diff = ParData::Value(freq) * freq_conv;
        for (int64_t i = 0; i < length; ++i) {
          double value = oscillator.advance(phase_diff, ParData::Value(shape, i));
          bufL[i] += value;
          bufR[i] += value;
        }
      } else {
        for (int64_t i = 0; i < length; ++i) {
          double phase_diff = ParData::Value(freq, i) * freq_conv;
          double value = oscillator.advance(phase_diff, ParData::Value(shape, i));
          bufL[i] += value;
          bufR[i] += value;
        }
      }
    }
  };

  constexpr double MIX_MUL = 0.1428571428571429;
  constexpr double MIX_COMP_MUL = 0.8571428571428571;
  constexpr double MUL_A1 = 0.181;
  constexpr double MUL_A2 = 0.572;
  constexpr double MUL_A3 = 1.0;
  constexpr double MUL_B1 = -0.17;
  constexpr double MUL_B2 = -0.55;
  constexpr double MUL_B3 = -0.981;

  template<class FreqType, class MixType, class DetuneType, class Oscillator>
  struct OscilSuperFunctorMono {
    inline static void call(const FreqType& freq, const MixType& mix, const DetuneType& detune, double phase,
                            uint_fast32_t seed, double sample_rate, int64_t length, double* buf) {
      double freq_conv = M_PI * 2.0 / sample_rate;
      RandomWrapper<std::mt19937> random(seed);
      Oscillator oscillator;
      Oscillator oscillatorA1;
      Oscillator oscillatorA2;
      Oscillator oscillatorA3;
      Oscillator oscillatorB1;
      Oscillator oscillatorB2;
      Oscillator oscillatorB3;
      oscillator.set(phase);
      oscillatorA1.set(random.randDouble(M_PI * 2));
      oscillatorA2.set(random.randDouble(M_PI * 2));
      oscillatorA3.set(random.randDouble(M_PI * 2));
      oscillatorB1.set(random.randDouble(M_PI * 2));
      oscillatorB2.set(random.randDouble(M_PI * 2));
      oscillatorB3.set(random.randDouble(M_PI * 2));
      if (ParData::IsConst(freq)) {
        double phase_diff = ParData::Value(freq) * freq_conv;
        for (int64_t i = 0; i < length; ++i) {
          double m = ParData::Value(mix, i);
          double d = phase_diff * ParData::Value(detune, i);
          buf[i] += m * MIX_MUL *
                    (oscillatorA1.advance(phase_diff + d * MUL_A1)
                     + oscillatorA2.advance(phase_diff + d * MUL_A2)
                     + oscillatorA3.advance(phase_diff + d * MUL_A3)
                     + oscillatorB1.advance(phase_diff + d * MUL_B1)
                     + oscillatorB2.advance(phase_diff + d * MUL_B2)
                     + oscillatorB3.advance(phase_diff + d * MUL_B3)
                    )
                    + (1.0 - m * MIX_COMP_MUL) * oscillator.advance(phase_diff);
        }
      } else {
        for (int64_t i = 0; i < length; ++i) {
          double phase_diff = ParData::Value(freq, i) * freq_conv;
          double m = ParData::Value(mix, i);
          double d = phase_diff * ParData::Value(detune, i);
          buf[i] += m * MIX_MUL *
                    (oscillatorA1.advance(phase_diff + d * MUL_A1)
                     + oscillatorA2.advance(phase_diff + d * MUL_A2)
                     + oscillatorA3.advance(phase_diff + d * MUL_A3)
                     + oscillatorB1.advance(phase_diff + d * MUL_B1)
                     + oscillatorB2.advance(phase_diff + d * MUL_B2)
                     + oscillatorB3.advance(phase_diff + d * MUL_B3)
                    )
                    + (1.0 - m * MIX_COMP_MUL) * oscillator.advance(phase_diff);
        }
      }
    }
  };

  template<class FreqType, class MixType, class DetuneType, class Oscillator>
  struct OscilSuperFunctorStereo {
    inline static void call(const FreqType& freq, const MixType& mix, const DetuneType& detune, double phase,
                            uint_fast32_t seed, double sample_rate, int64_t length, double* bufL, double* bufR) {
      double freq_conv = M_PI * 2.0 / sample_rate;
      RandomWrapper<std::mt19937> random(seed);
      Oscillator oscillator;
      Oscillator oscillatorA1;
      Oscillator oscillatorA2;
      Oscillator oscillatorA3;
      Oscillator oscillatorB1;
      Oscillator oscillatorB2;
      Oscillator oscillatorB3;
      oscillator.set(phase);
      oscillatorA1.set(random.randDouble(M_PI * 2));
      oscillatorA2.set(random.randDouble(M_PI * 2));
      oscillatorA3.set(random.randDouble(M_PI * 2));
      oscillatorB1.set(random.randDouble(M_PI * 2));
      oscillatorB2.set(random.randDouble(M_PI * 2));
      oscillatorB3.set(random.randDouble(M_PI * 2));
      if (ParData::IsConst(freq)) {
        double phase_diff = ParData::Value(freq) * freq_conv;
        for (int64_t i = 0; i < length; ++i) {
          double m = ParData::Value(mix, i);
          double d = phase_diff * ParData::Value(detune, i);
          double value = m * MIX_MUL *
                         (oscillatorA1.advance(phase_diff + d * MUL_A1)
                          + oscillatorA2.advance(phase_diff + d * MUL_A2)
                          + oscillatorA3.advance(phase_diff + d * MUL_A3)
                          + oscillatorB1.advance(phase_diff + d * MUL_B1)
                          + oscillatorB2.advance(phase_diff + d * MUL_B2)
                          + oscillatorB3.advance(phase_diff + d * MUL_B3)
                         )
                         + (1.0 - m * MIX_COMP_MUL) * oscillator.advance(phase_diff);
          bufL[i] += value;
          bufR[i] += value;
        }
      } else {
        for (int64_t i = 0; i < length; ++i) {
          double phase_diff = ParData::Value(freq, i) * freq_conv;
          double m = ParData::Value(mix, i);
          double d = phase_diff * ParData::Value(detune, i);
          double value = m * MIX_MUL *
                         (oscillatorA1.advance(phase_diff + d * MUL_A1)
                          + oscillatorA2.advance(phase_diff + d * MUL_A2)
                          + oscillatorA3.advance(phase_diff + d * MUL_A3)
                          + oscillatorB1.advance(phase_diff + d * MUL_B1)
                          + oscillatorB2.advance(phase_diff + d * MUL_B2)
                          + oscillatorB3.advance(phase_diff + d * MUL_B3)
                         )
                         + (1.0 - m * MIX_COMP_MUL) * oscillator.advance(phase_diff);
          bufL[i] += value;
          bufR[i] += value;
        }
      }
    }
  };

  template<class FreqType, class MixType, class DetuneType, class ShapeType, class Oscillator>
  struct OscilSuperShapedFunctorMono {
    inline static void call(const FreqType& freq, const MixType& mix, const DetuneType& detune, const ShapeType& shape,
                            double phase, uint_fast32_t seed, double sample_rate, int64_t length, double* buf) {
      double freq_conv = M_PI * 2.0 / sample_rate;
      RandomWrapper<std::mt19937> random(seed);
      Oscillator oscillator;
      Oscillator oscillatorA1;
      Oscillator oscillatorA2;
      Oscillator oscillatorA3;
      Oscillator oscillatorB1;
      Oscillator oscillatorB2;
      Oscillator oscillatorB3;
      oscillator.set(phase);
      oscillatorA1.set(random.randDouble(M_PI * 2));
      oscillatorA2.set(random.randDouble(M_PI * 2));
      oscillatorA3.set(random.randDouble(M_PI * 2));
      oscillatorB1.set(random.randDouble(M_PI * 2));
      oscillatorB2.set(random.randDouble(M_PI * 2));
      oscillatorB3.set(random.randDouble(M_PI * 2));
      if (ParData::IsConst(freq)) {
        double phase_diff = ParData::Value(freq) * freq_conv;
        for (int64_t i = 0; i < length; ++i) {
          double m = ParData::Value(mix, i);
          double d = phase_diff * ParData::Value(detune, i);
          double s = ParData::Value(shape, i);
          buf[i] += m * MIX_MUL *
                    (oscillatorA1.advance(phase_diff + d * MUL_A1, s)
                     + oscillatorA2.advance(phase_diff + d * MUL_A2, s)
                     + oscillatorA3.advance(phase_diff + d * MUL_A3, s)
                     + oscillatorB1.advance(phase_diff + d * MUL_B1, s)
                     + oscillatorB2.advance(phase_diff + d * MUL_B2, s)
                     + oscillatorB3.advance(phase_diff + d * MUL_B3, s)
                    )
                    + (1.0 - m * MIX_COMP_MUL) * oscillator.advance(phase_diff, s);
        }
      } else {
        for (int64_t i = 0; i < length; ++i) {
          double phase_diff = ParData::Value(freq, i) * freq_conv;
          double m = ParData::Value(mix, i);
          double d = phase_diff * ParData::Value(detune, i);
          double s = ParData::Value(shape, i);
          buf[i] += m * MIX_MUL *
                    (oscillatorA1.advance(phase_diff + d * MUL_A1, s)
                     + oscillatorA2.advance(phase_diff + d * MUL_A2, s)
                     + oscillatorA3.advance(phase_diff + d * MUL_A3, s)
                     + oscillatorB1.advance(phase_diff + d * MUL_B1, s)
                     + oscillatorB2.advance(phase_diff + d * MUL_B2, s)
                     + oscillatorB3.advance(phase_diff + d * MUL_B3, s)
                    )
                    + (1.0 - m * MIX_COMP_MUL) * oscillator.advance(phase_diff, s);
        }
      }
    }
  };

  template<class FreqType, class MixType, class DetuneType, class ShapeType, class Oscillator>
  struct OscilSuperShapedFunctorStereo {
    inline static void call(const FreqType& freq, const MixType& mix, const DetuneType& detune, const ShapeType& shape,
                            double phase, uint_fast32_t seed, double sample_rate, int64_t length, double* bufL,
                            double* bufR) {
      double freq_conv = M_PI * 2.0 / sample_rate;
      RandomWrapper<std::mt19937> random(seed);
      Oscillator oscillator;
      Oscillator oscillatorA1;
      Oscillator oscillatorA2;
      Oscillator oscillatorA3;
      Oscillator oscillatorB1;
      Oscillator oscillatorB2;
      Oscillator oscillatorB3;
      oscillator.set(phase);
      oscillatorA1.set(random.randDouble(M_PI * 2));
      oscillatorA2.set(random.randDouble(M_PI * 2));
      oscillatorA3.set(random.randDouble(M_PI * 2));
      oscillatorB1.set(random.randDouble(M_PI * 2));
      oscillatorB2.set(random.randDouble(M_PI * 2));
      oscillatorB3.set(random.randDouble(M_PI * 2));
      if (ParData::IsConst(freq)) {
        double phase_diff = ParData::Value(freq) * freq_conv;
        for (int64_t i = 0; i < length; ++i) {
          double m = ParData::Value(mix, i);
          double d = phase_diff * ParData::Value(detune, i);
          double s = ParData::Value(shape, i);
          double value = m * MIX_MUL *
                         (oscillatorA1.advance(phase_diff + d * MUL_A1, s)
                          + oscillatorA2.advance(phase_diff + d * MUL_A2, s)
                          + oscillatorA3.advance(phase_diff + d * MUL_A3, s)
                          + oscillatorB1.advance(phase_diff + d * MUL_B1, s)
                          + oscillatorB2.advance(phase_diff + d * MUL_B2, s)
                          + oscillatorB3.advance(phase_diff + d * MUL_B3, s)
                         )
                         + (1.0 - m * MIX_COMP_MUL) * oscillator.advance(phase_diff, s);
          bufL[i] += value;
          bufR[i] += value;
        }
      } else {
        for (int64_t i = 0; i < length; ++i) {
          double phase_diff = ParData::Value(freq, i) * freq_conv;
          double m = ParData::Value(mix, i);
          double d = phase_diff * ParData::Value(detune, i);
          double s = ParData::Value(shape, i);
          double value = m * MIX_MUL *
                         (oscillatorA1.advance(phase_diff + d * MUL_A1, s)
                          + oscillatorA2.advance(phase_diff + d * MUL_A2, s)
                          + oscillatorA3.advance(phase_diff + d * MUL_A3, s)
                          + oscillatorB1.advance(phase_diff + d * MUL_B1, s)
                          + oscillatorB2.advance(phase_diff + d * MUL_B2, s)
                          + oscillatorB3.advance(phase_diff + d * MUL_B3, s)
                         )
                         + (1.0 - m * MIX_COMP_MUL) * oscillator.advance(phase_diff, s);
          bufL[i] += value;
          bufR[i] += value;
        }
      }
    }
  };

}


#endif //LUAMUSGEN_OSCILLATORS_HPP
