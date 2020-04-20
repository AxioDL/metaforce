#include "Runtime/Input/CFinalInput.hpp"

#include <zeus/Math.hpp>

namespace urde {

CFinalInput::CFinalInput() = default;

CFinalInput::CFinalInput(int cIdx, float dt, const boo::DolphinControllerState& data, const CFinalInput& prevInput,
                         float leftDiv, float rightDiv)
: x0_dt(dt)
, x4_controllerIdx(cIdx)
, x8_anaLeftX(zeus::clamp(-1.0f, data.m_leftStick[0] / 72.0f / leftDiv, 1.0f))
, xc_anaLeftY(zeus::clamp(-1.0f, data.m_leftStick[1] / 72.0f / leftDiv, 1.0f))
, x10_anaRightX(zeus::clamp(-1.0f, data.m_rightStick[0] / 59.0f / rightDiv, 1.0f))
, x14_anaRightY(zeus::clamp(-1.0f, data.m_rightStick[1] / 59.0f / rightDiv, 1.0f))
, x18_anaLeftTrigger(data.m_analogTriggers[0] * 0.007f)
, x1c_anaRightTrigger(data.m_analogTriggers[1] * 0.007f)
, x20_enableAnaLeftXP(DLARight() && !prevInput.DLARight())
, x20_enableAnaLeftNegXP(DLALeft() && !prevInput.DLALeft())
, x21_enableAnaLeftYP(DLAUp() && !prevInput.DLAUp())
, x21_enableAnaLeftNegYP(DLADown() && !prevInput.DLADown())
, x22_enableAnaRightXP(DRARight() && !prevInput.DRARight())
, x22_enableAnaRightNegXP(DRALeft() && !prevInput.DRALeft())
, x23_enableAnaRightYP(DRAUp() && !prevInput.DRAUp())
, x23_enableAnaRightNegYP(DRADown() && !prevInput.DRADown())
, x24_anaLeftTriggerP(DLTrigger() && !prevInput.DLTrigger())
, x28_anaRightTriggerP(DRTrigger() && !prevInput.DRTrigger())
, x2c_b24_A(data.m_btns & uint16_t(boo::EDolphinControllerButtons::A))
, x2c_b25_B(data.m_btns & uint16_t(boo::EDolphinControllerButtons::B))
, x2c_b26_X(data.m_btns & uint16_t(boo::EDolphinControllerButtons::X))
, x2c_b27_Y(data.m_btns & uint16_t(boo::EDolphinControllerButtons::Y))
, x2c_b28_Z(data.m_btns & uint16_t(boo::EDolphinControllerButtons::Z))
, x2c_b29_L(data.m_btns & uint16_t(boo::EDolphinControllerButtons::L))
, x2c_b30_R(data.m_btns & uint16_t(boo::EDolphinControllerButtons::R))
, x2c_b31_DPUp(data.m_btns & uint16_t(boo::EDolphinControllerButtons::Up))
, x2d_b24_DPRight(data.m_btns & uint16_t(boo::EDolphinControllerButtons::Right))
, x2d_b25_DPDown(data.m_btns & uint16_t(boo::EDolphinControllerButtons::Down))
, x2d_b26_DPLeft(data.m_btns & uint16_t(boo::EDolphinControllerButtons::Left))
, x2d_b27_Start(data.m_btns & uint16_t(boo::EDolphinControllerButtons::Start))
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
, x2e_b31_PStart(DStart() && !prevInput.DStart()) {}

CFinalInput::CFinalInput(int cIdx, float dt, const CKeyboardMouseControllerData& data, const CFinalInput& prevInput)
: x0_dt(dt)
, x4_controllerIdx(cIdx)
, x18_anaLeftTrigger(false)
, x1c_anaRightTrigger(false)
, x20_enableAnaLeftXP(DLARight() && !prevInput.DLARight())
, x20_enableAnaLeftNegXP(DLALeft() && !prevInput.DLALeft())
, x21_enableAnaLeftYP(DLAUp() && !prevInput.DLAUp())
, x21_enableAnaLeftNegYP(DLADown() && !prevInput.DLADown())
, x22_enableAnaRightXP(DRARight() && !prevInput.DRARight())
, x22_enableAnaRightNegXP(DRALeft() && !prevInput.DRALeft())
, x23_enableAnaRightYP(DRAUp() && !prevInput.DRAUp())
, x23_enableAnaRightNegYP(DRADown() && !prevInput.DRADown())
, x24_anaLeftTriggerP(DLTrigger() && !prevInput.DLTrigger())
, x28_anaRightTriggerP(DRTrigger() && !prevInput.DRTrigger())
, x2c_b31_DPUp(data.m_specialKeys[size_t(boo::ESpecialKey::Up)])
, x2d_b24_DPRight(data.m_specialKeys[size_t(boo::ESpecialKey::Right)])
, x2d_b25_DPDown(data.m_specialKeys[size_t(boo::ESpecialKey::Down)])
, x2d_b26_DPLeft(data.m_specialKeys[size_t(boo::ESpecialKey::Left)])
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
  x20_enableAnaLeftNegXP |= other.x20_enableAnaLeftNegXP;
  x21_enableAnaLeftYP |= other.x21_enableAnaLeftYP;
  x21_enableAnaLeftNegYP |= other.x21_enableAnaLeftNegYP;
  x22_enableAnaRightXP |= other.x22_enableAnaRightXP;
  x22_enableAnaRightNegXP |= other.x22_enableAnaRightNegXP;
  x23_enableAnaRightYP |= other.x23_enableAnaRightYP;
  x23_enableAnaRightNegYP |= other.x23_enableAnaRightNegYP;
  x24_anaLeftTriggerP |= other.x24_anaLeftTriggerP;
  x28_anaRightTriggerP |= other.x28_anaRightTriggerP;
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
  return *this;
}

CFinalInput CFinalInput::ScaleAnalogueSticks(float leftDiv, float rightDiv) const {
  CFinalInput ret = *this;
  ret.x8_anaLeftX = zeus::clamp(-1.f, x8_anaLeftX / leftDiv, 1.f);
  ret.xc_anaLeftY = zeus::clamp(-1.f, xc_anaLeftY / leftDiv, 1.f);
  ret.x10_anaRightX = zeus::clamp(-1.f, x10_anaRightX / rightDiv, 1.f);
  ret.x14_anaRightY = zeus::clamp(-1.f, x14_anaRightY / rightDiv, 1.f);
  ret.m_leftMul = 1.f / leftDiv;
  ret.m_rightMul = 1.f / rightDiv;
  return ret;
}
} // namespace urde
