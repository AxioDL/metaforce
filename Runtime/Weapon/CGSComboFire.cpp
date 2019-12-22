#include "Runtime/Weapon/CGSComboFire.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/Character/CAnimData.hpp"
#include "Runtime/Character/CPASAnimParmData.hpp"

namespace urde {

bool CGSComboFire::Update(CAnimData& data, float dt, CStateManager& mgr) {
  if (x8_cueAnimId != -1) {
    x0_delay -= dt;
    if (x0_delay <= 0.f) {
      data.EnableLooping(x4_loopState == 1);
      CAnimPlaybackParms aparms(x8_cueAnimId, -1, 1.f, true);
      data.SetAnimation(aparms, false);
      x0_delay = 0.f;
      x8_cueAnimId = -1;
    }
  } else if (!data.IsAnimTimeRemaining(0.001f, "Whole Body")) {
    switch (x4_loopState) {
    case 0:
      SetAnim(data, xc_gunId, 1, mgr, 0.f);
      switch (xc_gunId) {
      case 4:
      case 0:
      case 1:
        x10_24_over = true;
        break;
      default:
        break;
      }
      break;
    case 2:
      x4_loopState = -1;
      return true;
    default:
      break;
    }
  }
  return false;
}

s32 CGSComboFire::SetAnim(CAnimData& data, s32 gunId, s32 loopState, CStateManager& mgr, float delay) {
  s32 useLoopState = 2;
  if (!x10_25_idle)
    useLoopState = loopState;
  x10_25_idle = false;
  const CPASDatabase& pas = data.GetCharacterInfo().GetPASDatabase();
  CPASAnimParmData parms(4, CPASAnimParm::FromInt32(gunId), CPASAnimParm::FromEnum(useLoopState));
  auto anim = pas.FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
  x10_24_over = false;
  xc_gunId = gunId;
  x4_loopState = useLoopState;
  if (delay != 0.f) {
    x0_delay = delay;
    x8_cueAnimId = anim.second;
  } else {
    data.EnableLooping(loopState == 1);
    CAnimPlaybackParms aparms(anim.second, -1, 1.f, true);
    data.SetAnimation(aparms, false);
  }
  return anim.second;
}

} // namespace urde
