#include "Runtime/Input/CDolphinController.hpp"

namespace PAD {
// clang-format off
enum BUTTON : u16 {
  LEFT         = 0x0001,
  RIGHT        = 0x0002,
  DOWN         = 0x0004,
  UP           = 0x0008,
  TRIGGER_Z    = 0x0010,
  TRIGGER_R    = 0x0020,
  TRIGGER_L    = 0x0040,
  BUTTON_A     = 0x0100,
  BUTTON_B     = 0x0200,
  BUTTON_X     = 0x0400,
  BUTTON_Y     = 0x0800,
  BUTTON_START = 0x1000,
};
// clang-format on
} // namespace PAD

namespace metaforce {
CDolphinController::CDolphinController() {
  static bool sIsInitialized = false;
  if (!sIsInitialized) {
    // PADSetSpec(5);
    // PADInit();
    sIsInitialized = true;
  }
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

void CDolphinController::ProcessAxis(u32 controller, EJoyAxis axis) {
  const auto maxAxisValue = GetAnalogStickMaxValue(axis);
  auto& data = x34_gamepadStates[controller].GetAxis(axis);

  float axisValue = 0.f;
  if (axis == EJoyAxis::LeftX) {
    axisValue = x4_status[controller].x2_stickX;
  } else if (axis == EJoyAxis::LeftY) {
    axisValue = x4_status[controller].x3_stickY;
  } else if (axis == EJoyAxis::RightX) {
    axisValue = x4_status[controller].x4_substickX;
  } else if (axis == EJoyAxis::RightY) {
    axisValue = x4_status[controller].x5_substickY;
  }
  axisValue *= 1.f / maxAxisValue;
  float absolute = kAbsoluteMinimum;
  if (axisValue < kAbsoluteMinimum) {
    absolute = kAbsoluteMinimum;
  } else if (axisValue > kAbsoluteMaximum) {
    absolute = kAbsoluteMaximum;
  }

  axisValue = absolute - data.GetAbsoluteValue();
  float relativeValue = kRelativeMinimum;
  if (axisValue < kRelativeMinimum) {
    relativeValue = kRelativeMinimum;
  } else if (axisValue > kRelativeMaximum) {
    relativeValue = kRelativeMaximum;
  }

  data.SetAbsoluteValue(absolute);
  data.SetRelativeValue(relativeValue);
}

static constexpr std::array<u16, size_t(EButton::MAX)> mButtonMapping{
    PAD::BUTTON_A, PAD::BUTTON_B, PAD::BUTTON_X, PAD::BUTTON_Y, PAD::BUTTON_START, PAD::TRIGGER_Z,
    PAD::UP,       PAD::RIGHT,    PAD::DOWN,     PAD::LEFT,     PAD::TRIGGER_L,    PAD::TRIGGER_R,
};

void CDolphinController::ProcessButtons(u32 controller) {
  for (u32 i = 0; i < u32(EButton::MAX); ++i) {
    ProcessDigitalButton(controller, x34_gamepadStates[controller].GetButton(EButton(i)), mButtonMapping[i]);
  }

  ProcessAnalogButton(x4_status[controller].x6_triggerL,
                      x34_gamepadStates[controller].GetAnalogButton(EAnalogButton::Left));
  ProcessAnalogButton(x4_status[controller].x7_triggerR,
                      x34_gamepadStates[controller].GetAnalogButton(EAnalogButton::Right));
}
void CDolphinController::ProcessDigitalButton(u32 controller, CControllerButton& button, u16 mapping) {}
void CDolphinController::ProcessAnalogButton(float value, CControllerAxis& axis) {}

void CDolphinController::Initialize() {}
} // namespace metaforce