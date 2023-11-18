#pragma once

#include "Runtime/GCNTypes.hpp"

#include <cmath>
#include <chrono>

namespace metaforce {
class CStopwatch {
private:
  static CStopwatch mGlobalTimer;

  using Time = std::chrono::steady_clock;
  using MicroSeconds = std::chrono::microseconds;
  using MilliSeconds = std::chrono::milliseconds;
  using FloatSeconds = std::chrono::duration<float>;
  Time::time_point m_startTime;

public:
  static void InitGlobalTimer();
  static CStopwatch& GetGlobalTimerObj() { return mGlobalTimer; }
  static float GetGlobalTime() { return mGlobalTimer.GetElapsedTime(); }

  void Reset();
  void Wait(float wait);

  float GetElapsedTime() const;
  u16 GetElapsedMicros() const;
  u64 GetCurMicros() const;
};
} // namespace metaforce
