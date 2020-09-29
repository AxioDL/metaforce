#pragma once

#include <chrono>
#include <fmt/format.h>

namespace urde {
class CStopwatch {
  std::chrono::steady_clock::time_point m_start;

public:
  CStopwatch() : m_start(std::chrono::steady_clock::now()) {}
  double report(const char* name) const {
    double t = std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::steady_clock::now() - m_start).count() / 1000000.0;
//#ifndef NDEBUG
//    fmt::print(FMT_STRING("{} {}\n"), name, t);
//#endif
    return t;
  }
  double reportReset(const char* name) {
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    double t = std::chrono::duration_cast<std::chrono::microseconds>(now - m_start).count() / 1000000.0;
//#ifndef NDEBUG
//    fmt::print(FMT_STRING("{} {}\n"), name, t);
//#endif
    m_start = now;
    return t;
  }
};
} // namespace urde
