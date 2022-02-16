#pragma once

#include <array>

#include <aurora/aurora.hpp>
#include <boo/IWindow.hpp>

namespace metaforce {

struct CKeyboardMouseControllerData {
  std::array<bool, 256> m_charKeys{};
  std::array<bool, static_cast<size_t>(aurora::SpecialKey::MAX)> m_specialKeys{};
  std::array<bool, 6> m_mouseButtons{};
  aurora::ModifierKey m_modMask = aurora::ModifierKey::None;
  boo::SWindowCoord m_mouseCoord;
  boo::SScrollDelta m_accumScroll;
};

} // namespace metaforce
