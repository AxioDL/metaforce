#include "Runtime/Weapon/CGunMotion.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/Character/CPASAnimParmData.hpp"
#include "Runtime/Weapon/WeaponCommon.hpp"

namespace urde {

CGunMotion::CGunMotion(CAssetId ancsId, const zeus::CVector3f& scale)
: x0_modelData(CAnimRes(ancsId, 0, scale, 0, false), 1), x4c_gunController(x0_modelData) {
  LoadAnimations();
}

void CGunMotion::LoadAnimations() {
  NWeaponTypes::get_token_vector(*x0_modelData.GetAnimationData(), 0, 14, xa8_anims, true);
}

bool CGunMotion::PlayPasAnim(SamusGun::EAnimationState state, CStateManager& mgr, float angle, bool bigStrike) {
  const CPASDatabase& pas = x0_modelData.GetAnimationData()->GetCharacterInfo().GetPASDatabase();

  s32 animId = -1;
  bool loop = true;
  switch (state) {
  case SamusGun::EAnimationState::Wander: {
    CPASAnimParmData parms((s32(state)));
    auto anim = pas.FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
    animId = anim.second;
    break;
  }
  case SamusGun::EAnimationState::Idle: {
    CPASAnimParmData parms(s32(state), CPASAnimParm::FromEnum(0));
    auto anim = pas.FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
    animId = anim.second;
    break;
  }
  case SamusGun::EAnimationState::Struck: {
    CPASAnimParmData parms(s32(state), CPASAnimParm::FromInt32(0), CPASAnimParm::FromReal32(angle),
                           CPASAnimParm::FromBool(bigStrike), CPASAnimParm::FromBool(false));
    auto anim = pas.FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
    animId = anim.second;
    loop = false;
    break;
  }
  case SamusGun::EAnimationState::FreeLook:
    x4c_gunController.EnterFreeLook(mgr, 0, -1);
    break;
  case SamusGun::EAnimationState::ComboFire:
    x4c_gunController.EnterComboFire(mgr, 0);
    break;
  default:
    break;
  }

  if (animId != -1) {
    x0_modelData.GetAnimationData()->EnableLooping(loop);
    CAnimPlaybackParms aparms(animId, -1, 1.f, true);
    x0_modelData.GetAnimationData()->SetAnimation(aparms, false);
  }

  return loop;
}

void CGunMotion::ReturnToDefault(CStateManager& mgr, bool setState) {
  x4c_gunController.ReturnToDefault(mgr, 0.f, setState);
}

void CGunMotion::BasePosition(bool bigStrikeReset) {
  x0_modelData.GetAnimationData()->EnableLooping(false);
  CAnimPlaybackParms aparms(bigStrikeReset ? 6 : 0, -1, 1.f, true);
  x0_modelData.GetAnimationData()->SetAnimation(aparms, false);
}

void CGunMotion::EnterFidget(CStateManager& mgr, SamusGun::EFidgetType type, s32 parm2) {
  xb8_24_animPlaying = true;
  x4c_gunController.EnterFidget(mgr, s32(type), 0, parm2);
}

void CGunMotion::Update(float dt, CStateManager& mgr) {
  x0_modelData.AdvanceAnimation(dt, mgr, kInvalidAreaId, true);
  if (x4c_gunController.Update(dt, mgr))
    xb8_24_animPlaying = false;
}

void CGunMotion::Draw(const CStateManager& mgr, const zeus::CTransform& xf) const {
  constexpr CModelFlags flags(0, 0, 3, zeus::skWhite);
  x0_modelData.Render(mgr, xf, nullptr, flags);
}

} // namespace urde
