#ifndef _CFIDGET
#define _CFIDGET

#include "types.h"

#include "MetroidPrime/Weapons/GunController/CGunMotion.hpp"

class CStateManager;

class CFidget {
public:
  enum EState {
    kS_NoFidget,
    kS_MinorFidget,
    kS_MajorFidget,
    kS_HolsterBeam,
    kS_StillMinorFidget,
    kS_StillMajorFidget,
    kS_StillHolsterBeam,
    kS_Loading,
  };

private:
  EState mState;
  SamusGun::EFidgetType mType;
  int mDelayTriggerBits;
  // 0: panel, 1: panel reset, 2: adjust nozzle, 3: panel buttons
  int mAnimSet;
  int mDelayTimerEnableBits;
  float mTimeSinceFire;
  float mTimeSinceStrikeCooldown;
  float mTimeSinceUnmorph;
  float mTimeSinceBobbing;
  float mMinorDelayTimer;
  float mMajorDelayTimer;
  float mHolsterTimeSinceFire;
  float mTimeUntilHolster;
  bool mLoading : 1;

public:
  CFidget();

  EState GetState() const { return mState; }
  SamusGun::EFidgetType GetType() const { return mType; }
  int GetAnimSet() const { return mAnimSet; }
  EState Update(int fireButtonStates, bool bobbing, bool inStrikeCooldown, float dt,
                CStateManager& mgr);
  void ResetMinor();
  void ResetAll();

  void DoneLoading() { mLoading = false; }
};
CHECK_SIZEOF(CFidget, 0x38)

#endif // _CFIDGET
