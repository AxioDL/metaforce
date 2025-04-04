#include "Runtime/Input/CDolphinController.hpp"

#include <dolphin/si.h>
#include <zeus/Math.hpp>

#include <cstring>

namespace metaforce {
CDolphinController::CDolphinController() {
  static bool sIsInitialized = false;
  if (!sIsInitialized) {
    PADSetSpec(5);
    PADInit();
    sIsInitialized = true;
  }
}

void CDolphinController::Poll() {
  ReadDevices();
  ProcessInputData();
}
void CDolphinController::SetMotorState(EIOPort port, EMotorState state) { x194_motorStates[u32(port)] = state; }

float CDolphinController::GetAnalogStickMaxValue(EJoyAxis axis) {
  if (axis >= EJoyAxis::LeftX && axis <= EJoyAxis::LeftY) {
    return 72.f;
  }

  if (axis >= EJoyAxis::RightX && axis <= EJoyAxis::RightY) {
    return 59.f;
  }

  return 0.f;
}

void CDolphinController::ReadDevices() {
  std::array<PADStatus, 4> status{};
  PADRead(status.data());
  if (status[0].err == PAD_ERR_NONE) {
    PADClamp(status.data());
    x4_status = status;
  } else {
    x4_status[0].err = status[0].err;
    x4_status[1].err = status[1].err;
    x4_status[2].err = status[2].err;
    x4_status[3].err = status[3].err;
  }

  for (u32 i = 0; i < 4; ++i) {
    if (x4_status[i].err != PAD_ERR_NOT_READY) {
      if (x4_status[i].err == PAD_ERR_NONE) {
        x34_gamepadStates[i].SetDeviceIsPresent(true);
      } else if (x4_status[i].err == PAD_ERR_NO_CONTROLLER) {
        x1c8_invalidControllers |= PAD_CHAN0_BIT >> i;
        x34_gamepadStates[i].SetDeviceIsPresent(false);
      }
    }

    if (x1b4_controllerTypePollTime[i] == 0) {
      const auto type = SIProbe(i);
      if ((type & (SI_ERROR_NO_RESPONSE | SI_ERROR_UNKNOWN | SI_ERROR_BUSY)) == 0) {
        x1b4_controllerTypePollTime[i] = 0x3c;
        if (type == SI_GC_WIRELESS) {
          x1a4_controllerTypes[i] = skTypeWavebird;
        } else if (type == SI_GBA) { /* here for completeness, the GameCube adapter does not support GBA */
          x1a4_controllerTypes[i] = skTypeGBA;
        } else if (type == SI_GC_STANDARD) {
          x1a4_controllerTypes[i] = skTypeStandard;
        }
      } else {
        x1a4_controllerTypes[i] = skTypeUnknown;
      }
    } else {
      --x1b4_controllerTypePollTime[i];
    }
  }

  if (x1c8_invalidControllers != 0 && PADReset(x1c8_invalidControllers)) {
    x1c8_invalidControllers = 0;
  }
}

void CDolphinController::ProcessInputData() {
  for (u32 i = 0; i < 4; ++i) {
    if (!x34_gamepadStates[i].DeviceIsPresent()) {
      continue;
    }
    ProcessAxis(i, EJoyAxis::LeftX);
    ProcessAxis(i, EJoyAxis::LeftY);
    ProcessAxis(i, EJoyAxis::RightX);
    ProcessAxis(i, EJoyAxis::RightY);
    ProcessButtons(i);
  }
}

void CDolphinController::ProcessAxis(u32 controller, EJoyAxis axis) {
  const auto maxAxisValue = GetAnalogStickMaxValue(axis);
  auto& data = x34_gamepadStates[controller].GetAxis(axis);

  float axisValue = 0.f;
  if (axis == EJoyAxis::LeftX) {
    axisValue = x4_status[controller].stickX;
  } else if (axis == EJoyAxis::LeftY) {
    axisValue = x4_status[controller].stickY;
  } else if (axis == EJoyAxis::RightX) {
    axisValue = x4_status[controller].substickX;
  } else if (axis == EJoyAxis::RightY) {
    axisValue = x4_status[controller].substickY;
  }
  axisValue *= 1.f / maxAxisValue;
  float absolute = zeus::clamp(kAbsoluteMinimum, axisValue, kAbsoluteMaximum);
  float relativeValue = zeus::clamp(kRelativeMinimum, absolute - data.GetAbsoluteValue(), kRelativeMaximum);
  data.SetAbsoluteValue(absolute);
  data.SetRelativeValue(relativeValue);
}

static constexpr std::array<u16, size_t(EButton::MAX)> mButtonMapping{
    PAD_BUTTON_A,  PAD_BUTTON_B,     PAD_BUTTON_X,    PAD_BUTTON_Y,    PAD_BUTTON_START, PAD_TRIGGER_Z,
    PAD_BUTTON_UP, PAD_BUTTON_RIGHT, PAD_BUTTON_DOWN, PAD_BUTTON_LEFT, PAD_TRIGGER_L,    PAD_TRIGGER_R,
};

void CDolphinController::ProcessButtons(u32 controller) {
  for (u32 i = 0; i < u32(EButton::MAX); ++i) {
    ProcessDigitalButton(controller, x34_gamepadStates[controller].GetButton(EButton(i)), mButtonMapping[i]);
  }

  ProcessAnalogButton(x4_status[controller].triggerL,
                      x34_gamepadStates[controller].GetAnalogButton(EAnalogButton::Left));
  ProcessAnalogButton(x4_status[controller].triggerR,
                      x34_gamepadStates[controller].GetAnalogButton(EAnalogButton::Right));
}

void CDolphinController::ProcessDigitalButton(u32 controller, CControllerButton& button, u16 mapping) {
  bool btnPressed = (x4_status[controller].button & mapping) != 0;
  button.SetPressEvent(PADButtonDown(button.GetIsPressed(), btnPressed));
  button.SetReleaseEvent(PADButtonUp(button.GetIsPressed(), btnPressed));
  button.SetIsPressed(btnPressed);
}

void CDolphinController::ProcessAnalogButton(float value, CControllerAxis& axis) {
  float absolute = value * (1 / 150.f);
  if (value * (1 / 150.f) > kAbsoluteMaximum) {
    absolute = kAbsoluteMaximum;
  }

  float relative = absolute - axis.GetAbsoluteValue();
  if (relative > kRelativeMaximum) {
    relative = kRelativeMaximum;
  }

  axis.SetRelativeValue(relative);
  axis.SetAbsoluteValue(absolute);
}

bool CDolphinController::Initialize() {
  // GBAInit();
  memset(x4_status.data(), 0, sizeof(PADStatus) * x4_status.size());
  x34_gamepadStates[0].SetDeviceIsPresent(false);
  x194_motorStates[0] = EMotorState::StopHard;
  x1b4_controllerTypePollTime[0] = 0;
  x1a4_controllerTypes[0] = skTypeUnknown;
  x34_gamepadStates[1].SetDeviceIsPresent(false);
  x194_motorStates[1] = EMotorState::StopHard;
  x1b4_controllerTypePollTime[1] = 0;
  x1a4_controllerTypes[0] = skTypeUnknown;
  x34_gamepadStates[2].SetDeviceIsPresent(false);
  x194_motorStates[2] = EMotorState::StopHard;
  x1b4_controllerTypePollTime[2] = 0;
  x1a4_controllerTypes[0] = skTypeUnknown;
  x34_gamepadStates[3].SetDeviceIsPresent(false);
  x194_motorStates[3] = EMotorState::StopHard;
  x1b4_controllerTypePollTime[3] = 0;
  x1a4_controllerTypes[0] = skTypeUnknown;
  PADControlAllMotors(reinterpret_cast<const u32*>(x194_motorStates.data()));
  Poll();
  return true;
}
} // namespace metaforce
