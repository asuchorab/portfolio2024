//
// Created by rutio on 01.05.19.
//

#ifndef LUAMUSGEN_MATHUTIL_H
#define LUAMUSGEN_MATHUTIL_H

#include <cmath>

namespace mathut {

  const double COMPARISON_TOLERANCE = 1e-10;
  inline bool equals(double v1, double v2) { return std::abs(v2 - v1) < COMPARISON_TOLERANCE; }
  inline double limit(double v) { if (v > 1.0) return 1.0; else if (v < -1.0) return -1.0; else return v; }
  inline double limit(double v, double l) { if (v > l) return l; else if (v < -l) return -l; else return v; }
  inline double amp_to_dB(double v) { return 20.0*log10(std::fabs(v)); }
  inline double dB_to_amp(double v) { return pow(10.0, v/20.0); }

  inline double pseudoExponential(double v1, double v2) {
    if (v1 > 0) {
      return pow(v1, v2);
    } else if (v1 < 0) {
      return -pow(-v1, v2);
    } else {
      return 0;
    }
  }


}

#endif //LUAMUSGEN_MATHUTIL_H
