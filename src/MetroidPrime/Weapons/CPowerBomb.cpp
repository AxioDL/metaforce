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

, x158_24_canStartFilter(true)
, x158_25_filterEnabled(false)
, x15c_curTime(0.f)
, x160_curRadius(0.f)
, x164_radiusIncrement(dInfo.GetRadius() / 2.5f)
, x168_particle(rs_new CElementGen(particle))
, x16c_radius(dInfo.GetRadius()) {
  x168_particle->SetGlobalTranslation(xf.GetTranslation());
}

CPowerBomb::~CPowerBomb() {}

void CPowerBomb::ApplyDynamicDamage(const CVector3f& pos, CStateManager& mgr) {
  mgr.ApplyDamageToWorld(GetOwnerId(), *this, pos, x12c_curDamageInfo, CMaterialFilter(xf8_filter));
}

void CPowerBomb::Touch(CActor&, CStateManager&) {
  if (x158_24_canStartFilter) {
    return;
  }
}

rstl::optional_object< CAABox > CPowerBomb::GetTouchBounds() const {
  return rstl::optional_object_null();
}

void CPowerBomb::AddToRenderer(const CFrustumPlanes&, const CStateManager&) const {
  gpRender->AddParticleGen(*x168_particle);
}

void CPowerBomb::Render(const CStateManager&) const {}

void CPowerBomb::Think(float dt, CStateManager& mgr) {
  CWeapon::Think(dt, mgr);

  if (x158_24_canStartFilter) {
    if (x15c_curTime > 1.f && x158_25_filterEnabled != true) {
      mgr.CameraFilterPass(CStateManager::kCFS_Six)
          .SetFilter(CCameraFilterPass::kFT_Add, CCameraFilterPass::kFS_Fullscreen, 1.5f,
                     kFadeColor, kInvalidAssetId);
      x158_25_filterEnabled = true;
    }

    if (x15c_curTime > 2.5f)
      x158_24_canStartFilter = false;
  } else {
    if (x15c_curTime > 3.75f && x158_25_filterEnabled) {
      mgr.CameraFilterPass(CStateManager::kCFS_Six).DisableFilter(.5f);
      x158_25_filterEnabled = false;
    }

    if (x15c_curTime > 7.f) {
      if (x168_particle->IsSystemDeletable())
        mgr.DeleteObjectRequest(GetUniqueId());
    }

    if (x15c_curTime > 30.f) {
      mgr.DeleteObjectRequest(GetUniqueId());
      return;
    }
  }

  if (x15c_curTime > 1.f && x15c_curTime < 4.f) {
    x110_origDamageInfo.SetRadius(x160_curRadius);
    ApplyDynamicDamage(GetTranslation(), mgr);
    x160_curRadius += x164_radiusIncrement * dt;
  }

  x168_particle->Update(dt);
  x15c_curTime += dt;
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
    if (x15c_curTime <= 7.0f) {
      mgr.CameraFilterPass(CStateManager::kCFS_Six).DisableFilter(0.f);
    }
    mgr.RemoveWeaponId(GetOwnerId(), GetType());
    break;

  default:
    break;
  }
  CActor::AcceptScriptMsg(msg, uid, mgr);
}
