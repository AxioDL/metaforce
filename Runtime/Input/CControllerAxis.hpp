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
  float x0_relative = 0.f;
  float x4_absolute = 0.f;

public:
  void SetRelativeValue(float val) { x0_relative = val; }
  float GetRelativeValue() const { return x0_relative; }
  void SetAbsoluteValue(float val) { x4_absolute = val; }
  float GetAbsoluteValue() const { return x4_absolute; }
};
} // namespace metaforce