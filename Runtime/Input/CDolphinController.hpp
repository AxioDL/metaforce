#pragma once

#include "Runtime/Input/IController.hpp"

namespace metaforce {
struct PADStatus {
  u16 x0_buttons;
  s8 x2_stickX;
  s8 x3_stickY;
  s8 x4_substickX;
  s8 x5_substickY;
  u8 x6_triggerL;
  u8 x7_triggerR;
  u8 x8_analogA;
  u8 x9_analogB;
  s8 xa_err;
};

class CDolphinController : public IController {

  std::array<PADStatus, 4> x4_status;
  std::array<CControllerGamepadData, 4> x34_gamepadStates{};
  std::array<EMotorState, 4> x194_motorStates;
  std::array<u32, 4> x1a4_controllerTypes{};
  std::array<u32, 4> x1b4_{};
  u32 x1c4_ = 0xf0000000;
  u32 x1c8_ = 0;
  u32 x1cc_ = 0;

public:
  CDolphinController();
  void Poll() override{};
  [[nodiscard]] u32 GetDeviceCount() const override { return 4; };
  [[nodiscard]] CControllerGamepadData& GetGamepadData(u32 controller) override {
    return x34_gamepadStates[controller];
  };
  [[nodiscard]] u32 GetControllerType(u32 controller) const override { return x1a4_controllerTypes[controller]; }
  void SetMotorState(EIOPort port, EMotorState state) override;

  float GetAnalogStickMaxValue(EJoyAxis axis);
  void ProcessAxis(u32 controller, EJoyAxis axis);
  void ProcessButtons(u32 controller);
  void ProcessDigitalButton(u32 controller, CControllerButton& button, u16 mapping);
  void ProcessAnalogButton(float value, CControllerAxis& axis);

  void Initialize();
};
} // namespace metaforce