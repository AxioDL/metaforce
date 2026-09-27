#include "MetroidPrime/Weapons/GunController/CGSComboFire.hpp"

#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CStateManager.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"

CGSComboFire::CGSComboFire()
: mDelay(0.f)
, mLoopState(-1)
, mCueAnimId(-1)
, mGunId(-1)
, mOver(false)
, mIdle(false) {}

bool CGSComboFire::Update(CAnimData& data, float dt, CStateManager& mgr) {
  if (mCueAnimId != -1) {
    mDelay -= dt;
    if (mDelay <= 0.f) {
      data.EnableLooping(mLoopState == 1);
      CAnimPlaybackParms aparms(mCueAnimId, -1, 1.f, true);
      data.SetAnimation(aparms, false);
      mDelay = 0.f;
      mCueAnimId = -1;
    }
  } else if (!data.IsAnimTimeRemaining(0.001f, rstl::string_l("Whole Body"))) {
    switch (mLoopState) {
    case 0:
      SetAnim(data, mGunId, 1, mgr, 0.f);
      switch (mGunId) {
      case 4:
      case 0:
      case 1:
        mOver = true;
        break;
      default:
        break;
      }
      break;
    case 2:
      mLoopState = -1;
      return true;
    default:
      break;
    }
  }
  return false;
}

int CGSComboFire::SetAnim(CAnimData& data, int gunId, int loopState, CStateManager& mgr,
                          float delay) {
  int useLoopState = !mIdle ? loopState : 2;
  mIdle = false;
  
  const CPASDatabase& pas = data.GetCharacterInfo().GetPASDatabase();

  rstl::pair< float, int > anim =
      pas.FindBestAnimation(CPASAnimParmData(pas::kAS_Death, CPASAnimParm::FromInt32(gunId),
                                             CPASAnimParm::FromEnum(useLoopState)),
                            *mgr.Random(), -1);
  mOver = false;
  mGunId = gunId;
  mLoopState = useLoopState;
  if (delay != 0.f) {
    mDelay = delay;
    mCueAnimId = anim.second;
  } else {
    data.EnableLooping(useLoopState == 1);
    CAnimPlaybackParms aparms(anim.second, -1, 1.f, true);
    data.SetAnimation(aparms, false);
  }
  return anim.second;
}
