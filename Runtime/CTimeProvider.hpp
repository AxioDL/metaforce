#pragma once
namespace urde {
class CTimeProvider {
public:
  const float& x0_currentTime; // in seconds
  bool x4_first = true;
  CTimeProvider* x8_lastProvider = nullptr;

  CTimeProvider(const float& time);
  ~CTimeProvider();
};
} // namespace urde
