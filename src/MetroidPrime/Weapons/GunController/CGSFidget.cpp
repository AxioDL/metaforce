#include "MetroidPrime/Weapons/GunController/CGSFidget.hpp"

#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Weapons/WeaponCommon.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/CToken.hpp"

CGSFidget::CGSFidget() : x10_(-1), mGunId(-1), mAnimSet(-1) {}

bool CGSFidget::Update(CAnimData& data, float dt, CStateManager& mgr) {
  return !data.IsAnimTimeRemaining(0.001f, rstl::string_l("Whole Body"));
}

int CGSFidget::SetAnim(CAnimData& data, int type, int gunId, int animSet, CStateManager& mgr) {
  const CPASDatabase& pas = data.GetCharacterInfo().GetPASDatabase();
  rstl::pair< float, int > anim = pas.FindBestAnimation(
      CPASAnimParmData(pas::kAS_Getup, CPASAnimParm::FromEnum(type), CPASAnimParm::FromInt32(gunId),
                       CPASAnimParm::FromInt32(animSet)),
      *mgr.Random(), -1);
    
  CPASAnimParm parmData(pas.GetAnimState(pas::kAS_Getup)->GetAnimParmData(anim.second, 3));
  bool loop = parmData.GetBoolValue();
  mGunId = gunId;
  mAnimSet = animSet;
  if (anim.second != -1) {
    data.EnableLooping(loop);
    CAnimPlaybackParms aParms(anim.second, -1, 1.f, true);
    data.SetAnimation(aParms, false);
    UnLoadAnim();
  }
  return anim.second;
}

void CGSFidget::LoadAnimAsync(CAnimData& data, int type, int gunId, int animSet,
                              CStateManager& mgr) {

  rstl::pair< float, int > anim = data.GetCharacterInfo().GetPASDatabase().FindBestAnimation(
      CPASAnimParmData(pas::kAS_Getup, CPASAnimParm::FromEnum(type), CPASAnimParm::FromInt32(gunId),
                       CPASAnimParm::FromInt32(animSet)),
      *mgr.Random(), -1);
  if (anim.second != -1)
    NWeaponTypes::get_token_vector(data, anim.second, mAnims, true);
}

void CGSFidget::UnLoadAnim() {
  if (!mAnims.empty()) {
    mAnims = rstl::vector< CToken >();
  }
}

bool CGSFidget::IsAnimLoaded() const { return NWeaponTypes::are_tokens_ready(mAnims); }
