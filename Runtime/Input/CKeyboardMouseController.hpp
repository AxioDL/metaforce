#pragma once

#include <array>
#include "boo2/boo2.hpp"

namespace metaforce {

struct CKeyboardMouseControllerData {
  std::array<bool, 256> m_charKeys{};
  std::array<bool, 256> m_specialKeys{};
  std::array<bool, 6> m_mouseButtons{};
  boo2::KeyModifier m_modMask = boo2::KeyModifier::None;
  hsh::offset2dF m_mouseCoord;
  hsh::offset2dF m_mouseCoordNorm;
  hsh::offset2dF m_accumScroll;
};

} // namespace metaforce
