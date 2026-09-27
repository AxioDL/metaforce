#include "MetroidPrime/Weapons/GunController/CGunController.hpp"

#include "Kyoto/Animation/CPASAnimParm.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"

CGunController::CGunController(CModelData& modelData)
: mModelData(modelData)
, mGunState(kGS_Inactive)
, mCurAnimId(-1)
, mAnimDone(true)
, mEnteredComboFire(false) {}

void CGunController::EnterFreeLook(CStateManager& mgr, const int gunId, const int setId) {
  if (mGunState != kGS_ComboFire && !mEnteredComboFire) {
    mCurAnimId = mFreeLook.SetAnim(*mModelData.AnimationData(), gunId, setId, 0, mgr, 0.f);
  } else {
    mFreeLook.SetLoopState(mComboFire.GetLoopState());
  }

  mGunState = kGS_FreeLook;
}

void CGunController::EnterComboFire(CStateManager& mgr, const int gunId) {
  if (mGunState != kGS_FreeLook) {
    mCurAnimId = mComboFire.SetAnim(*mModelData.AnimationData(), gunId, 0, mgr, 0.f);
  } else {
    mComboFire.SetLoopState(mFreeLook.GetLoopState());
  }

  mGunState = kGS_ComboFire;
  mEnteredComboFire = true;
}

void CGunController::EnterFidget(CStateManager& mgr, const int type, const int gunId,
                                 const int setId) {
  mCurAnimId = mFidget.SetAnim(*mModelData.AnimationData(), type, gunId, setId, mgr);
  mGunState = kGS_Fidget;
}

void CGunController::EnterStruck(CStateManager& mgr, const float angle, const bool bigStrike,
                                 const bool b2) {
  switch (mGunState) {
  case kGS_FreeLook:
    mFreeLook.SetIdle(true);
    break;
  case kGS_Inactive:
  case kGS_Fidget:
    break;
  default:
    return;
  }
  const CPASAnimParmData aparam =
      CPASAnimParmData(pas::kAS_LieOnGround, CPASAnimParm::FromInt32(mFreeLook.GetGunId()),
                       CPASAnimParm::FromReal32(angle), CPASAnimParm::FromBool(bigStrike),
                       CPASAnimParm::FromBool(b2));
  CAnimData& animData = *mModelData.AnimationData();
  const rstl::pair< float, int > anim =
      animData.GetPASDatabase().FindBestAnimation(aparam, *mgr.Random(), -1);
  animData.EnableLooping(false);
  animData.SetAnimation(CAnimPlaybackParms(anim.second, -1, 1.f, true), false);
  mCurAnimId = anim.second;
  mGunState = bigStrike ? kGS_BigStrike : kGS_Strike;
}

void CGunController::LoadFidgetAnimAsync(CStateManager& mgr, const int type, const int gunId,
                                         const int setId) {
  mFidget.LoadAnimAsync(*mModelData.AnimationData(), type, gunId, setId, mgr);
}

int CGunController::Update(float dt, CStateManager& mgr) {
  CAnimData& animData = *mModelData.AnimationData();
  mAnimDone = false;
  switch (mGunState) {
  case kGS_FreeLook: {
    mAnimDone = mFreeLook.Update(animData, dt, mgr);
    if (!mAnimDone || !mEnteredComboFire) {
      break;
    }

    EnterComboFire(mgr, mFreeLook.GetGunId());
    mAnimDone = false;
    break;
  }
  case kGS_ComboFire:
    mAnimDone = mComboFire.Update(animData, dt, mgr);
    break;
  case kGS_Fidget:
    mAnimDone = mFidget.Update(animData, dt, mgr);
    break;
  case kGS_Strike: {
    if (animData.IsAnimTimeRemaining(0.001f, rstl::string_l("Whole Body"))) {
      break;
    }

    mCurAnimId = mFreeLook.SetAnim(*mModelData.AnimationData(), mFreeLook.GetGunId(),
                                        mFreeLook.GetSetId(), 0, mgr, 0.f);
    mGunState = kGS_FreeLook;
    break;
  }
  case kGS_BigStrike:
    mAnimDone = !animData.IsAnimTimeRemaining(0.001f, rstl::string_l("Whole Body"));
    break;
  case kGS_Inactive:
  case kGS_Default:
  case kGS_Idle:
    break;
  }

  if (mAnimDone) {
    mGunState = kGS_Inactive;
    mEnteredComboFire = false;
    return true;
  }
  return false;
}

void CGunController::EnterIdle(CStateManager& mgr) {
  CPASAnimParm parm = CPASAnimParm::NoParameter();
  switch (mGunState) {
  case kGS_FreeLook:
    parm = CPASAnimParm::FromEnum(1);
    mFreeLook.SetIdle(true);
    break;
  case kGS_ComboFire:
    parm = CPASAnimParm::FromEnum(1);
    mComboFire.SetIdle(true);
    break;
  default:
    return;
  }

  CAnimData& animData = *mModelData.AnimationData();
  const rstl::pair< float, int > anim =
      animData.GetCharacterInfo().GetPASDatabase().FindBestAnimation(
          CPASAnimParmData(pas::kAS_Locomotion, parm), *mgr.Random(), -1);
  animData.EnableLooping(false);
  animData.SetAnimation(CAnimPlaybackParms(anim.second, -1, 1.f, true), false);
  mCurAnimId = anim.second;
  mGunState = kGS_Idle;
  mEnteredComboFire = false;
}

void CGunController::ReturnToDefault(CStateManager& mgr, const float dt, const bool setState) {
  CAnimData& animData = *mModelData.AnimationData();

  switch (mGunState) {
  case kGS_Strike:
    mGunState = kGS_FreeLook;
  case kGS_Idle:
    mFreeLook.SetIdle(false);
  case kGS_FreeLook:
    if (setState) {
      break;
    }
    mCurAnimId =
        mFreeLook.SetAnim(animData, mFreeLook.GetGunId(), mFreeLook.GetSetId(), 2, mgr, dt);
    mEnteredComboFire = false;
    break;
  case kGS_ComboFire:
    mCurAnimId = mComboFire.SetAnim(animData, mComboFire.GetGunId(), 2, mgr, dt);
    break;
  case kGS_Fidget:
    ReturnToBasePosition(mgr);
    break;
  case kGS_BigStrike:
    mFreeLook.SetIdle(false);
    break;
  default:
    break;
  }

  if (setState) {
    mGunState = kGS_Default;
  }
}

void CGunController::Reset() {
  mAnimDone = true;
  mEnteredComboFire = false;
  mGunState = kGS_Inactive;
}

void CGunController::ReturnToBasePosition(CStateManager& mgr) {
  CAnimData& animData = *mModelData.AnimationData();
  const rstl::pair< float, int > anim =
      animData.GetCharacterInfo().GetPASDatabase().FindBestAnimation(
          CPASAnimParmData(pas::kAS_KnockBack), *mgr.Random(), -1);
  animData.EnableLooping(false);
  animData.SetAnimation(CAnimPlaybackParms(anim.second, -1, 1.f, true), false);
  mCurAnimId = anim.second;
  mEnteredComboFire = false;
}
