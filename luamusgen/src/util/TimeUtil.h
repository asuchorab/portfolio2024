//
// Created by rutio on 24.02.19.
//

#ifndef LUAMUSGEN_TIMEUTIL_H
#define LUAMUSGEN_TIMEUTIL_H

#include <chrono>
#include <sstream>

namespace timeutil {
  typedef std::chrono::time_point<std::chrono::high_resolution_clock> point;

  inline timeutil::point now() {
    return std::chrono::high_resolution_clock::now();
  }

  inline double duration(timeutil::point p) {
    return std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - p).count();
  }

  inline std::string timeFormat(double seconds) {
    if (seconds < 1e-6) {
      std::stringstream ss;
      ss.precision(5);
      ss << seconds * 1e9 << "ns";
      return ss.str();
    } else if (seconds < 1e-3) {
      std::stringstream ss;
      ss.precision(5);
      ss << seconds * 1e6 << "us";
      return ss.str();
    } else if (seconds < 1) {
      std::stringstream ss;
      ss.precision(5);
      ss << seconds * 1e3 << "ms";
      return ss.str();
    } else if (seconds < 60) {
      std::stringstream ss;
      ss.precision(5);
      ss << seconds << "s";
      return ss.str();
    } else {
      std::stringstream ss;
      int secs = (int) seconds;
      int minutes = secs / 60;
      ss << minutes << "m";
      ss.width(2);
      ss.fill('0');
      ss << secs % 60 << "s";
      return ss.str();
    }
  }
}


#endif //LUAMUSGEN_TIMEUTIL_H
