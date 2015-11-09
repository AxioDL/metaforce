#include <Math.hpp>
#include "CFinalInput.hpp"

namespace Retro
{

CFinalInput::CFinalInput()
: x0_dt(0.0),
  x4_controllerIdx(0),
  x8_anaLeftX(0.0),
  xc_anaLeftY(0.0),
  x10_anaRightX(0.0),
  x14_anaRightY(0.0),
  x18_anaLeftTrigger(0.0),
  x1c_anaRightTrigger(0.0),
  x20_enableAnaLeftXP(false),
  x20_enableAnaLeftNegXP(false),
  x21_enableAnaLeftYP(false),
  x21_enableAnaLeftNegYP(false),
  x22_enableAnaRightXP(false),
  x22_enableAnaRightNegXP(false),
  x23_enableAnaRightYP(false),
  x23_enableAnaRightNegYP(false),
  x24_anaLeftTriggerP(false),
  x28_anaRightTriggerP(false),
  x2c_b24_A(false),
  x2c_b25_B(false),
  x2c_b26_X(false),
  x2c_b27_Y(false),
  x2c_b28_Z(false),
  x2c_b29_L(false),
  x2c_b30_R(false),
  x2c_b31_DPUp(false),
  x2d_b24_DPRight(false),
  x2d_b25_DPDown(false),
  x2d_b26_DPLeft(false),
  x2d_b27_Start(false),
  x2d_b28_PA(false),
  x2d_b29_PB(false),
  x2d_b30_PX(false),
  x2d_b31_PY(false),
  x2e_b24_PZ(false),
  x2e_b25_PL(false),
  x2e_b26_PR(false),
  x2e_b27_PDPUp(false),
  x2e_b28_PDPRight(false),
  x2e_b29_PDPDown(false),
  x2e_b30_PDPLeft(false),
  x2e_b31_PStart(false) {}


CFinalInput::CFinalInput(int cIdx, float dt,
                         const boo::DolphinControllerState& data,
                         const CFinalInput& prevInput,
                         float leftDiv, float rightDiv)
: x0_dt(dt),
  x4_controllerIdx(cIdx),
  x8_anaLeftX(Zeus::Math::clamp(-1.0f, data.m_leftStick[0] / 72.0f / leftDiv, 1.0f)),
  xc_anaLeftY(Zeus::Math::clamp(-1.0f, data.m_leftStick[1] / 72.0f / leftDiv, 1.0f)),
  x10_anaRightX(Zeus::Math::clamp(-1.0f, data.m_rightStick[0] / 59.0f / rightDiv, 1.0f)),
  x14_anaRightY(Zeus::Math::clamp(-1.0f, data.m_rightStick[0] / 59.0f / rightDiv, 1.0f)),
  x18_anaLeftTrigger(data.m_analogTriggers[0] * 0.0066666668),
  x1c_anaRightTrigger(data.m_analogTriggers[1] * 0.0066666668),
  x20_enableAnaLeftXP(DLARight() && !prevInput.DLARight()),
  x20_enableAnaLeftNegXP(DLALeft() && !prevInput.DLALeft()),
  x21_enableAnaLeftYP(DLAUp() && !prevInput.DLAUp()),
  x21_enableAnaLeftNegYP(DLADown() && !prevInput.DLADown()),
  x22_enableAnaRightXP(DRARight() && !prevInput.DRARight()),
  x22_enableAnaRightNegXP(DRALeft() && !prevInput.DRALeft()),
  x23_enableAnaRightYP(DRAUp() && !prevInput.DRAUp()),
  x23_enableAnaRightNegYP(DRADown() && !prevInput.DRADown()),
  x24_anaLeftTriggerP(DLTrigger() && !prevInput.DLTrigger()),
  x28_anaRightTriggerP(DRTrigger() && !prevInput.DRTrigger()),
  x2c_b24_A(data.m_btns & boo::DOL_A),
  x2c_b25_B(data.m_btns & boo::DOL_B),
  x2c_b26_X(data.m_btns & boo::DOL_X),
  x2c_b27_Y(data.m_btns & boo::DOL_Y),
  x2c_b28_Z(data.m_btns & boo::DOL_Z),
  x2c_b29_L(data.m_btns & boo::DOL_L),
  x2c_b30_R(data.m_btns & boo::DOL_R),
  x2c_b31_DPUp(data.m_btns & boo::DOL_UP),
  x2d_b24_DPRight(data.m_btns & boo::DOL_RIGHT),
  x2d_b25_DPDown(data.m_btns & boo::DOL_DOWN),
  x2d_b26_DPLeft(data.m_btns & boo::DOL_LEFT),
  x2d_b27_Start(data.m_btns & boo::DOL_START),
  x2d_b28_PA(DA() && !prevInput.DA()),
  x2d_b29_PB(DB() && !prevInput.DB()),
  x2d_b30_PX(DX() && !prevInput.DX()),
  x2d_b31_PY(DY() && !prevInput.DY()),
  x2e_b24_PZ(DZ() && !prevInput.DZ()),
  x2e_b25_PL(DL() && !prevInput.DL()),
  x2e_b26_PR(DR() && !prevInput.DR()),
  x2e_b27_PDPUp(DDPUp() && !prevInput.DDPUp()),
  x2e_b28_PDPRight(DDPRight() && !prevInput.DDPRight()),
  x2e_b29_PDPDown(DDPDown() && !prevInput.DDPDown()),
  x2e_b30_PDPLeft(DDPLeft() && !prevInput.DDPLeft()),
  x2e_b31_PStart(DStart() && !prevInput.DStart()) {}

static float KBToAnaLeftX(const CKeyboardMouseControllerData& data)
{
    float retval = 0.0;
    if (data.m_charKeys['a'])
        retval -= 1.0;
    if (data.m_charKeys['d'])
        retval += 1.0;
    return retval;
}

static float KBToAnaLeftY(const CKeyboardMouseControllerData& data)
{
    float retval = 0.0;
    if (data.m_charKeys['s'])
        retval -= 1.0;
    if (data.m_charKeys['w'])
        retval += 1.0;
    return retval;
}

static float KBToAnaRightX(const CKeyboardMouseControllerData& data)
{
    float retval = 0.0;
    if (data.m_charKeys['2'])
        retval -= 1.0;
    if (data.m_charKeys['4'])
        retval += 1.0;
    return retval;
}

static float KBToAnaRightY(const CKeyboardMouseControllerData& data)
{
    float retval = 0.0;
    if (data.m_charKeys['3'])
        retval -= 1.0;
    if (data.m_charKeys['1'])
        retval += 1.0;
    return retval;
}

CFinalInput::CFinalInput(int cIdx, float dt,
                         const CKeyboardMouseControllerData& data,
                         const CFinalInput& prevInput)
: x0_dt(dt),
  x4_controllerIdx(cIdx),
  x8_anaLeftX(KBToAnaLeftX(data)),
  xc_anaLeftY(KBToAnaLeftY(data)),
  x10_anaRightX(KBToAnaRightX(data)),
  x14_anaRightY(KBToAnaRightY(data)),
  x18_anaLeftTrigger(data.m_charKeys['q'] ? 1.0 : 0.0),
  x1c_anaRightTrigger(data.m_charKeys['e'] ? 1.0 : 0.0),
  x20_enableAnaLeftXP(DLARight() && !prevInput.DLARight()),
  x20_enableAnaLeftNegXP(DLALeft() && !prevInput.DLALeft()),
  x21_enableAnaLeftYP(DLAUp() && !prevInput.DLAUp()),
  x21_enableAnaLeftNegYP(DLADown() && !prevInput.DLADown()),
  x22_enableAnaRightXP(DRARight() && !prevInput.DRARight()),
  x22_enableAnaRightNegXP(DRALeft() && !prevInput.DRALeft()),
  x23_enableAnaRightYP(DRAUp() && !prevInput.DRAUp()),
  x23_enableAnaRightNegYP(DRADown() && !prevInput.DRADown()),
  x24_anaLeftTriggerP(DLTrigger() && !prevInput.DLTrigger()),
  x28_anaRightTriggerP(DRTrigger() && !prevInput.DRTrigger()),
  x2c_b24_A(data.m_mouseButtons[boo::BUTTON_PRIMARY]),
  x2c_b25_B(data.m_charKeys[' ']),
  x2c_b26_X(data.m_charKeys['c']),
  x2c_b27_Y(data.m_mouseButtons[boo::BUTTON_SECONDARY]),
  x2c_b28_Z(data.m_charKeys['\t']),
  x2c_b29_L(data.m_charKeys['q']),
  x2c_b30_R(data.m_charKeys['e']),
  x2c_b31_DPUp(data.m_specialKeys[boo::KEY_UP]),
  x2d_b24_DPRight(data.m_specialKeys[boo::KEY_RIGHT]),
  x2d_b25_DPDown(data.m_specialKeys[boo::KEY_DOWN]),
  x2d_b26_DPLeft(data.m_specialKeys[boo::KEY_LEFT]),
  x2d_b27_Start(data.m_specialKeys[boo::KEY_ESC]),
  x2d_b28_PA(DA() && !prevInput.DA()),
  x2d_b29_PB(DB() && !prevInput.DB()),
  x2d_b30_PX(DX() && !prevInput.DX()),
  x2d_b31_PY(DY() && !prevInput.DY()),
  x2e_b24_PZ(DZ() && !prevInput.DZ()),
  x2e_b25_PL(DL() && !prevInput.DL()),
  x2e_b26_PR(DR() && !prevInput.DR()),
  x2e_b27_PDPUp(DDPUp() && !prevInput.DDPUp()),
  x2e_b28_PDPRight(DDPRight() && !prevInput.DDPRight()),
  x2e_b29_PDPDown(DDPDown() && !prevInput.DDPDown()),
  x2e_b30_PDPLeft(DDPLeft() && !prevInput.DDPLeft()),
  x2e_b31_PStart(DStart() && !prevInput.DStart())
{
    if (x8_anaLeftX || xc_anaLeftY)
    {
        float len = sqrtf(x8_anaLeftX * x8_anaLeftX + xc_anaLeftY * xc_anaLeftY);
        x8_anaLeftX /= len;
        xc_anaLeftY /= len;
    }
    if (x10_anaRightX || x14_anaRightY)
    {
        float len = sqrtf(x10_anaRightX * x10_anaRightX + x14_anaRightY * x14_anaRightY);
        x10_anaRightX /= len;
        x14_anaRightY /= len;
    }
}

CFinalInput& CFinalInput::operator|=(const CFinalInput& other)
{
    if (fabsf(other.x8_anaLeftX) > fabsf(x8_anaLeftX))
        x8_anaLeftX = other.x8_anaLeftX;
    if (fabsf(other.xc_anaLeftY) > fabsf(xc_anaLeftY))
        xc_anaLeftY = other.xc_anaLeftY;
    if (fabsf(other.x10_anaRightX) > fabsf(x10_anaRightX))
        x10_anaRightX = other.x10_anaRightX;
    if (fabsf(other.x14_anaRightY) > fabsf(x14_anaRightY))
        x14_anaRightY = other.x14_anaRightY;
    if (fabsf(other.x18_anaLeftTrigger) > fabsf(x18_anaLeftTrigger))
        x18_anaLeftTrigger = other.x18_anaLeftTrigger;
    if (fabsf(other.x1c_anaRightTrigger) > fabsf(x1c_anaRightTrigger))
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
    return *this;
}

}
