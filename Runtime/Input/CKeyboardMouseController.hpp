#pragma once

#include <array>
#include <boo/IWindow.hpp>

namespace urde {

struct CKeyboardMouseControllerData {
  std::array<bool, 256> m_charKeys{};
  std::array<bool, 26> m_specialKeys{};
  std::array<bool, 6> m_mouseButtons{};
  boo::EModifierKey m_modMask = boo::EModifierKey::None;
  boo::SWindowCoord m_mouseCoord;
  boo::SScrollDelta m_accumScroll;
};

} // namespace urde
