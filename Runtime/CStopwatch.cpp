#include "Runtime/CStopwatch.hpp"

namespace metaforce {
CStopwatch CStopwatch::mGlobalTimer = {};

float CStopwatch::GetElapsedTime() const {
  return static_cast<float>(std::chrono::duration_cast<MilliSeconds>(Time::now() - m_startTime).count()) / 1000.f;
}

u16 CStopwatch::GetElapsedMicros() const {
  return std::chrono::duration_cast<MicroSeconds>(Time::now() - m_startTime).count();
}

u64 CStopwatch::GetCurMicros() const {
  return std::chrono::duration_cast<MicroSeconds>(Time::now().time_since_epoch()).count();
}

void CStopwatch::InitGlobalTimer() { mGlobalTimer.Reset(); }

void CStopwatch::Reset() { m_startTime = std::chrono::steady_clock::now(); }

void CStopwatch::Wait(float wait) {
  if (std::fabs(wait) < 0.001f) {
    wait = 0.f;
  }

  auto waitDur = FloatSeconds{wait};
  while ((Time::now() - m_startTime) < waitDur) {}
}
} // namespace metaforce