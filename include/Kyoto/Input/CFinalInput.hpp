#ifndef _CFINALINPUT
#define _CFINALINPUT

#include <types.h>

#ifdef __MWERKS__
#pragma cpp_extensions on
#endif

class CControllerGamepadData;
class COsContext;

class CFinalInput {
  static const float kInput_AnalogOnThreshhold;
  static const float kInput_AnalogTriggerOnThreshhold;

public:
  CFinalInput();
  CFinalInput(const int channel, const float dt, const CControllerGamepadData& data, const float leftDiv,
              const float rightDiv);
  CFinalInput(int channel, float dt, const COsContext& context);
  CFinalInput(const CFinalInput& other)
  : mDt(other.mDt)
  , mControllerIdx(other.mControllerIdx)
  , mAnaLeftX(other.mAnaLeftX)
  , mAnaLeftY(other.mAnaLeftY)
  , mAnaRightX(other.mAnaRightX)
  , mAnaRightY(other.mAnaRightY)
  , mAnaLeftTrigger(other.mAnaLeftTrigger)
  , mAnaRightTrigger(other.mAnaRightTrigger)
  , mEnableAnaLeftXP(other.mEnableAnaLeftXP)
  , mEnableAnaLeftYP(other.mEnableAnaLeftYP)
  , mEnableAnaRightXP(other.mEnableAnaRightXP)
  , mEnableAnaRightYP(other.mEnableAnaRightYP)
  , mAnaLeftTriggerP(other.mAnaLeftTriggerP)
  , mAnaRightTriggerP(other.mAnaRightTriggerP)
  , btns1(other.btns1)
  , btns2(other.btns2)
  , btns3(other.btns3) {}

  void InitializeAnalog(float leftDiv, float rightDiv);

  CFinalInput ScaleAnalogueSticks(float leftDiv, float rightDiv) const;

  float Time() const { return mDt; }
  void SetTime(float time) { mDt = time; }

  int ControllerNumber() const { return mControllerIdx; }

  float ALAUp() const { return mAnaLeftY > 0.f ? mAnaLeftY : 0.f; }

  float ALADown() const { return mAnaLeftY < 0.f ? -mAnaLeftY : 0.f; }

  float ALALeft() const { return mAnaLeftX < 0.f ? -mAnaLeftX : 0.f; }

  float ALARight() const { return mAnaLeftX > 0.f ? mAnaLeftX : 0.f; }

  float ARAUp() const { return mAnaRightY > 0.f ? mAnaRightY : 0.f; }

  float ARADown() const { return mAnaRightY < 0.f ? -mAnaRightY : 0.f; }

  float ARALeft() const { return mAnaRightX < 0.f ? -mAnaRightX : 0.f; }

  float ARARight() const { return mAnaRightX > 0.f ? mAnaRightX : 0.f; }

  float ALTrigger() const { return mAnaLeftTrigger; }

  float ARTrigger() const { return mAnaRightTrigger; }

  float ADPUp() const { return mB31_DPUp ? 1.f : 0.f; }

  float ADPDown() const { return mB25_DPDown ? 1.f : 0.f; }

  float ADPLeft() const { return mB26_DPLeft ? 1.f : 0.f; }

  float ADPRight() const { return mB24_DPRight ? 1.f : 0.f; }

  float AA() const { return mB24_A ? 1.f : 0.f; }

  float AB() const { return mB25_B ? 1.f : 0.f; }

  float AX() const { return mB26_X ? 1.f : 0.f; }

  float AY() const { return mB27_Y ? 1.f : 0.f; }

  float AZ() const { return mB28_Z ? 1.f : 0.f; }

  float AL() const { return mB29_L ? 1.f : 0.f; }

  float AR() const { return mB30_R ? 1.f : 0.f; }

  float AStart() const { return mB27_Start ? 1.f : 0.f; }

  float ALeftX() const { return mAnaLeftX; }

  float ALeftY() const { return mAnaLeftY; }

  bool DLAUp() const { return kInput_AnalogOnThreshhold < mAnaLeftY ? true : false; }

  bool DLADown() const { return -kInput_AnalogOnThreshhold > mAnaLeftY ? true : false; }

  bool DLALeft() const { return -kInput_AnalogOnThreshhold > mAnaLeftX ? true : false; }

  bool DLARight() const { return kInput_AnalogOnThreshhold < mAnaLeftX ? true : false; }

  bool DRAUp() const { return kInput_AnalogOnThreshhold < mAnaRightY ? true : false; }

  bool DRADown() const { return -kInput_AnalogOnThreshhold > mAnaRightY ? true : false; }

  bool DRALeft() const { return -kInput_AnalogOnThreshhold > mAnaRightX ? true : false; }

  bool DRARight() const { return kInput_AnalogOnThreshhold < mAnaRightX ? true : false; }

  bool DLTrigger() const {
    return kInput_AnalogTriggerOnThreshhold < mAnaLeftTrigger ? true : false;
  }

  bool DRTrigger() const {
    return kInput_AnalogTriggerOnThreshhold < mAnaRightTrigger ? true : false;
  }

  bool DDPUp() const { return mB31_DPUp; }

  bool DDPDown() const { return mB25_DPDown; }

  bool DDPLeft() const { return mB26_DPLeft; }

  bool DDPRight() const { return mB24_DPRight; }

  bool DA() const { return mB24_A; }

  bool DB() const { return mB25_B; }

  bool DX() const { return mB26_X; }

  bool DY() const { return mB27_Y; }

  bool DZ() const { return mB28_Z; }

  bool DL() const { return mB29_L; }

  bool DR() const { return mB30_R; }

  bool DStart() const { return mB27_Start; }

  bool PLAUp() const { return DLAUp() && mEnableAnaLeftYP ? true : false; }

  bool PLADown() const { return -kInput_AnalogOnThreshhold > mAnaLeftY && mEnableAnaLeftYP ? true : false; }

  bool PLALeft() const { return -kInput_AnalogOnThreshhold > mAnaLeftX && mEnableAnaLeftXP ? true : false; }

  bool PLARight() const { return kInput_AnalogOnThreshhold < mAnaLeftX && mEnableAnaLeftXP ? true : false; }

  bool PRAUp() const { return kInput_AnalogOnThreshhold < mAnaRightY && mEnableAnaRightYP ? true : false; }

  bool PRADown() const { return -kInput_AnalogOnThreshhold > mAnaRightY && mEnableAnaRightYP ? true : false; }

  bool PRALeft() const { return -kInput_AnalogOnThreshhold > mAnaRightX && mEnableAnaRightXP ? true : false; }

  bool PRARight() const { return kInput_AnalogOnThreshhold < mAnaRightX && mEnableAnaRightXP ? true : false; }

  bool PLTrigger() const { return kInput_AnalogTriggerOnThreshhold < mAnaLeftTriggerP ? true : false; }

  bool PRTrigger() const { return kInput_AnalogTriggerOnThreshhold < mAnaRightTriggerP ? true : false; }

  bool PDPUp() const { return mB27_PDPUp; }
  
  bool PDPDown() const { return mB29_PDPDown; }
  
  bool PDPLeft() const { return mB30_PDPLeft; }
  
  bool PDPRight() const { return mB28_PDPRight; }

  bool PA() const { return mB28_PA; }

  bool PB() const { return mB29_PB; }

  bool PX() const { return mB30_PX; }

  bool PY() const { return mB31_PY; }

  bool PZ() const { return mB24_PZ; }

  bool PL() const { return mB25_PL; }

  bool PR() const { return mB26_PR; }

  bool PStart() const { return mB31_PStart; }

private:
  float mDt;
  int mControllerIdx;
  float mAnaLeftX;
  float mAnaLeftY;
  float mAnaRightX;
  float mAnaRightY;
  float mAnaLeftTrigger;
  float mAnaRightTrigger;

  bool mEnableAnaLeftXP;
  bool mEnableAnaLeftYP;
  bool mEnableAnaRightXP;
  bool mEnableAnaRightYP;

  float mAnaLeftTriggerP;
  float mAnaRightTriggerP;

  union {
    struct {
      uchar mB24_A : 1;
      uchar mB25_B : 1;
      uchar mB26_X : 1;
      uchar mB27_Y : 1;
      uchar mB28_Z : 1;
      uchar mB29_L : 1;
      uchar mB30_R : 1;
      uchar mB31_DPUp : 1;
    };
    uchar btns1;
  };
  union {
    struct {
      uchar mB24_DPRight : 1;
      uchar mB25_DPDown : 1;
      uchar mB26_DPLeft : 1;
      uchar mB27_Start : 1;
      uchar mB28_PA : 1;
      uchar mB29_PB : 1;
      uchar mB30_PX : 1;
      uchar mB31_PY : 1;
    };
    uchar btns2;
  };
  union {
    struct {
      uchar mB24_PZ : 1;
      uchar mB25_PL : 1;
      uchar mB26_PR : 1;
      uchar mB27_PDPUp : 1;
      uchar mB28_PDPRight : 1;
      uchar mB29_PDPDown : 1;
      uchar mB30_PDPLeft : 1;
      uchar mB31_PStart : 1;
    };
    uchar btns3;
  };
};

CHECK_SIZEOF(CFinalInput, 48)

#ifdef __MWERKS__
#pragma cpp_extensions reset
#endif

#endif // _CFINALINPUT
