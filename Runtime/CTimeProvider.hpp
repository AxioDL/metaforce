#pragma once
namespace urde {
class CTimeProvider;
class CTimeProvider {
public:
  static CTimeProvider* g_currentTimeProvider;
  const float& x0_currentTime; // in seconds
  bool x4_first = true;
  CTimeProvider* x8_lastProvider = nullptr;

  CTimeProvider(const float& time);
  ~CTimeProvider();
};
} // namespace urde
