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
  bool x0_;
  bool x1_pressed;
  bool x2_;

public:
  void SetIsPressed(bool pressed) { x1_pressed = pressed; }
  [[nodiscard]] bool GetIsPressed() const { return x1_pressed; }
  void SetPressEvent(bool press);
  [[nodiscard]] bool GetPressEvent() const;
  void SetReleaseEvent(bool release);
};
} // namespace metaforce