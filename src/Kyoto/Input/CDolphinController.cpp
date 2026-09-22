#include "Kyoto/Input/CDolphinController.hpp"

#include <Kyoto/Alloc/CMemory.hpp>

#include <dolphin/gba.h>
#include <dolphin/si.h>

#include <string.h>

CDolphinController::CDolphinController()
: x1c4_validControllers(PAD_CHAN0_BIT | PAD_CHAN1_BIT | PAD_CHAN2_BIT | PAD_CHAN3_BIT)
, x1c8_invalidControllers(0)
, x1cc_(0) {
  static bool sIsInitialized = false;
  if (!sIsInitialized) {
    PADSetSpec(PAD_SPEC_5);
    PADInit();
    sIsInitialized = true;
  }
}

CDolphinController::~CDolphinController() {}

bool CDolphinController::Initialize() {
  GBAInit();
  memset(x4_status, 0, sizeof(PADStatus) * 4);
  for (uint i = 0; i < 4; ++i) {
    x34_gamepadStates[i].SetDeviceIsPresent(false);
    x194_motorStates[i] = kMS_StopHard;
    x1b4_controllerTypePollTime[i] = 0;
    x1a4_controllerTypes[i] = skTypeUnknown;
  }

  PADControlAllMotors((const u32*)x194_motorStates);
  Poll();
  return true;
}

void CDolphinController::Poll() {
  ReadDevices();
  ProcessInputData();
}

void CDolphinController::ReadDevices() {
  PADStatus status[4];
  PADRead(status);
  if (status[0].err == PAD_ERR_NONE) {
    PADClamp(status);
    memcpy(x4_status, status, sizeof(PADStatus) * 4);
  } else {
    for (int i = 0; i < 4; ++i) {
      x4_status[i].err = status[i].err;
    }
  }

  for (int i = 0; i < 4; ++i) {
    uint controller = (PAD_CHAN0_BIT >> i);
    if (x4_status[i].err != PAD_ERR_NOT_READY) {
      if (x4_status[i].err == PAD_ERR_NONE) {
        x34_gamepadStates[i].SetDeviceIsPresent(true);
      } else if (x4_status[i].err == PAD_ERR_NO_CONTROLLER) {
        x1c8_invalidControllers |= controller;
        x34_gamepadStates[i].SetDeviceIsPresent(false);
      }
    }

    if (x1b4_controllerTypePollTime[i] != 0) {
      --x1b4_controllerTypePollTime[i];
    } else {
      const uint type = SIProbe(i);
      if ((type & (SI_ERROR_NO_RESPONSE | SI_ERROR_UNKNOWN | SI_ERROR_BUSY)) != 0) {
        if (x1b4_controllerTypePollTime[i] == 0) {
          x1a4_controllerTypes[i] = skTypeUnknown;
        }
      } else {
        x1b4_controllerTypePollTime[i] = 60;
        if (type == SI_GC_WAVEBIRD) {
          x1a4_controllerTypes[i] = skTypeWavebird;
        } else if (type == SI_GBA) {
          x1a4_controllerTypes[i] = skTypeGBA;
        } else if (type == SI_GC_CONTROLLER) {
          x1a4_controllerTypes[i] = skTypeStandard;
        }
      }
    }
  }

  if (x1c8_invalidControllers != 0 && PADReset(x1c8_invalidControllers)) {
    x1c8_invalidControllers = 0;
  }
}

void CDolphinController::ProcessInputData() {
  for (int i = 0; i < 4; ++i) {
    if (x34_gamepadStates[i].DeviceIsPresent()) {
      ProcessAxis(i, kJA_LeftX);
      ProcessAxis(i, kJA_LeftY);
      ProcessAxis(i, kJA_RightX);
      ProcessAxis(i, kJA_RightY);
      ProcessButtons(i);
    }
  }
}

void CDolphinController::ProcessAxis(int controller, EJoyAxis axis) {
  const float maxAxisValue = 1.f / GetAnalogStickMaxValue(axis);
  CControllerAxis& data = x34_gamepadStates[controller].GetAxis(axis);

  float axisValue = 0.f;
  switch (axis) {
  case kJA_LeftX:
    axisValue = x4_status[controller].stickX;
    break;
  case kJA_LeftY:
    axisValue = x4_status[controller].stickY;
    break;
  case kJA_RightX:
    axisValue = x4_status[controller].substickX;
    break;
  case kJA_RightY:
    axisValue = x4_status[controller].substickY;
    break;
  default:
    break;
  }

  float absolute = axisValue * maxAxisValue;
  if (absolute < kAbsoluteMinimum) {
    absolute = kAbsoluteMinimum;
  } else if (absolute > kAbsoluteMaximum) {
    absolute = kAbsoluteMaximum;
  }

  float relativeValue = absolute - data.GetAbsoluteValue();
  if (relativeValue < kRelativeMinimum) {
    relativeValue = kRelativeMinimum;
  } else if (relativeValue > kRelativeMaximum) {
    relativeValue = kRelativeMaximum;
  }
  data.SetRelativeValue(relativeValue);
  data.SetAbsoluteValue(absolute);
}

static ushort mButtonMapping[size_t(kBU_MAX)] = {
    PAD_BUTTON_A,     PAD_BUTTON_B,    PAD_BUTTON_X,  PAD_BUTTON_Y,
    PAD_BUTTON_START, PAD_TRIGGER_Z,   PAD_BUTTON_UP, PAD_BUTTON_RIGHT,
    PAD_BUTTON_DOWN,  PAD_BUTTON_LEFT, PAD_TRIGGER_L, PAD_TRIGGER_R,
};

void CDolphinController::ProcessButtons(int controller) {
  for (int i = 0; i < int(kBU_MAX); ++i) {
    ProcessDigitalButton(controller, x34_gamepadStates[controller].GetButton(EButton(i)),
                         mButtonMapping[i]);
  }

  ProcessAnalogButton(x4_status[controller].triggerLeft,
                      x34_gamepadStates[controller].GetAnalogButton(kBA_Left));
  ProcessAnalogButton(x4_status[controller].triggerRight,
                      x34_gamepadStates[controller].GetAnalogButton(kBA_Right));
}

void CDolphinController::ProcessDigitalButton(int controller, CControllerButton& button,
                                              ushort mapping) {
  bool btnPressed = (x4_status[controller].button & mapping);
  button.SetPressEvent(PADButtonDown(button.GetIsPressed(), btnPressed));
  button.SetReleaseEvent(PADButtonUp(button.GetIsPressed(), btnPressed));
  button.SetIsPressed(btnPressed);
}

void CDolphinController::ProcessAnalogButton(float value, CControllerAxis& axis) {
  value *= 1.f / 150.f;
  if (value > kAbsoluteMaximum) {
    value = kAbsoluteMaximum;
  }

  float relative = value - axis.GetAbsoluteValue();
  if (relative > kRelativeMaximum) {
    relative = kRelativeMaximum;
  }

  axis.SetRelativeValue(relative);
  axis.SetAbsoluteValue(value);
}

uint CDolphinController::GetDeviceCount() const { return 4; }

CControllerGamepadData& CDolphinController::GetGamepadData(int controller) {
  return x34_gamepadStates[controller];
}

uint CDolphinController::GetControllerType(int controller) const {
  return x1a4_controllerTypes[controller];
}

void CDolphinController::SetMotorState(EIOPort port, EMotorState state) {
  x194_motorStates[port] = state;
  PADControlAllMotors((const u32*)x194_motorStates);
}

float CDolphinController::GetAnalogStickMaxValue(EJoyAxis axis) const {
  switch (axis) {
  case kJA_LeftX:
  case kJA_LeftY:
    return 72.0f;

  case kJA_RightX:
  case kJA_RightY:
    return 59.0f;

  default:
    return 0.0f;
  }
}

const uint CDolphinController::skTypeUnknown = 'UNKN';
const uint CDolphinController::skTypeStandard = 'STND';
const uint CDolphinController::skTypeGBA = 'GBA_';
const uint CDolphinController::skTypeWavebird = 'WAVE';
