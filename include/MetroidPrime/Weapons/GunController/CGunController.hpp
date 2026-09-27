#ifndef _CGUNCONTROLLER
#define _CGUNCONTROLLER

#include "types.h"

#include "MetroidPrime/Weapons/GunController/CGSComboFire.hpp"
#include "MetroidPrime/Weapons/GunController/CGSFidget.hpp"
#include "MetroidPrime/Weapons/GunController/CGSFreeLook.hpp"

class CModelData;

enum EGunState {
  kGS_Inactive,
  kGS_Default,
  kGS_FreeLook,
  kGS_ComboFire,
  kGS_Idle,
  kGS_Fidget,
  kGS_Strike,
  kGS_BigStrike
};

class CStateManager;

class CGunController {
  CModelData& mModelData;
  CGSFreeLook mFreeLook;
  CGSComboFire mComboFire;
  CGSFidget mFidget;
  char mPad[0x4];
  EGunState mGunState;
  int mCurAnimId;
  bool mAnimDone : 1;
  bool mEnteredComboFire : 1;

public:
  explicit CGunController(CModelData& modelData);
  //~CGunController();

  void EnterFreeLook(CStateManager&, int, int);
  void EnterComboFire(CStateManager&, int);
  void EnterFidget(CStateManager&, int, int, int);
  void EnterStruck(CStateManager&, float, bool, bool);
  void LoadFidgetAnimAsync(CStateManager&, int, int, int);
  int Update(float, CStateManager&);
  void EnterIdle(CStateManager&);
  void ReturnToDefault(CStateManager&, float, bool);
  void Reset();
  void ReturnToBasePosition(CStateManager&);
  bool IsComboOver() const { return mComboFire.IsComboOver(); }
  void UnLoadFidget() { mFidget.UnLoadAnim(); }
  bool IsFidgetLoaded() const { return mFidget.IsAnimLoaded(); }

  int GetCurAnimId() const { return mCurAnimId; }
  int GetFreeLookSetId() const { return mFreeLook.GetSetId(); }
};
CHECK_SIZEOF(CGunController, 0x5C)

#endif // _CGUNCONTROLLER
