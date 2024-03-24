//
// Created by rutio on 12.08.2019.
//

#ifndef LUAMUSGEN_BUTTERWORTHFILTERS_HPP
#define LUAMUSGEN_BUTTERWORTHFILTERS_HPP

#include <cmath>
#include <vector>

namespace butterworth {

  class FilterBase {
  public:
    FilterBase(double sample_rate, int64_t iterations) : freq_convert(M_PI / sample_rate), iterations(iterations) {
      if (this->iterations < 0) {
        this->iterations = 0;
      }
    }

    inline void updateWetDry(double wetDry) {
      weight1 = 1.0 - wetDry;
      weight2 = wetDry;
    }

  protected:
    double freq_convert;
    int64_t iterations;
    double weight1;
    double weight2;
  };

  class LowPassMono : public FilterBase {
    struct IterationData {
      double A;
      double d1;
      double d2;
      double w0;
      double w1;
      double w2;
    };
  public:
    LowPassMono(double sample_rate, int64_t iterations) : FilterBase(sample_rate, iterations) {
      data.resize(this->iterations);
    }

    inline void updateFrequency(double frequency) {
      double a = tan(frequency * freq_convert);
      double a2 = a * a;
      for (int64_t j = 0; j < iterations; ++j) {
        double r = sin(M_PI * (2.0 * j + 1.0) / (4.0 * iterations));
        double ar2 = 2.0 * a * r;
        double s = a2 + ar2 + 1.0;
        data[j].A = a2 / s;
        data[j].d1 = 2.0 * (1.0 - a2) / s;
        data[j].d2 = (ar2 - a2 - 1.0) / s;
      }
    }

    inline void advance(double& sample, double resonance) {
      double in_mem = sample;
      sample -= res_mem * resonance;
      for (int64_t j = 0; j < iterations; j++) {
        data[j].w0 = data[j].d1 * data[j].w1 + data[j].d2 * data[j].w2 + sample;
        sample = data[j].A * (data[j].w0 + 2.0 * data[j].w1 + data[j].w2);
        data[j].w2 = data[j].w1;
        data[j].w1 = data[j].w0;
      }
      res_mem = sample;
      sample = in_mem * weight1 + sample * weight2;
    }

  private:
    std::vector<IterationData> data;
    double res_mem = 0;
  };

  class LowPassStereo : public FilterBase {
    struct IterationData {
      double A;
      double d1;
      double d2;
      double w0L;
      double w1L;
      double w2L;
      double w0R;
      double w1R;
      double w2R;
    };
  public:
    LowPassStereo(double sample_rate, int64_t iterations) : FilterBase(sample_rate, iterations) {
      data.resize(this->iterations);
    }

    inline void updateFrequency(double frequency) {
      double a = tan(frequency * freq_convert);
      double a2 = a * a;
      for (int64_t j = 0; j < iterations; ++j) {
        double r = sin(M_PI * (2.0 * j + 1.0) / (4.0 * iterations));
        double ar2 = 2.0 * a * r;
        double s = a2 + ar2 + 1.0;
        data[j].A = a2 / s;
        data[j].d1 = 2.0 * (1.0 - a2) / s;
        data[j].d2 = (ar2 - a2 - 1.0) / s;
      }
    }

    inline void advance(double& sampleL, double& sampleR, double resonance) {
      double in_memL = sampleL;
      double in_memR = sampleR;
      sampleL -= res_memL * resonance;
      sampleR -= res_memR * resonance;
      for (int64_t j = 0; j < iterations; j++) {
        data[j].w0L = data[j].d1 * data[j].w1L + data[j].d2 * data[j].w2L + sampleL;
        sampleL = data[j].A * (data[j].w0L + 2.0 * data[j].w1L + data[j].w2L);
        data[j].w2L = data[j].w1L;
        data[j].w1L = data[j].w0L;
        data[j].w0R = data[j].d1 * data[j].w1R + data[j].d2 * data[j].w2R + sampleR;
        sampleR = data[j].A * (data[j].w0R + 2.0 * data[j].w1R + data[j].w2R);
        data[j].w2R = data[j].w1R;
        data[j].w1R = data[j].w0R;
      }
      res_memL = sampleL;
      res_memR = sampleR;
      sampleL = in_memL * weight1 + sampleL * weight2;
      sampleR = in_memR * weight1 + sampleR * weight2;
    }

  private:
    std::vector<IterationData> data;
    double res_memL = 0;
    double res_memR = 0;
  };

  class HighPassMono : public FilterBase {
    struct IterationData {
      double A;
      double d1;
      double d2;
      double w0;
      double w1;
      double w2;
    };
  public:
    HighPassMono(double sample_rate, int64_t iterations) : FilterBase(sample_rate, iterations) {
      data.resize(this->iterations);
    }

    inline void updateFrequency(double frequency) {
      double a = tan(frequency * freq_convert);
      double a2 = a * a;
      for (int64_t j = 0; j < iterations; ++j) {
        double r = sin(M_PI * (2.0 * j + 1.0) / (4.0 * iterations));
        double ar2 = 2.0 * a * r;
        double s = a2 + ar2 + 1.0;
        data[j].A = 1.0 / s;
        data[j].d1 = 2.0 * (1.0 - a2) / s;
        data[j].d2 = (ar2 - a2 - 1.0) / s;
      }
    }

    inline void advance(double& sample, double resonance) {
      double in_mem = sample;
      sample -= res_mem * resonance;
      for (int64_t j = 0; j < iterations; j++) {
        data[j].w0 = data[j].d1 * data[j].w1 + data[j].d2 * data[j].w2 + sample;
        sample = data[j].A * (data[j].w0 - 2.0 * data[j].w1 + data[j].w2);
        data[j].w2 = data[j].w1;
        data[j].w1 = data[j].w0;
      }
      res_mem = sample;
      sample = in_mem * weight1 + sample * weight2;
    }

  private:
    std::vector<IterationData> data;
    double res_mem = 0;
  };

  class HighPassStereo : public FilterBase {
    struct IterationData {
      double A;
      double d1;
      double d2;
      double w0L;
      double w1L;
      double w2L;
      double w0R;
      double w1R;
      double w2R;
    };
  public:
    HighPassStereo(double sample_rate, int64_t iterations) : FilterBase(sample_rate, iterations) {
      data.resize(this->iterations);
    }

    inline void updateFrequency(double frequency) {
      double a = tan(frequency * freq_convert);
      double a2 = a * a;
      for (int64_t j = 0; j < iterations; ++j) {
        double r = sin(M_PI * (2.0 * j + 1.0) / (4.0 * iterations));
        double ar2 = 2.0 * a * r;
        double s = a2 + ar2 + 1.0;
        data[j].A = 1.0 / s;
        data[j].d1 = 2.0 * (1.0 - a2) / s;
        data[j].d2 = (ar2 - a2 - 1.0) / s;
      }
    }

    inline void advance(double& sampleL, double& sampleR, double resonance) {
      double in_memL = sampleL;
      double in_memR = sampleR;
      sampleL -= res_memL * resonance;
      sampleR -= res_memR * resonance;
      for (int64_t j = 0; j < iterations; j++) {
        data[j].w0L = data[j].d1 * data[j].w1L + data[j].d2 * data[j].w2L + sampleL;
        sampleL = data[j].A * (data[j].w0L - 2.0 * data[j].w1L + data[j].w2L);
        data[j].w2L = data[j].w1L;
        data[j].w1L = data[j].w0L;
        data[j].w0R = data[j].d1 * data[j].w1R + data[j].d2 * data[j].w2R + sampleR;
        sampleR = data[j].A * (data[j].w0R - 2.0 * data[j].w1R + data[j].w2R);
        data[j].w2R = data[j].w1R;
        data[j].w1R = data[j].w0R;
      }
      res_memL = sampleL;
      res_memR = sampleR;
      sampleL = in_memL * weight1 + sampleL * weight2;
      sampleR = in_memR * weight1 + sampleR * weight2;
    }

  private:
    std::vector<IterationData> data;
    double res_memL = 0;
    double res_memR = 0;
  };


  class BandPassMono : public FilterBase {
    struct IterationData {
      double A;
      double d1;
      double d2;
      double d3;
      double d4;
      double w0;
      double w1;
      double w2;
      double w3;
      double w4;
    };
  public:
    BandPassMono(double sample_rate, int64_t iterations) : FilterBase(sample_rate, iterations) {
      data.resize(this->iterations);
    }

    inline void updateFrequency(double frequency1, double frequency2) {
      double f_sum = (frequency1 + frequency2) * freq_convert;
      double f_diff = (frequency1 - frequency2) * freq_convert;
      double a = cos(f_sum) / cos(f_diff);
      double a2 = a * a;
      double b = tan(f_diff);
      double b2 = b * b;

      for (int64_t j = 0; j < iterations; ++j) {
        double r = sin(M_PI * (2.0 * j + 1.0) / (4.0 * iterations));
        double br = b * r;
        double br2 = 2.0 * br;
        double s = b2 + br2 + 1.0;
        r = 4.0 * a;
        data[j].A = b2 / s;
        data[j].d1 = r * (1.0 + br) / s;
        data[j].d2 = 2.0 * (b2 - 2.0 * a2 - 1.0) / s;
        data[j].d3 = r * (1.0 - br) / s;
        data[j].d4 = (br2 - b2 - 1.0) / s;
      }
    }

    inline void advance(double& sample, double resonance) {
      double in_mem = sample;
      sample -= res_mem * resonance;
      for (int64_t j = 0; j < iterations; j++) {
        data[j].w0 =
            data[j].d1 * data[j].w1 + data[j].d2 * data[j].w2 + data[j].d3 * data[j].w3 + data[j].d4 * data[j].w4
            + sample;
        sample = data[j].A * (data[j].w0 - 2.0 * data[j].w2 + data[j].w4);
        data[j].w4 = data[j].w3;
        data[j].w3 = data[j].w2;
        data[j].w2 = data[j].w1;
        data[j].w1 = data[j].w0;
      }
      res_mem = sample;
      sample = in_mem * weight1 + sample * weight2;
    }

  private:
    std::vector<IterationData> data;
    double res_mem = 0;
  };

  class BandPassStereo : public FilterBase {
    struct IterationData {
      double A;
      double d1;
      double d2;
      double d3;
      double d4;
      double w0L;
      double w1L;
      double w2L;
      double w3L;
      double w4L;
      double w0R;
      double w1R;
      double w2R;
      double w3R;
      double w4R;
    };
  public:
    BandPassStereo(double sample_rate, int64_t iterations) : FilterBase(sample_rate, iterations) {
      data.resize(this->iterations);
    }

    inline void updateFrequency(double frequency1, double frequency2) {
      double f_sum = (frequency1 + frequency2) * freq_convert;
      double f_diff = (frequency1 - frequency2) * freq_convert;
      double a = cos(f_sum) / cos(f_diff);
      double a2 = a * a;
      double b = tan(f_diff);
      double b2 = b * b;

      for (int64_t j = 0; j < iterations; ++j) {
        double r = sin(M_PI * (2.0 * j + 1.0) / (4.0 * iterations));
        double br = b * r;
        double br2 = 2.0 * br;
        double s = b2 + br2 + 1.0;
        r = 4.0 * a;
        data[j].A = b2 / s;
        data[j].d1 = r * (1.0 + br) / s;
        data[j].d2 = 2.0 * (b2 - 2.0 * a2 - 1.0) / s;
        data[j].d3 = r * (1.0 - br) / s;
        data[j].d4 = (br2 - b2 - 1.0) / s;
      }
    }

    inline void advance(double& sampleL, double& sampleR, double resonance) {
      double in_memL = sampleL;
      double in_memR = sampleR;
      sampleL -= res_memL * resonance;
      sampleR -= res_memR * resonance;
      for (int64_t j = 0; j < iterations; j++) {
        data[j].w0L =
            data[j].d1 * data[j].w1L + data[j].d2 * data[j].w2L + data[j].d3 * data[j].w3L + data[j].d4 * data[j].w4L
            + sampleL;
        sampleL = data[j].A * (data[j].w0L - 2.0 * data[j].w2L + data[j].w4L);
        data[j].w4L = data[j].w3L;
        data[j].w3L = data[j].w2L;
        data[j].w2L = data[j].w1L;
        data[j].w1L = data[j].w0L;
        data[j].w0R =
            data[j].d1 * data[j].w1R + data[j].d2 * data[j].w2R + data[j].d3 * data[j].w3R + data[j].d4 * data[j].w4R
            + sampleR;
        sampleR = data[j].A * (data[j].w0R - 2.0 * data[j].w2R + data[j].w4R);
        data[j].w4R = data[j].w3R;
        data[j].w3R = data[j].w2R;
        data[j].w2R = data[j].w1R;
        data[j].w1R = data[j].w0R;
      }
      res_memL = sampleL;
      res_memR = sampleR;
      sampleL = in_memL * weight1 + sampleL * weight2;
      sampleR = in_memR * weight1 + sampleR * weight2;
    }

  private:
    std::vector<IterationData> data;
    double res_memL = 0;
    double res_memR = 0;
  };

  class BandStopMono : public FilterBase {
    struct IterationData {
      double A;
      double d1;
      double d2;
      double d3;
      double d4;
      double w0;
      double w1;
      double w2;
      double w3;
      double w4;
    };
  public:
    BandStopMono(double sample_rate, int64_t iterations) : FilterBase(sample_rate, iterations) {
      data.resize(this->iterations);
    }

    inline void updateFrequency(double frequency1, double frequency2) {
      double f_sum = (frequency1 + frequency2) * freq_convert;
      double f_diff = (frequency1 - frequency2) * freq_convert;
      double a = cos(f_sum) / cos(f_diff);
      double a2 = a * a;
      double b = tan(f_diff);
      double b2 = b * b;

      for (int64_t j = 0; j < iterations; ++j) {
        r = sin(M_PI * (2.0 * j + 1.0) / (4.0 * iterations));
        double br = b * r;
        double br2 = 2.0 * br;
        s = b2 + br2 + 1.0;
        r = 4.0 * a;
        data[j].A = 1.0 / s;
        data[j].d1 = r * (1.0 + br) / s;
        data[j].d2 = 2.0 * (b2 - 2.0 * a2 - 1.0) / s;
        data[j].d3 = r * (1.0 - br) / s;
        data[j].d4 = (br2 - b2 - 1.0) / s;
        s = 4.0 * a2 + 2.0;
      }
    }

    inline void advance(double& sample, double resonance) {
      double in_mem = sample;
      sample -= res_mem * resonance;
      for (int64_t j = 0; j < iterations; j++) {
        data[j].w0 =
            data[j].d1 * data[j].w1 + data[j].d2 * data[j].w2 + data[j].d3 * data[j].w3 + data[j].d4 * data[j].w4
            + sample;
        sample = data[j].A * (data[j].w0 - r * data[j].w1 + s * data[j].w2 - r * data[j].w3 + data[j].w4);
        data[j].w4 = data[j].w3;
        data[j].w3 = data[j].w2;
        data[j].w2 = data[j].w1;
        data[j].w1 = data[j].w0;
      }
      res_mem = sample;
      sample = in_mem * weight1 + sample * weight2;
    }

  private:
    std::vector<IterationData> data;
    double r, s;
    double res_mem = 0;
  };

  class BandStopStereo : public FilterBase {
    struct IterationData {
      double A;
      double d1;
      double d2;
      double d3;
      double d4;
      double w0L;
      double w1L;
      double w2L;
      double w3L;
      double w4L;
      double w0R;
      double w1R;
      double w2R;
      double w3R;
      double w4R;
    };
  public:
    BandStopStereo(double sample_rate, int64_t iterations) : FilterBase(sample_rate, iterations) {
      data.resize(this->iterations);
    }

    inline void updateFrequency(double frequency1, double frequency2) {
      double f_sum = (frequency1 + frequency2) * freq_convert;
      double f_diff = (frequency1 - frequency2) * freq_convert;
      double a = cos(f_sum) / cos(f_diff);
      double a2 = a * a;
      double b = tan(f_diff);
      double b2 = b * b;

      for (int64_t j = 0; j < iterations; ++j) {
        r = sin(M_PI * (2.0 * j + 1.0) / (4.0 * iterations));
        double br = b * r;
        double br2 = 2.0 * br;
        s = b2 + br2 + 1.0;
        r = 4.0 * a;
        data[j].A = 1.0 / s;
        data[j].d1 = r * (1.0 + br) / s;
        data[j].d2 = 2.0 * (b2 - 2.0 * a2 - 1.0) / s;
        data[j].d3 = r * (1.0 - br) / s;
        data[j].d4 = (br2 - b2 - 1.0) / s;
        s = 4.0 * a2 + 2.0;
      }
    }

    inline void advance(double& sampleL, double& sampleR, double resonance) {
      double in_memL = sampleL;
      double in_memR = sampleR;
      sampleL -= res_memL * resonance;
      sampleR -= res_memR * resonance;
      for (int64_t j = 0; j < iterations; j++) {
        data[j].w0L =
            data[j].d1 * data[j].w1L + data[j].d2 * data[j].w2L + data[j].d3 * data[j].w3L + data[j].d4 * data[j].w4L
            + sampleL;
        sampleL = data[j].A * (data[j].w0L - r * data[j].w1L + s * data[j].w2L - r * data[j].w3L + data[j].w4L);
        data[j].w4L = data[j].w3L;
        data[j].w3L = data[j].w2L;
        data[j].w2L = data[j].w1L;
        data[j].w1L = data[j].w0L;
        data[j].w0R =
            data[j].d1 * data[j].w1R + data[j].d2 * data[j].w2R + data[j].d3 * data[j].w3R + data[j].d4 * data[j].w4R
            + sampleR;
        sampleR = data[j].A * (data[j].w0R - r * data[j].w1R + s * data[j].w2R - r * data[j].w3R + data[j].w4R);
        data[j].w4R = data[j].w3R;
        data[j].w3R = data[j].w2R;
        data[j].w2R = data[j].w1R;
        data[j].w1R = data[j].w0R;
      }
      res_memL = sampleL;
      res_memR = sampleR;
      sampleL = in_memL * weight1 + sampleL * weight2;
      sampleR = in_memR * weight1 + sampleR * weight2;
    }

  private:
    std::vector<IterationData> data;
    double r, s;
    double res_memL = 0;
    double res_memR = 0;
  };

}

#endif //LUAMUSGEN_BUTTERWORTHFILTERS_HPP
