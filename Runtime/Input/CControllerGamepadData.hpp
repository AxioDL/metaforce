#pragma once

#include "Runtime/Input/CControllerAxis.hpp"
#include "Runtime/Input/CControllerButton.hpp"

#include <array>

namespace metaforce {
class CControllerGamepadData {
  bool x0_present;
  std::array<CControllerAxis, 4> x4_axes;
  std::array<CControllerAxis, 2> x24_triggers;
  std::array<CControllerButton, 12> x34_buttons;

public:
  void SetDeviceIsPresent(bool present) { x0_present = present; }
  [[nodiscard]] bool DeviceIsPresent() const { return x0_present; }

  [[nodiscard]] const CControllerAxis& GetAxis(EJoyAxis axis) const { return x4_axes[u32(axis)]; }
  [[nodiscard]] CControllerAxis& GetAxis(EJoyAxis axis) { return x4_axes[u32(axis)]; }

  [[nodiscard]] const CControllerButton& GetButton(EButton button) const { return x34_buttons[u32(button)]; }
  [[nodiscard]] CControllerButton& GetButton(EButton button) { return x34_buttons[u32(button)]; }

  [[nodiscard]] const CControllerAxis& GetAnalogButton(EAnalogButton button) const { return x24_triggers[u32(button)]; }
  [[nodiscard]] CControllerAxis& GetAnalogButton(EAnalogButton button) { return x24_triggers[u32(button)]; }
};
} // namespace metaforce