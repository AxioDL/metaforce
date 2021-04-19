#pragma once

#ifdef __SWITCH__

#include <array>
#include "boo2/boo2.hpp"

namespace metaforce {

struct CLibnxControllerData {
  u64 keysDown;
  JoystickPosition lStick;
  JoystickPosition rStick;
};

} // namespace urde

#endif
