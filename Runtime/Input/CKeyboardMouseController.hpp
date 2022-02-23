#pragma once

#include <array>

#include <aurora/aurora.hpp>

namespace metaforce {

// TODO: copied from boo; should be rewritten and included from from aurora
enum class EMouseButton { None = 0, Primary = 1, Secondary = 2, Middle = 3, Aux1 = 4, Aux2 = 5 };
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
  std::array<bool, static_cast<size_t>(aurora::SpecialKey::MAX)> m_specialKeys{};
  std::array<bool, 6> m_mouseButtons{};
  aurora::ModifierKey m_modMask = aurora::ModifierKey::None;
  SWindowCoord m_mouseCoord;
  SScrollDelta m_accumScroll;
};

} // namespace metaforce
