#include "Runtime/Input/CFinalInput.hpp"

#include <zeus/Math.hpp>

namespace metaforce {

CFinalInput::CFinalInput() = default;

CFinalInput::CFinalInput(int cIdx, float dt, const CControllerGamepadData& data, float leftDiv, float rightDiv)
: x0_dt(dt)
, x4_controllerIdx(cIdx)
, x8_anaLeftX(data.GetAxis(EJoyAxis::LeftX).GetAbsoluteValue())
, xc_anaLeftY(data.GetAxis(EJoyAxis::LeftY).GetAbsoluteValue())
, x10_anaRightX(data.GetAxis(EJoyAxis::RightX).GetAbsoluteValue())
, x14_anaRightY(data.GetAxis(EJoyAxis::RightY).GetAbsoluteValue())
, x18_anaLeftTrigger(data.GetAnalogButton(EAnalogButton::Left).GetAbsoluteValue())
, x1c_anaRightTrigger(data.GetAnalogButton(EAnalogButton::Right).GetAbsoluteValue())
, x24_anaLeftTriggerP(data.GetAnalogButton(EAnalogButton::Left).GetRelativeValue())
, x28_anaRightTriggerP(data.GetAnalogButton(EAnalogButton::Right).GetRelativeValue())
, x2c_b24_A(data.GetButton(EButton::A).GetIsPressed())
, x2c_b25_B(data.GetButton(EButton::B).GetIsPressed())
, x2c_b26_X(data.GetButton(EButton::X).GetIsPressed())
, x2c_b27_Y(data.GetButton(EButton::Y).GetIsPressed())
, x2c_b28_Z(data.GetButton(EButton::Z).GetIsPressed())
, x2c_b29_L(data.GetButton(EButton::L).GetIsPressed())
, x2c_b30_R(data.GetButton(EButton::R).GetIsPressed())
, x2c_b31_DPUp(data.GetButton(EButton::Up).GetIsPressed())
, x2d_b24_DPRight(data.GetButton(EButton::Right).GetIsPressed())
, x2d_b25_DPDown(data.GetButton(EButton::Down).GetIsPressed())
, x2d_b26_DPLeft(data.GetButton(EButton::Left).GetIsPressed())
, x2d_b27_Start(data.GetButton(EButton::Start).GetIsPressed())
, x2d_b28_PA(data.GetButton(EButton::A).GetPressEvent())
, x2d_b29_PB(data.GetButton(EButton::B).GetPressEvent())
, x2d_b30_PX(data.GetButton(EButton::X).GetPressEvent())
, x2d_b31_PY(data.GetButton(EButton::Y).GetPressEvent())
, x2e_b24_PZ(data.GetButton(EButton::Z).GetPressEvent())
, x2e_b25_PL(data.GetButton(EButton::L).GetPressEvent())
, x2e_b26_PR(data.GetButton(EButton::R).GetPressEvent())
, x2e_b27_PDPUp(data.GetButton(EButton::Up).GetPressEvent())
, x2e_b28_PDPRight(data.GetButton(EButton::Right).GetPressEvent())
, x2e_b29_PDPDown(data.GetButton(EButton::Down).GetPressEvent())
, x2e_b30_PDPLeft(data.GetButton(EButton::Left).GetPressEvent())
, x2e_b31_PStart(data.GetButton(EButton::Start).GetPressEvent()) {
  InitializeAnalog(leftDiv, rightDiv);
}

#if 0
CFinalInput::CFinalInput(int cIdx, float dt, const CKeyboardMouseControllerData& data, const CFinalInput& prevInput)
: x0_dt(dt)
, x4_controllerIdx(cIdx)
, x18_anaLeftTrigger(false)
, x1c_anaRightTrigger(false)
, x20_enableAnaLeftXP(DLARight() && !prevInput.DLARight())
, x21_enableAnaLeftYP(DLAUp() && !prevInput.DLAUp())
, x22_enableAnaRightXP(DRARight() && !prevInput.DRARight())
, x23_enableAnaRightYP(DRAUp() && !prevInput.DRAUp())
, x24_anaLeftTriggerP(DLTrigger() && !prevInput.DLTrigger())
, x28_anaRightTriggerP(DRTrigger() && !prevInput.DRTrigger())
, x2c_b31_DPUp(data.m_specialKeys[size_t(aurora::SpecialKey::Up)])
, x2d_b24_DPRight(data.m_specialKeys[size_t(aurora::SpecialKey::Right)])
, x2d_b25_DPDown(data.m_specialKeys[size_t(aurora::SpecialKey::Down)])
, x2d_b26_DPLeft(data.m_specialKeys[size_t(aurora::SpecialKey::Left)])
, x2d_b28_PA(DA() && !prevInput.DA())
, x2d_b29_PB(DB() && !prevInput.DB())
, x2d_b30_PX(DX() && !prevInput.DX())
, x2d_b31_PY(DY() && !prevInput.DY())
, x2e_b24_PZ(DZ() && !prevInput.DZ())
, x2e_b25_PL(DL() && !prevInput.DL())
, x2e_b26_PR(DR() && !prevInput.DR())
, x2e_b27_PDPUp(DDPUp() && !prevInput.DDPUp())
, x2e_b28_PDPRight(DDPRight() && !prevInput.DDPRight())
, x2e_b29_PDPDown(DDPDown() && !prevInput.DDPDown())
, x2e_b30_PDPLeft(DDPLeft() && !prevInput.DDPLeft())
, x2e_b31_PStart(DStart() && !prevInput.DStart())
, m_kbm(data) {
  if (prevInput.m_kbm) {
    for (size_t i = 0; i < m_PCharKeys.size(); ++i) {
      m_PCharKeys[i] = data.m_charKeys[i] && !prevInput.m_kbm->m_charKeys[i];
    }
    for (size_t i = 0; i < m_PSpecialKeys.size(); ++i) {
      m_PSpecialKeys[i] = data.m_specialKeys[i] && !prevInput.m_kbm->m_specialKeys[i];
    }
    for (size_t i = 0; i < m_PMouseButtons.size(); ++i) {
      m_PMouseButtons[i] = data.m_mouseButtons[i] && !prevInput.m_kbm->m_mouseButtons[i];
    }
  }
}

CFinalInput& CFinalInput::operator|=(const CFinalInput& other) {
  if (std::fabs(other.x8_anaLeftX) > std::fabs(x8_anaLeftX))
    x8_anaLeftX = other.x8_anaLeftX;
  if (std::fabs(other.xc_anaLeftY) > std::fabs(xc_anaLeftY))
    xc_anaLeftY = other.xc_anaLeftY;
  if (std::fabs(other.x10_anaRightX) > std::fabs(x10_anaRightX))
    x10_anaRightX = other.x10_anaRightX;
  if (std::fabs(other.x14_anaRightY) > std::fabs(x14_anaRightY))
    x14_anaRightY = other.x14_anaRightY;
  if (std::fabs(other.x18_anaLeftTrigger) > std::fabs(x18_anaLeftTrigger))
    x18_anaLeftTrigger = other.x18_anaLeftTrigger;
  if (std::fabs(other.x1c_anaRightTrigger) > std::fabs(x1c_anaRightTrigger))
    x1c_anaRightTrigger = other.x1c_anaRightTrigger;
  x20_enableAnaLeftXP |= other.x20_enableAnaLeftXP;
  x21_enableAnaLeftYP |= other.x21_enableAnaLeftYP;
  x22_enableAnaRightXP |= other.x22_enableAnaRightXP;
  x23_enableAnaRightYP |= other.x23_enableAnaRightYP;
  x24_anaLeftTriggerP = other.x24_anaLeftTriggerP;
  x28_anaRightTriggerP = other.x28_anaRightTriggerP;
  x2c_b24_A |= other.x2c_b24_A;
  x2c_b25_B |= other.x2c_b25_B;
  x2c_b26_X |= other.x2c_b26_X;
  x2c_b27_Y |= other.x2c_b27_Y;
  x2c_b28_Z |= other.x2c_b28_Z;
  x2c_b29_L |= other.x2c_b29_L;
  x2c_b30_R |= other.x2c_b30_R;
  x2c_b31_DPUp |= other.x2c_b31_DPUp;
  x2d_b24_DPRight |= other.x2d_b24_DPRight;
  x2d_b25_DPDown |= other.x2d_b25_DPDown;
  x2d_b26_DPLeft |= other.x2d_b26_DPLeft;
  x2d_b27_Start |= other.x2d_b27_Start;
  x2d_b28_PA |= other.x2d_b28_PA;
  x2d_b29_PB |= other.x2d_b29_PB;
  x2d_b30_PX |= other.x2d_b30_PX;
  x2d_b31_PY |= other.x2d_b31_PY;
  x2e_b24_PZ |= other.x2e_b24_PZ;
  x2e_b25_PL |= other.x2e_b25_PL;
  x2e_b26_PR |= other.x2e_b26_PR;
  x2e_b27_PDPUp |= other.x2e_b27_PDPUp;
  x2e_b28_PDPRight |= other.x2e_b28_PDPRight;
  x2e_b29_PDPDown |= other.x2e_b29_PDPDown;
  x2e_b30_PDPLeft |= other.x2e_b30_PDPLeft;
  x2e_b31_PStart |= other.x2e_b31_PStart;
  if (other.m_kbm) {
    m_kbm = other.m_kbm;
    m_PCharKeys = other.m_PCharKeys;
    m_PSpecialKeys = other.m_PSpecialKeys;
    m_PMouseButtons = other.m_PMouseButtons;
  }
  m_which = other.m_which;
  return *this;
}
#endif

CFinalInput CFinalInput::ScaleAnalogueSticks(float leftDiv, float rightDiv) const {
  CFinalInput ret = *this;
  ret.x8_anaLeftX = zeus::clamp(-1.f, x8_anaLeftX / leftDiv, 1.f);
  ret.xc_anaLeftY = zeus::clamp(-1.f, xc_anaLeftY / leftDiv, 1.f);
  ret.x10_anaRightX = zeus::clamp(-1.f, x10_anaRightX / rightDiv, 1.f);
  ret.x14_anaRightY = zeus::clamp(-1.f, x14_anaRightY / rightDiv, 1.f);
  return ret;
}

static std::array<std::array<bool, 4>, 4> sIsAnalogPressed{};

void CFinalInput::InitializeAnalog(float leftDiv, float rightDiv) {
  x8_anaLeftX = zeus::clamp(-1.f, x8_anaLeftX / leftDiv, 1.f);
  xc_anaLeftY = zeus::clamp(-1.f, xc_anaLeftY / leftDiv, 1.f);
  x10_anaRightX = zeus::clamp(-1.f, x10_anaRightX / rightDiv, 1.f);
  x14_anaRightY = zeus::clamp(-1.f, x14_anaRightY / rightDiv, 1.f);

  if (xc_anaLeftY > 0.7f && !sIsAnalogPressed[x4_controllerIdx][0]) {
    sIsAnalogPressed[x4_controllerIdx][0] = true;
    x21_enableAnaLeftYP = true;
  } else if (xc_anaLeftY > 0.7f && sIsAnalogPressed[x4_controllerIdx][0]) {
    x21_enableAnaLeftYP = false;
  } else if (xc_anaLeftY < 0.7f && !sIsAnalogPressed[x4_controllerIdx][0]) {
    x21_enableAnaLeftYP = true;
    sIsAnalogPressed[x4_controllerIdx][0] = true;
  } else if (xc_anaLeftY < 0.7f && sIsAnalogPressed[x4_controllerIdx][0]) {
    x21_enableAnaLeftYP = false;
  } else if (std::fabs(xc_anaLeftY) < 0.7f) {
    x21_enableAnaLeftYP = false;
    sIsAnalogPressed[x4_controllerIdx][0] = false;
  }

  if (x8_anaLeftX > 0.7f && !sIsAnalogPressed[x4_controllerIdx][1]) {
    sIsAnalogPressed[x4_controllerIdx][1] = true;
    x20_enableAnaLeftXP = true;
  } else if (x8_anaLeftX > 0.7f && sIsAnalogPressed[x4_controllerIdx][1]) {
    x20_enableAnaLeftXP = false;
  } else if (x8_anaLeftX < 0.7f && !sIsAnalogPressed[x4_controllerIdx][1]) {
    x20_enableAnaLeftXP = true;
    sIsAnalogPressed[x4_controllerIdx][1] = true;
  } else if (x8_anaLeftX < 0.7f && sIsAnalogPressed[x4_controllerIdx][1]) {
    x20_enableAnaLeftXP = false;
  } else if (std::fabs(x8_anaLeftX) < 0.7f) {
    x20_enableAnaLeftXP = false;
    sIsAnalogPressed[x4_controllerIdx][1] = false;
  }

  if (x14_anaRightY > 0.7f && !sIsAnalogPressed[x4_controllerIdx][2]) {
    sIsAnalogPressed[x4_controllerIdx][2] = true;
    x23_enableAnaRightYP = true;
  } else if (x14_anaRightY > 0.7f && sIsAnalogPressed[x4_controllerIdx][2]) {
    x23_enableAnaRightYP = false;
  } else if (x14_anaRightY < 0.7f && !sIsAnalogPressed[x4_controllerIdx][2]) {
    x23_enableAnaRightYP = true;
    sIsAnalogPressed[x4_controllerIdx][2] = true;
  } else if (x14_anaRightY < 0.7f && sIsAnalogPressed[x4_controllerIdx][2]) {
    x23_enableAnaRightYP = false;
  } else if (std::fabs(x14_anaRightY) < 0.7f) {
    x23_enableAnaRightYP = false;
    sIsAnalogPressed[x4_controllerIdx][2] = false;
  }

  if (x10_anaRightX > 0.7f && !sIsAnalogPressed[x4_controllerIdx][3]) {
    sIsAnalogPressed[x4_controllerIdx][3] = true;
    x22_enableAnaRightXP = true;
  } else if (x10_anaRightX > 0.7f && sIsAnalogPressed[x4_controllerIdx][3]) {
    x22_enableAnaRightXP = false;
  } else if (x10_anaRightX < 0.7f && !sIsAnalogPressed[x4_controllerIdx][3]) {
    x22_enableAnaRightXP = true;
    sIsAnalogPressed[x4_controllerIdx][3] = true;
  } else if (x10_anaRightX < 0.7f && sIsAnalogPressed[x4_controllerIdx][3]) {
    x22_enableAnaRightXP = false;
  } else if (std::fabs(x10_anaRightX) < 0.7f) {
    x22_enableAnaRightXP = false;
    sIsAnalogPressed[x4_controllerIdx][3] = false;
  }
}

/* The following code is derived from pad.c in libogc
 *
 *  Copyright (C) 2004 - 2009
 *  Michael Wiedenbauer (shagkur)
 *  Dave Murphy (WinterMute)
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 *    must not claim that you wrote the original software. If you use
 *    this software in a product, an acknowledgment in the product
 *    documentation would be appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and
 *    must not be misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 */

constexpr std::array<int16_t, 8> pad_clampregion{
    30, 180, 15, 72, 40, 15, 59, 31,
};

static void pad_clampstick(int16_t& px, int16_t& py, int16_t max, int16_t xy, int16_t min) {
  int x = px;
  int y = py;

  int signX;
  if (x > 0) {
    signX = 1;
  } else {
    signX = -1;
    x = -x;
  }

  int signY;
  if (y > 0) {
    signY = 1;
  } else {
    signY = -1;
    y = -y;
  }

  if (x <= min) {
    x = 0;
  } else {
    x -= min;
  }

  if (y <= min) {
    y = 0;
  } else {
    y -= min;
  }

  if (x == 0 && y == 0) {
    px = py = 0;
    return;
  }

  if (xy * y <= xy * x) {
    const int d = xy * x + (max - xy) * y;
    if (xy * max < d) {
      x = int16_t(xy * max * x / d);
      y = int16_t(xy * max * y / d);
    }
  } else {
    const int d = xy * y + (max - xy) * x;
    if (xy * max < d) {
      x = int16_t(xy * max * x / d);
      y = int16_t(xy * max * y / d);
    }
  }

  px = int16_t(signX * x);
  py = int16_t(signY * y);
}

static void pad_clamptrigger(int16_t& trigger) {
  const int16_t min = pad_clampregion[0];
  const int16_t max = pad_clampregion[1];

  if (trigger <= min) {
    trigger = 0;
  } else {
    if (max < trigger) {
      trigger = max;
    }
    trigger -= min;
  }
}

void SAuroraControllerState::clamp() {
  pad_clampstick(m_axes[size_t(aurora::ControllerAxis::LeftX)], m_axes[size_t(aurora::ControllerAxis::LeftY)],
                 pad_clampregion[3], pad_clampregion[4], pad_clampregion[2]);
  pad_clampstick(m_axes[size_t(aurora::ControllerAxis::RightX)], m_axes[size_t(aurora::ControllerAxis::RightY)],
                 pad_clampregion[6], pad_clampregion[7], pad_clampregion[5]);
  pad_clamptrigger(m_axes[size_t(aurora::ControllerAxis::TriggerLeft)]);
  pad_clamptrigger(m_axes[size_t(aurora::ControllerAxis::TriggerRight)]);
}
} // namespace metaforce
