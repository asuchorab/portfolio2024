//
// Created by rutio on 13.08.2019.
//

#ifndef LUAMUSGEN_MOOGFILTERS_HPP
#define LUAMUSGEN_MOOGFILTERS_HPP

#include <cmath>
#include <array>
#include "MathUtil.h"

namespace moog {

  //https://sites.google.com/site/patrickignoto/mumt-618-final-project
  //http://courses.cs.washington.edu/courses/cse490s/11au/Readings/Digital_Sound_Generation_2.pdf

  struct SaturationNone {
    inline static double call(double value) {
      return value;
    }
  };

  struct SaturationLimit {
    inline static double call(double value) {
      return mathut::limit(value);
    }
  };

  struct SaturationTanh {
    inline static double call(double value) {
      return std::tanh(value);
    }
  };

  class FilterBase {
  public:
    inline void updateWetDry(double wetDry) {
      weight1 = 1.0 - wetDry;
      weight2 = wetDry;
    }

    inline void updateFrequency(double frequency) {
      w = freq_convert * frequency;
      g = 0.9892 * w - 0.4342 * w * w + 0.1381 * w * w * w - 0.0202 * w * w * w * w;
    }

    inline void updateResonance(double resonance) {
      rg = resonance * (1.0029 + 0.0526 * w - 0.0926 * w * w + 0.0218 * w * w * w);
    }

  protected:
    explicit FilterBase(double sample_rate) : freq_convert(2 * M_PI / sample_rate) {}

    double freq_convert;
    double weight1;
    double weight2;
    double w, g, rg;
  };

  template<class Saturation>
  class MonoBase : public FilterBase {
  public:
    explicit MonoBase(double sample_rate) : FilterBase(sample_rate), x1({0, 0, 0, 0}), y1({0, 0, 0, 0}) {}

    inline void baseAdvance(double& sample, double inner_mul) {
      sample = Saturation::call(inner_mul * (sample - 4 * rg * (y1[3] - 0.5 * sample)));
      for (int64_t i = 0; i < 4; i++) {
        y1[i] = ((10.0 / 13.0 * sample + 3.0 / 13.0 * x1[i] - y1[i]) * g) + y1[i];
        x1[i] = sample;
        sample = y1[i];
      }
    }

  protected:
    std::array<double, 4> x1, y1;
  };

  template<class Saturation>
  class StereoBase : public FilterBase {
  public:
    explicit StereoBase(double sample_rate) : FilterBase(sample_rate), x1L({0, 0, 0, 0}), y1L({0, 0, 0, 0}),
                                              x1R({0, 0, 0, 0}), y1R({0, 0, 0, 0}) {}

    inline void baseAdvance(double& sampleL, double& sampleR, double inner_mul) {
      sampleL = Saturation::call(inner_mul * (sampleL - 4 * rg * (y1L[3] - 0.5 * sampleL)));
      sampleR = Saturation::call(inner_mul * (sampleR - 4 * rg * (y1R[3] - 0.5 * sampleR)));
      for (int64_t i = 0; i < 4; i++) {
        y1L[i] = (((10.0 / 13.0) * sampleL + (3.0 / 13.0) * x1L[i] - y1L[i]) * g) + y1L[i];
        x1L[i] = sampleL;
        sampleL = y1L[i];
        y1R[i] = (((10.0 / 13.0) * sampleR + (3.0 / 13.0) * x1R[i] - y1R[i]) * g) + y1R[i];
        x1R[i] = sampleR;
        sampleR = y1R[i];
      }
    }

  protected:
    std::array<double, 4> x1L, y1L, x1R, y1R;
  };

  template<class Saturation>
  class LowPassMono : public MonoBase<Saturation> {
  public:
    explicit LowPassMono(double sample_rate) : MonoBase<Saturation>(sample_rate) {}

    void advance(double& sample, double inner_mul) {
      double in = sample;
      MonoBase<Saturation>::baseAdvance(sample, inner_mul);
      sample = FilterBase::weight1 * in + FilterBase::weight2 * sample;
    }
  };

  template<class Saturation>
  class LowPassStereo : public StereoBase<Saturation> {
  public:
    explicit LowPassStereo(double sample_rate) : StereoBase<Saturation>(sample_rate) {}

    void advance(double& sampleL, double& sampleR, double inner_mul) {
      double inL = sampleL;
      double inR = sampleR;
      StereoBase<Saturation>::baseAdvance(sampleL, sampleR, inner_mul);
      sampleL = FilterBase::weight1 * inL + FilterBase::weight2 * sampleL;
      sampleR = FilterBase::weight1 * inR + FilterBase::weight2 * sampleR;
    }
  };

  template<class Saturation>
  class HighPassMono : public MonoBase<Saturation> {
  public:
    explicit HighPassMono(double sample_rate) : MonoBase<Saturation>(sample_rate) {}

    void advance(double& sample, double inner_mul) {
      double in = sample;
      MonoBase<Saturation>::baseAdvance(sample, inner_mul);
      sample = FilterBase::weight1 * in + FilterBase::weight2 * (MonoBase<Saturation>::x1[0] - sample);
    }
  };

  template<class Saturation>
  class HighPassStereo : public StereoBase<Saturation> {
  public:
    explicit HighPassStereo(double sample_rate) : StereoBase<Saturation>(sample_rate) {}

    void advance(double& sampleL, double& sampleR, double inner_mul) {
      double inL = sampleL;
      double inR = sampleR;
      StereoBase<Saturation>::baseAdvance(sampleL, sampleR, inner_mul);
      sampleL = FilterBase::weight1 * inL + FilterBase::weight2 * (StereoBase<Saturation>::x1L[0] - sampleL);
      sampleR = FilterBase::weight1 * inR + FilterBase::weight2 * (StereoBase<Saturation>::x1R[0] - sampleR);
    }
  };

}

#endif //LUAMUSGEN_MOOGFILTERS_HPP
