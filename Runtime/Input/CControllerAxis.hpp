#pragma once

namespace metaforce {
enum class EJoyAxis {
  LeftX,
  LeftY,
  RightX,
  RightY,
  MAX
};

class CControllerAxis {
  float x0_absolute = 0.f;
  float x4_relative = 0.f;

public:
  void SetRelativeValue(float val) { x0_absolute = val; }
  float GetRelativeValue() const { return x0_absolute; }
  void SetAbsoluteValue(float val) { x4_relative = val; }
  float GetAbsoluteValue() const { return x4_relative; }
};
} // namespace metaforce