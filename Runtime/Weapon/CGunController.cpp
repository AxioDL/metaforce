#include "Runtime/Weapon/CGunController.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/Character/CCharLayoutInfo.hpp"
#include "Runtime/Character/CPASAnimParmData.hpp"

namespace urde {

void CGunController::LoadFidgetAnimAsync(CStateManager& mgr, s32 type, s32 gunId, s32 animSet) {
  x30_fidget.LoadAnimAsync(*x0_modelData.GetAnimationData(), type, gunId, animSet, mgr);
}

void CGunController::EnterFidget(CStateManager& mgr, s32 type, s32 gunId, s32 animSet) {
  x54_curAnimId = x30_fidget.SetAnim(*x0_modelData.GetAnimationData(), type, gunId, animSet, mgr);
  x50_gunState = EGunState::Fidget;
}

void CGunController::EnterFreeLook(CStateManager& mgr, s32 gunId, s32 setId) {
  if (x50_gunState != EGunState::ComboFire && !x58_25_enteredComboFire)
    x54_curAnimId = x4_freeLook.SetAnim(*x0_modelData.GetAnimationData(), gunId, setId, 0, mgr, 0.f);
  else
    x4_freeLook.SetLoopState(x1c_comboFire.GetLoopState());
  x50_gunState = EGunState::FreeLook;
}

void CGunController::EnterComboFire(CStateManager& mgr, s32 gunId) {
  if (x50_gunState != EGunState::FreeLook)
    x54_curAnimId = x1c_comboFire.SetAnim(*x0_modelData.GetAnimationData(), gunId, 0, mgr, 0.f);
  else
    x1c_comboFire.SetLoopState(x4_freeLook.GetLoopState());
  x50_gunState = EGunState::ComboFire;
  x58_25_enteredComboFire = true;
}

void CGunController::EnterStruck(CStateManager& mgr, float angle, bool bigStrike, bool b2) {
  switch (x50_gunState) {
  case EGunState::Default:
  case EGunState::ComboFire:
  case EGunState::Idle:
  case EGunState::Strike:
  case EGunState::BigStrike:
    return;
  case EGunState::FreeLook:
    x4_freeLook.SetIdle(true);
    break;
  default:
    break;
  }

  const CPASDatabase& pasDatabase = x0_modelData.GetAnimationData()->GetCharacterInfo().GetPASDatabase();
  CPASAnimParmData parms(2, CPASAnimParm::FromInt32(x4_freeLook.GetGunId()), CPASAnimParm::FromReal32(angle),
                         CPASAnimParm::FromBool(bigStrike), CPASAnimParm::FromBool(b2));
  std::pair<float, s32> anim = pasDatabase.FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
  x0_modelData.GetAnimationData()->EnableLooping(false);
  CAnimPlaybackParms aparms(anim.second, -1, 1.f, true);
  x0_modelData.GetAnimationData()->SetAnimation(aparms, false);
  x54_curAnimId = anim.second;
  x58_25_enteredComboFire = false;
  x50_gunState = bigStrike ? EGunState::BigStrike : EGunState::Strike;
}

void CGunController::EnterIdle(CStateManager& mgr) {
  CPASAnimParm parm = CPASAnimParm::NoParameter();
  switch (x50_gunState) {
  case EGunState::FreeLook:
    parm = CPASAnimParm::FromEnum(1);
    x4_freeLook.SetIdle(true);
    break;
  case EGunState::ComboFire:
    parm = CPASAnimParm::FromEnum(1);
    x1c_comboFire.SetIdle(true);
    break;
  default:
    return;
  }

  const CPASDatabase& pasDatabase = x0_modelData.GetAnimationData()->GetCharacterInfo().GetPASDatabase();
  CPASAnimParmData parms(5, parm);
  std::pair<float, s32> anim = pasDatabase.FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
  x0_modelData.GetAnimationData()->EnableLooping(false);
  CAnimPlaybackParms aparms(anim.second, -1, 1.f, true);
  x0_modelData.GetAnimationData()->SetAnimation(aparms, false);
  x54_curAnimId = anim.second;
  x50_gunState = EGunState::Idle;
  x58_25_enteredComboFire = false;
}

bool CGunController::Update(float dt, CStateManager& mgr) {
  CAnimData& animData = *x0_modelData.GetAnimationData();
  switch (x50_gunState) {
  case EGunState::FreeLook: {
    x58_24_animDone = x4_freeLook.Update(animData, dt, mgr);
    if (!x58_24_animDone || !x58_25_enteredComboFire)
      break;

    EnterComboFire(mgr, x4_freeLook.GetGunId());
    x58_24_animDone = false;
    break;
  }
  case EGunState::ComboFire:
    x58_24_animDone = x1c_comboFire.Update(animData, dt, mgr);
    break;
  case EGunState::Fidget:
    x58_24_animDone = x30_fidget.Update(animData, dt, mgr);
    break;
  case EGunState::Strike: {
    if (animData.IsAnimTimeRemaining(0.001f, "Whole Body"))
      break;
    x54_curAnimId = x4_freeLook.SetAnim(animData, x4_freeLook.GetGunId(), x4_freeLook.GetSetId(), 0, mgr, 0.f);
    x50_gunState = EGunState::FreeLook;
    break;
  }
  case EGunState::BigStrike:
    x58_24_animDone = !animData.IsAnimTimeRemaining(0.001f, "Whole Body");
    break;
  default:
    break;
  }

  if (!x58_24_animDone)
    return false;

  x50_gunState = EGunState::Inactive;
  x58_25_enteredComboFire = false;

  return true;
}

void CGunController::ReturnToDefault(CStateManager& mgr, float dt, bool setState) {
  CAnimData& animData = *x0_modelData.GetAnimationData();

  switch (x50_gunState) {
  case EGunState::Strike:
    x50_gunState = EGunState::FreeLook;
    [[fallthrough]];
  case EGunState::Idle:
    x4_freeLook.SetIdle(false);
    [[fallthrough]];
  case EGunState::FreeLook:
    if (setState)
      break;
    x54_curAnimId = x4_freeLook.SetAnim(animData, x4_freeLook.GetGunId(), x4_freeLook.GetSetId(), 2, mgr, dt);
    x58_25_enteredComboFire = false;
    break;
  case EGunState::ComboFire:
    x54_curAnimId = x1c_comboFire.SetAnim(animData, x1c_comboFire.GetGunId(), 2, mgr, dt);
    break;
  case EGunState::Fidget:
    ReturnToBasePosition(mgr, dt);
    break;
  case EGunState::BigStrike:
    x4_freeLook.SetIdle(false);
    break;
  default:
    break;
  }

  if (setState)
    x50_gunState = EGunState::Default;
}

void CGunController::ReturnToBasePosition(CStateManager& mgr, float) {
  const CPASDatabase& pasDatabase = x0_modelData.GetAnimationData()->GetCharacterInfo().GetPASDatabase();
  std::pair<float, s32> anim = pasDatabase.FindBestAnimation(CPASAnimParmData(6), *mgr.GetActiveRandom(), -1);
  x0_modelData.GetAnimationData()->EnableLooping(false);
  CAnimPlaybackParms parms(anim.second, -1, 1.f, true);
  x0_modelData.GetAnimationData()->SetAnimation(parms, false);
  x54_curAnimId = anim.second;
  x58_25_enteredComboFire = false;
}

void CGunController::Reset() {
  x58_24_animDone = true;
  x58_25_enteredComboFire = false;
  x50_gunState = EGunState::Inactive;
}
} // namespace urde
