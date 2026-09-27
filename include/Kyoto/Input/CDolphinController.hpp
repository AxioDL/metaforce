#ifndef _CDOLPHINCONTROLLER
#define _CDOLPHINCONTROLLER

#include "Kyoto/Input/IController.hpp"

class CDolphinController : public IController {
  static const uint skTypeUnknown;
  static const uint skTypeStandard;
  static const uint skTypeGBA;
  static const uint skTypeWavebird;

public:
  CDolphinController();
  ~CDolphinController() override;
  void Poll() override;
  uint GetDeviceCount() const override;
  CControllerGamepadData& GetGamepadData(int controller) override;
  uint GetControllerType(int) const override;
  void SetMotorState(EIOPort port, EMotorState state) override;

  bool Initialize();
  float GetAnalogStickMaxValue(EJoyAxis axis) const;

private:
  void ReadDevices();
  void ProcessInputData();
  void ProcessAxis(int controller, EJoyAxis axis);
  void ProcessButtons(int controller);
  void ProcessDigitalButton(int value, CControllerButton& button, ushort mapping);
  void ProcessAnalogButton(float value, CControllerAxis& axis);
  PADStatus mStatus[4];
  CControllerGamepadData mGamepadStates[4];
  EMotorState mMotorStates[4];
  uint mControllerTypes[4];
  uint mControllerTypePollTime[4];
  uint mValidControllers;
  uint mInvalidControllers;
  uint x1cc_;
};

#endif // _CDOLPHINCONTROLLER
