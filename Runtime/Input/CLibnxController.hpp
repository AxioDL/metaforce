#pragma once

#ifdef __SWITCH__

#include <array>
#include "boo2/boo2.hpp"

namespace urde {

struct CLibnxControllerData {
  u64 keysDown;
  JoystickPosition lStick;
  JoystickPosition rStick;
};

} // namespace urde

#endif
