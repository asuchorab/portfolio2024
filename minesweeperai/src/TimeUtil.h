//
// Created by rutio on 2020-07-12.
//

#ifndef MINESWEEPER_TIMEUTIL_H
#define MINESWEEPER_TIMEUTIL_H

#include <chrono>
#include <sstream>

/**
 * Provides more convenient time measurement over std
 */
namespace timeutil {
  typedef std::chrono::time_point<std::chrono::high_resolution_clock> point;

  /**
   * Gets time point representing current time
   * @return time point to be used with duration functions
   */
  inline timeutil::point now() {
    return std::chrono::high_resolution_clock::now();
  }

  /**
   * Gets duration between time point and now
   * @return seconds from p
   */
  inline double duration(timeutil::point p) {
    return std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - p).count();
  }

  /**
   * Gets duration between two time points
   * @return seconds between p1 and p2
   */
  inline double duration(timeutil::point p1, timeutil::point p2) {
    return std::chrono::duration_cast<std::chrono::duration<double>>(
        p2 - p1).count();
  }

  /**
   * Formats duration in seconds for logging
   * @return string representing duration
   */
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
#endif //MINESWEEPER_TIMEUTIL_H
