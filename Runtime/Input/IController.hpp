#pragma once

#include "Runtime/Input/InputTypes.hpp"
#include "Runtime/Input/CControllerGamepadData.hpp"

#include "Runtime/GCNTypes.hpp"

namespace metaforce {

class IController {
protected:
  static constexpr float kAbsoluteMinimum = -1.f;
  static constexpr float kAbsoluteMaximum = 1.f;
  static constexpr float kRelativeMinimum = -1.f;
  static constexpr float kRelativeMaximum = 1.f;
public:
  virtual ~IController() = default;
  virtual void Poll() = 0;
  virtual u32 GetDeviceCount() const = 0;
  virtual CControllerGamepadData& GetGamepadData(u32 controller) = 0;
  virtual u32 GetControllerType(u32 controller) const = 0;
  virtual void SetMotorState(EIOPort port, EMotorState state) = 0;

  static IController* Create();
};

} // namespace metaforce
