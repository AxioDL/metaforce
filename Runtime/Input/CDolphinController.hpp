#pragma once

#include "Runtime/Input/IController.hpp"


namespace metaforce {

class CDolphinController : public IController {
  static constexpr u32 skTypeUnknown = SBIG('UNKN');
  static constexpr u32 skTypeStandard = SBIG('STND');
  static constexpr u32 skTypeGBA = SBIG('GBA_');
  static constexpr u32 skTypeWavebird = SBIG('WAVE');

  std::array<PADStatus, 4> x4_status{};
  std::array<CControllerGamepadData, 4> x34_gamepadStates{};
  std::array<EMotorState, 4> x194_motorStates{};
  std::array<u32, 4> x1a4_controllerTypes{};
  std::array<u32, 4> x1b4_controllerTypePollTime{};
  u32 x1c4_ = 0xf0000000;
  u32 x1c8_invalidControllers = 0;
  u32 x1cc_ = 0;

public:
  CDolphinController();
  void Poll() override;
  [[nodiscard]] u32 GetDeviceCount() const override { return 4; };
  [[nodiscard]] CControllerGamepadData& GetGamepadData(u32 controller) override {
    return x34_gamepadStates[controller];
  };
  [[nodiscard]] u32 GetControllerType(u32 controller) const override { return x1a4_controllerTypes[controller]; }
  void SetMotorState(EIOPort port, EMotorState state) override;
  float GetAnalogStickMaxValue(EJoyAxis axis);
  bool Initialize();

private:
  void ReadDevices();
  void ProcessInputData();
  void ProcessAxis(u32 controller, EJoyAxis axis);
  void ProcessButtons(u32 controller);
  void ProcessDigitalButton(u32 controller, CControllerButton& button, u16 mapping);
  void ProcessAnalogButton(float value, CControllerAxis& axis);
};
} // namespace metaforce