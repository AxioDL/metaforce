#include "Kyoto/Input/CFinalInput.hpp"

#include "Kyoto/Basics/COsContext.hpp"
#include "Kyoto/Input/CControllerGamepadData.hpp"
#include "Kyoto/Math/CMath.hpp"

const float CFinalInput::kInput_AnalogOnThreshhold = 0.7f;
const float CFinalInput::kInput_AnalogTriggerOnThreshhold = 0.05f;

CFinalInput::CFinalInput()
: mDt(0.f)
, mControllerIdx(0)
, mAnaLeftX(0.f)
, mAnaLeftY(0.f)
, mAnaRightX(0.f)
, mAnaRightY(0.f)
, mAnaLeftTrigger(0.f)
, mAnaRightTrigger(0.f)
, mEnableAnaLeftXP(false)
, mEnableAnaLeftYP(false)
, mEnableAnaRightXP(false)
, mEnableAnaRightYP(false)
, mAnaLeftTriggerP(0.f)
, mAnaRightTriggerP(0.f)
, mB24_A(false)
, mB25_B(false)
, mB26_X(false)
, mB27_Y(false)
, mB28_Z(false)
, mB29_L(false)
, mB30_R(false)
, mB31_DPUp(false)
, mB24_DPRight(false)
, mB25_DPDown(false)
, mB26_DPLeft(false)
, mB27_Start(false)
, mB28_PA(false)
, mB29_PB(false)
, mB30_PX(false)
, mB31_PY(false)
, mB24_PZ(false)
, mB25_PL(false)
, mB26_PR(false)
, mB27_PDPUp(false)
, mB28_PDPRight(false)
, mB29_PDPDown(false)
, mB30_PDPLeft(false)
, mB31_PStart(false) {}

CFinalInput::CFinalInput(int channel, float dt, const CControllerGamepadData& data, float leftDiv,
                         float rightDiv)
: mDt(dt)
, mControllerIdx(channel)
, mAnaLeftX(data.GetAxis(kJA_LeftX).GetAbsoluteValue())
, mAnaLeftY(data.GetAxis(kJA_LeftY).GetAbsoluteValue())
, mAnaRightX(data.GetAxis(kJA_RightX).GetAbsoluteValue())
, mAnaRightY(data.GetAxis(kJA_RightY).GetAbsoluteValue())
, mAnaLeftTrigger(data.GetAnalogButton(kBA_Left).GetAbsoluteValue())
, mAnaRightTrigger(data.GetAnalogButton(kBA_Right).GetAbsoluteValue())
, mAnaLeftTriggerP(data.GetAnalogButton(kBA_Left).GetRelativeValue())
, mAnaRightTriggerP(data.GetAnalogButton(kBA_Right).GetRelativeValue())
, mEnableAnaLeftXP(false)
, mEnableAnaLeftYP(false)
, mEnableAnaRightXP(false)
, mEnableAnaRightYP(false)
, mB24_A(data.GetButton(kBU_A).GetIsPressed())
, mB25_B(data.GetButton(kBU_B).GetIsPressed())
, mB26_X(data.GetButton(kBU_X).GetIsPressed())
, mB27_Y(data.GetButton(kBU_Y).GetIsPressed())
, mB28_Z(data.GetButton(kBU_Z).GetIsPressed())
, mB29_L(data.GetButton(kBU_L).GetIsPressed())
, mB30_R(data.GetButton(kBU_R).GetIsPressed())
, mB31_DPUp(data.GetButton(kBU_Up).GetIsPressed())
, mB24_DPRight(data.GetButton(kBU_Right).GetIsPressed())
, mB25_DPDown(data.GetButton(kBU_Down).GetIsPressed())
, mB26_DPLeft(data.GetButton(kBU_Left).GetIsPressed())
, mB27_Start(data.GetButton(kBU_Start).GetIsPressed())
, mB28_PA(data.GetButton(kBU_A).GetPressEvent())
, mB29_PB(data.GetButton(kBU_B).GetPressEvent())
, mB30_PX(data.GetButton(kBU_X).GetPressEvent())
, mB31_PY(data.GetButton(kBU_Y).GetPressEvent())
, mB24_PZ(data.GetButton(kBU_Z).GetPressEvent())
, mB25_PL(data.GetButton(kBU_L).GetPressEvent())
, mB26_PR(data.GetButton(kBU_R).GetPressEvent())
, mB27_PDPUp(data.GetButton(kBU_Up).GetPressEvent())
, mB28_PDPRight(data.GetButton(kBU_Right).GetPressEvent())
, mB29_PDPDown(data.GetButton(kBU_Down).GetPressEvent())
, mB30_PDPLeft(data.GetButton(kBU_Left).GetPressEvent())
, mB31_PStart(data.GetButton(kBU_Start).GetPressEvent()) {
  InitializeAnalog(leftDiv, rightDiv);
}

static bool sIsAnalogPressed[4][4] = {};

void CFinalInput::InitializeAnalog(float leftDiv, float rightDiv) {
  mAnaLeftX = CMath::Clamp(-1.f, mAnaLeftX / leftDiv, 1.f);
  mAnaLeftY = CMath::Clamp(-1.f, mAnaLeftY / leftDiv, 1.f);
  mAnaRightX = CMath::Clamp(-1.f, mAnaRightX / rightDiv, 1.f);
  mAnaRightY = CMath::Clamp(-1.f, mAnaRightY / rightDiv, 1.f);

  if (mAnaLeftY > 0.7f && !sIsAnalogPressed[mControllerIdx][0]) {
    sIsAnalogPressed[mControllerIdx][0] = true;
    mEnableAnaLeftYP = true;
  } else if (mAnaLeftY > 0.7f && sIsAnalogPressed[mControllerIdx][0]) {
    mEnableAnaLeftYP = false;
  } else if (mAnaLeftY < -0.7f && !sIsAnalogPressed[mControllerIdx][0]) {
    mEnableAnaLeftYP = true;
    sIsAnalogPressed[mControllerIdx][0] = true;
  } else if (mAnaLeftY < -0.7f && sIsAnalogPressed[mControllerIdx][0]) {
    mEnableAnaLeftYP = false;
  } else if (fabsf(mAnaLeftY) < 0.7f) {
    mEnableAnaLeftYP = false;
    sIsAnalogPressed[mControllerIdx][0] = false;
  }

  if (mAnaLeftX > 0.7f && !sIsAnalogPressed[mControllerIdx][1]) {
    mEnableAnaLeftXP = true;
    sIsAnalogPressed[mControllerIdx][1] = true;
  } else if (mAnaLeftX > 0.7f && sIsAnalogPressed[mControllerIdx][1]) {
    mEnableAnaLeftXP = false;
  } else if (mAnaLeftX < -0.7f && !sIsAnalogPressed[mControllerIdx][1]) {
    sIsAnalogPressed[mControllerIdx][1] = true;
    mEnableAnaLeftXP = true;
  } else if (mAnaLeftX < -0.7f && sIsAnalogPressed[mControllerIdx][1]) {
    mEnableAnaLeftXP = false;
  } else if (fabsf(mAnaLeftX) < 0.7f) {
    mEnableAnaLeftXP = false;
    sIsAnalogPressed[mControllerIdx][1] = false;
  }

  if (mAnaRightY > 0.7f && !sIsAnalogPressed[mControllerIdx][2]) {
    sIsAnalogPressed[mControllerIdx][2] = true;
    mEnableAnaRightYP = true;
  } else if (mAnaRightY > 0.7f && sIsAnalogPressed[mControllerIdx][2]) {
    mEnableAnaRightYP = false;
  } else if (mAnaRightY < -0.7f && !sIsAnalogPressed[mControllerIdx][2]) {
    mEnableAnaRightYP = true;
    sIsAnalogPressed[mControllerIdx][2] = true;
  } else if (mAnaRightY < -0.7f && sIsAnalogPressed[mControllerIdx][2]) {
    mEnableAnaRightYP = false;
  } else if (fabsf(mAnaRightY) < 0.7f) {
    mEnableAnaRightYP = false;
    sIsAnalogPressed[mControllerIdx][2] = false;
  }

  if (mAnaRightX > 0.7f && !sIsAnalogPressed[mControllerIdx][3]) {
    mEnableAnaRightXP = true;
    sIsAnalogPressed[mControllerIdx][3] = true;
  } else if (mAnaRightX > 0.7f && sIsAnalogPressed[mControllerIdx][3]) {
    mEnableAnaRightXP = false;
  } else if (mAnaRightX < -0.7f && !sIsAnalogPressed[mControllerIdx][3]) {
    sIsAnalogPressed[mControllerIdx][3] = true;
    mEnableAnaRightXP = true;
  } else if (mAnaRightX < -0.7f && sIsAnalogPressed[mControllerIdx][3]) {
    mEnableAnaRightXP = false;
  } else if (fabsf(mAnaRightX) < 0.7f) {
    mEnableAnaRightXP = false;
    sIsAnalogPressed[mControllerIdx][3] = false;
  }
}

CFinalInput::CFinalInput(int channel, float dt, const COsContext& ctx)
: mDt(dt)
, mControllerIdx(channel)
, mAnaLeftX(0.f)
, mAnaLeftY(0.f)
, mAnaRightX(0.f)
, mAnaRightY(0.f)
, mAnaLeftTrigger(0.f)
, mAnaRightTrigger(0.f)
#if NONMATCHING
, mEnableAnaLeftXP(false)
, mEnableAnaLeftYP(false)
#endif
, mEnableAnaRightXP(false)
, mEnableAnaRightYP(false)
, mAnaLeftTriggerP(0.f)
, mAnaRightTriggerP(0.f)
, mB24_A(ctx.GetOsKeyState(0x6d).IsPressed())
, mB25_B(ctx.GetOsKeyState(0x6e).IsPressed())
, mB26_X(ctx.GetOsKeyState(0x84).IsPressed())
, mB27_Y(ctx.GetOsKeyState(0x85).IsPressed())
, mB28_Z(ctx.GetOsKeyState(0x16).IsPressed())
, mB29_L(ctx.GetOsKeyState(0x78).IsPressed())
, mB30_R(ctx.GetOsKeyState(0x7e).IsPressed())
, mB31_DPUp(ctx.GetOsKeyState(0x1c).IsPressed())
, mB24_DPRight(ctx.GetOsKeyState(0x1d).IsPressed())
, mB25_DPDown(ctx.GetOsKeyState(0x1e).IsPressed())
, mB26_DPLeft(ctx.GetOsKeyState(0x1b).IsPressed())
, mB27_Start(ctx.GetOsKeyState(5).IsPressed())
, mB28_PA(ctx.GetOsKeyState(0x6d).JustPressed())
, mB29_PB(ctx.GetOsKeyState(0x6e).JustPressed())
, mB30_PX(ctx.GetOsKeyState(0x84).JustPressed())
, mB31_PY(ctx.GetOsKeyState(0x85).JustPressed())
, mB24_PZ(ctx.GetOsKeyState(0x16).JustPressed())
, mB25_PL(ctx.GetOsKeyState(0x78).JustPressed())
, mB26_PR(ctx.GetOsKeyState(0x7e).JustPressed())
, mB27_PDPUp(ctx.GetOsKeyState(0x1c).JustPressed())
, mB28_PDPRight(ctx.GetOsKeyState(0x1d).JustPressed())
, mB29_PDPDown(ctx.GetOsKeyState(0x1e).JustPressed())
, mB30_PDPLeft(ctx.GetOsKeyState(0x1b).JustPressed())
, mB31_PStart(ctx.GetOsKeyState(5).JustPressed()) {}

CFinalInput CFinalInput::ScaleAnalogueSticks(float leftDiv, float rightDiv) const {
  CFinalInput ret = *this;
  ret.mAnaLeftX = CMath::Clamp(-1.f, mAnaLeftX / leftDiv, 1.f);
  ret.mAnaLeftY = CMath::Clamp(-1.f, mAnaLeftY / leftDiv, 1.f);
  ret.mAnaRightX = CMath::Clamp(-1.f, mAnaRightX / rightDiv, 1.f);
  ret.mAnaRightY = CMath::Clamp(-1.f, mAnaRightY / rightDiv, 1.f);
  return ret;
}
