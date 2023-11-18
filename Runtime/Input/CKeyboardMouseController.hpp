#pragma once

#include <array>

#include <aurora/aurora.h>
#include "GCNTypes.hpp"

namespace metaforce {
// TODO: copied from boo; should be rewritten
enum class EControllerButton : uint8_t {
  A,
  B,
  X,
  Y,
  Back,
  Guide,
  Start,
  LeftStick,
  RightStick,
  LeftShoulder,
  RightShoulder,
  DPadUp,
  DPadDown,
  DPadLeft,
  DPadRight,
  Other,
  MAX,
};
enum class EControllerAxis : uint8_t {
  LeftX,
  LeftY,
  RightX,
  RightY,
  TriggerLeft,
  TriggerRight,
  MAX,
};
enum class EMouseButton { None = 0, Primary = 1, Secondary = 2, Middle = 3, Aux1 = 4, Aux2 = 5 };
enum class ESpecialKey : uint8_t {
  None = 0,
  F1 = 1,
  F2 = 2,
  F3 = 3,
  F4 = 4,
  F5 = 5,
  F6 = 6,
  F7 = 7,
  F8 = 8,
  F9 = 9,
  F10 = 10,
  F11 = 11,
  F12 = 12,
  F13 = 13,
  F14 = 14,
  F15 = 15,
  F16 = 16,
  F17 = 17,
  F18 = 18,
  F19 = 19,
  F20 = 20,
  F21 = 21,
  F22 = 22,
  F23 = 23,
  F24 = 24,
  Esc = 25,
  Enter = 26,
  Backspace = 27,
  Insert = 28,
  Delete = 29,
  Home = 30,
  End = 31,
  PgUp = 32,
  PgDown = 33,
  Left = 34,
  Right = 35,
  Up = 36,
  Down = 37,
  Tab = 38,
  PrintScreen = 39,
  ScrollLock = 40,
  Pause = 41,
  NumLockClear = 42,
  KpDivide = 43,
  KpMultiply = 44,
  KpMinus = 45,
  KpPlus = 46,
  KpEnter = 47,
  KpNum0 = 48,
  KpNum1 = 49,
  KpNum2 = 50,
  KpNum3 = 51,
  KpNum4 = 52,
  KpNum5 = 53,
  KpNum6 = 54,
  KpNum7 = 55,
  KpNum8 = 56,
  KpNum9 = 57,
  KpPercent = 58,
  KpPeriod = 59,
  KpComma = 60,
  KpEquals = 61,
  Application = 62,
  Power = 63,
  Execute = 64,
  Help = 65,
  Menu = 66,
  Select = 67,
  Stop = 68,
  Again = 69,
  Undo = 70,
  Cut = 71,
  Paste = 72,
  Find = 73,
  VolumeUp = 74,
  VolumeDown = 75,
  MAX,
};
enum class EModifierKey {
  None = 0,
  Ctrl = 1 << 0,
  Alt = 1 << 2,
  Shift = 1 << 3,
  Command = 1 << 4,
  CtrlCommand = EModifierKey::Ctrl | EModifierKey::Command
};
ENABLE_BITWISE_ENUM(EModifierKey)
struct SWindowCoord {
  std::array<int, 2> pixel;
  std::array<int, 2> virtualPixel;
  std::array<float, 2> norm;
};
struct SScrollDelta {
  std::array<double, 2> delta{};
  bool isFine = false;        /* Use system-scale fine-scroll (for scrollable-trackpads) */
  bool isAccelerated = false; /* System performs acceleration computation */

  constexpr SScrollDelta operator+(const SScrollDelta& other) const noexcept {
    SScrollDelta ret;
    ret.delta[0] = delta[0] + other.delta[0];
    ret.delta[1] = delta[1] + other.delta[1];
    ret.isFine = isFine || other.isFine;
    ret.isAccelerated = isAccelerated || other.isAccelerated;
    return ret;
  }
  constexpr SScrollDelta operator-(const SScrollDelta& other) const noexcept {
    SScrollDelta ret;
    ret.delta[0] = delta[0] - other.delta[0];
    ret.delta[1] = delta[1] - other.delta[1];
    ret.isFine = isFine || other.isFine;
    ret.isAccelerated = isAccelerated || other.isAccelerated;
    return ret;
  }
  constexpr SScrollDelta& operator+=(const SScrollDelta& other) noexcept {
    delta[0] += other.delta[0];
    delta[1] += other.delta[1];
    isFine |= other.isFine;
    isAccelerated |= other.isAccelerated;
    return *this;
  }
  constexpr void zeroOut() noexcept { delta = {}; }
  constexpr bool isZero() const noexcept { return delta[0] == 0.0 && delta[1] == 0.0; }
};

struct CKeyboardMouseControllerData {
  std::array<bool, 256> m_charKeys{};
  std::array<bool, static_cast<size_t>(ESpecialKey::MAX)> m_specialKeys{};
  std::array<bool, 6> m_mouseButtons{};
  EModifierKey m_modMask = EModifierKey::None;
  SWindowCoord m_mouseCoord;
  SScrollDelta m_accumScroll;
};

} // namespace metaforce
