#ifndef _CCONTROLLERGAMEPADDATA
#define _CCONTROLLERGAMEPADDATA

#include "Kyoto/Input/CControllerAxis.hpp"
#include "Kyoto/Input/CControllerButton.hpp"
#include "Kyoto/Input/InputTypes.hpp"

class CControllerGamepadData {
public:
  void SetDeviceIsPresent(bool present) { mPresent = present; }
  const bool DeviceIsPresent() const { return mPresent; }

  const CControllerAxis& GetAxis(EJoyAxis axis) const { return mAxes[axis]; }
  CControllerAxis& GetAxis(EJoyAxis axis) { return mAxes[axis]; }

  const CControllerButton& GetButton(EButton button) const { return mButtons[button]; }
  CControllerButton& GetButton(EButton button) { return mButtons[button]; }

  const CControllerAxis& GetAnalogButton(EAnalogButton button) const {
    return mTriggers[button];
  }
  CControllerAxis& GetAnalogButton(EAnalogButton button) { return mTriggers[button]; }

private:
  bool mPresent;
  CControllerAxis mAxes[4];
  CControllerAxis mTriggers[2];
  CControllerButton mButtons[12];
};

#endif // _CCONTROLLERGAMEPADDATA
