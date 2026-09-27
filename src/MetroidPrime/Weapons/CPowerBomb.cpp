#include "MetroidPrime/Weapons/CPowerBomb.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/SFX/Weapons.h"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetaRender/CCubeRenderer.hpp"

CColor CPowerBomb::kFadeColor(0xffffff7f);
const float CPowerBomb::kEndingTime = 4.25f;

CPowerBomb::CPowerBomb(TToken< CGenDescription > particle, TUniqueId uid, TAreaId aid,
                       TUniqueId playerId, const CTransform4f& xf, const CDamageInfo& dInfo)
: CWeapon(uid, aid, true, playerId, kWT_PowerBomb, rstl::string_l("PowerBomb"), xf,
          CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Trigger, kMT_Immovable, kMT_Solid),
                                              CMaterialList(kMT_Projectile, kMT_PowerBomb)),
          CMaterialList(kMT_Projectile, kMT_PowerBomb), dInfo, CWeapon::kPA_PowerBombs,
          CModelData::CModelDataNull())

, mCanStartFilter(true)
, mFilterEnabled(false)
, mCurTime(0.f)
, mCurRadius(0.f)
, mRadiusIncrement(dInfo.GetRadius() / 2.5f)
, mParticle(rs_new CElementGen(particle))
, mRadius(dInfo.GetRadius()) {
  mParticle->SetGlobalTranslation(xf.GetTranslation());
}

CPowerBomb::~CPowerBomb() {}

void CPowerBomb::ApplyDynamicDamage(const CVector3f& pos, CStateManager& mgr) {
  mgr.ApplyDamageToWorld(GetOwnerId(), *this, pos, mCurDamageInfo, CMaterialFilter(mFilter));
}

void CPowerBomb::Touch(CActor&, CStateManager&) {
  if (mCanStartFilter) {
    return;
  }
}

rstl::optional_object< CAABox > CPowerBomb::GetTouchBounds() const {
  return rstl::optional_object_null();
}

void CPowerBomb::AddToRenderer(const CFrustumPlanes&, const CStateManager&) const {
  gpRender->AddParticleGen(*mParticle);
}

void CPowerBomb::Render(const CStateManager&) const {}

void CPowerBomb::Think(float dt, CStateManager& mgr) {
  CWeapon::Think(dt, mgr);

  if (mCanStartFilter) {
    if (mCurTime > 1.f && mFilterEnabled != true) {
      mgr.CameraFilterPass(CStateManager::kCFS_Six)
          .SetFilter(CCameraFilterPass::kFT_Add, CCameraFilterPass::kFS_Fullscreen, 1.5f,
                     kFadeColor, kInvalidAssetId);
      mFilterEnabled = true;
    }

    if (mCurTime > 2.5f)
      mCanStartFilter = false;
  } else {
    if (mCurTime > 3.75f && mFilterEnabled) {
      mgr.CameraFilterPass(CStateManager::kCFS_Six).DisableFilter(.5f);
      mFilterEnabled = false;
    }

    if (mCurTime > 7.f) {
      if (mParticle->IsSystemDeletable())
        mgr.DeleteObjectRequest(GetUniqueId());
    }

    if (mCurTime > 30.f) {
      mgr.DeleteObjectRequest(GetUniqueId());
      return;
    }
  }

  if (mCurTime > 1.f && mCurTime < 4.f) {
    mOrigDamageInfo.SetRadius(mCurRadius);
    ApplyDynamicDamage(GetTranslation(), mgr);
    mCurRadius += mRadiusIncrement * dt;
  }

  mParticle->Update(dt);
  mCurTime += dt;
}

ENTITY_ACCEPT_IMPL(CPowerBomb)

void CPowerBomb::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case kSM_Registered:
    mgr.AddWeaponId(GetOwnerId(), GetType());
    OrigDamageInfo().SetRadius(0.f);
    if (mgr.GetPlayerState()->IsAlive()) {
      CSfxManager::AddEmitter(SFXsam_a_powexpl_00, GetTranslation(), CVector3f::Zero(), true,
                              false);
      mgr.InformListeners(GetTranslation(), kLNT_BombExplode);
    } else {
      mgr.Player()->DoSfxEffects(
          CSfxManager::SfxStart(SFXsam_r_diemorph_00, 127, 64, false, CSfxManager::kMaxPriority));
    }
    break;

  case kSM_Deleted:
    if (mCurTime <= 7.0f) {
      mgr.CameraFilterPass(CStateManager::kCFS_Six).DisableFilter(0.f);
    }
    mgr.RemoveWeaponId(GetOwnerId(), GetType());
    break;

  default:
    break;
  }
  CActor::AcceptScriptMsg(msg, uid, mgr);
}
