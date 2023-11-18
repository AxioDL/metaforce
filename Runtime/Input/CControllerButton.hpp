#pragma once

#include "Runtime/GCNTypes.hpp"

namespace metaforce {
enum class EButton {
  A,
  B,
  X,
  Y,
  Start,
  Z,
  Up,
  Right,
  Down,
  Left,
  L,
  R,
  MAX,
};

enum class EAnalogButton {
  Left,
  Right
};

class CControllerButton {
  bool x0_pressed = false;
  bool x1_pressEvent = false;
  bool x2_releaseEvent = false;

public:
  void SetIsPressed(bool pressed) { x0_pressed = pressed; }
  [[nodiscard]] bool GetIsPressed() const { return x0_pressed; }
  void SetPressEvent(bool press){ x1_pressEvent = press; }
  [[nodiscard]] bool GetPressEvent() const{ return x1_pressEvent; }
  void SetReleaseEvent(bool release) { x2_releaseEvent = release;};
  [[nodiscard]] bool GetReleaseEvent() const { return x2_releaseEvent; }
};
} // namespace metaforce