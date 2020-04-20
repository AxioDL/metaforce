#include "Runtime/Weapon/CPowerBomb.hpp"

#include "Runtime/CPlayerState.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/World/CDamageInfo.hpp"
#include "Runtime/World/CPlayer.hpp"

#include "DataSpec/DNAMP1/SFX/Weapons.h"

#include "TCastTo.hpp" // Generated file, do not modify include path

#include <zeus/CColor.hpp>

namespace urde {
constexpr zeus::CColor kFadeColor(COLOR(0xffffff7));

CPowerBomb::CPowerBomb(const TToken<CGenDescription>& particle, TUniqueId uid, TAreaId aid, TUniqueId playerId,
                       const zeus::CTransform& xf, const CDamageInfo& dInfo)
: CWeapon(
      uid, aid, true, playerId, EWeaponType::PowerBomb, "PowerBomb", xf,
      CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid, EMaterialTypes::Immovable, EMaterialTypes::Trigger},
                                          {EMaterialTypes::Projectile, EMaterialTypes::PowerBomb}),
      {EMaterialTypes::Projectile, EMaterialTypes::PowerBomb}, dInfo, EProjectileAttrib::PowerBombs,
      CModelData::CModelDataNull())
, x164_radiusIncrement(dInfo.GetRadius() / 2.5f)
, x168_particle(std::make_unique<CElementGen>(particle))
, x16c_radius(dInfo.GetRadius()) {
  x168_particle->SetGlobalTranslation(GetTranslation());
}

void CPowerBomb::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CPowerBomb::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  if (msg == EScriptObjectMessage::Registered) {
    mgr.AddWeaponId(xec_ownerId, xf0_weaponType);
    if (mgr.GetPlayerState()->IsPlayerAlive()) {
      CSfxManager::AddEmitter(SFXsfx0710, GetTranslation(), {}, true, false, 0x7f, -1);
      mgr.InformListeners(GetTranslation(), EListenNoiseType::BombExplode);
    } else {
      auto handle = CSfxManager::AddEmitter(SFXsfx073F, GetTranslation(), {}, true, false, 0x7f, -1);
      mgr.Player()->ApplySubmergedPitchBend(handle);
    }
  } else if (msg == EScriptObjectMessage::Deleted) {
    if (x15c_curTime > 0.7f)
      mgr.GetCameraFilterPass(6).DisableFilter(0.f);

    mgr.RemoveWeaponId(xec_ownerId, xf0_weaponType);
  }
  CActor::AcceptScriptMsg(msg, uid, mgr);
}

void CPowerBomb::Think(float dt, CStateManager& mgr) {
  CWeapon::Think(dt, mgr);
  if (x158_24_canStartFilter) {
    if (x15c_curTime > 1.f && !x158_25_filterEnabled) {
      mgr.GetCameraFilterPass(6).SetFilter(EFilterType::Add, EFilterShape::Fullscreen, 1.5f, kFadeColor, -1);
      x158_25_filterEnabled = true;
    }

    if (x15c_curTime > 2.5f)
      x158_24_canStartFilter = false;
  } else if (x15c_curTime > 3.75 && x158_25_filterEnabled) {
    mgr.GetCameraFilterPass(6).DisableFilter(.5f);
    x158_25_filterEnabled = false;
  }

  if (x15c_curTime > 7.f) {
    if (x168_particle->IsSystemDeletable())
      mgr.FreeScriptObject(GetUniqueId());
  }

  if (x15c_curTime > 30.f) {
    mgr.FreeScriptObject(GetUniqueId());
    return;
  }

  if (x15c_curTime > 1.f && x15c_curTime < 4.f) {
    x110_origDamageInfo.SetRadius(x160_curRadius);
    ApplyDynamicDamage(GetTranslation(), mgr);
    x160_curRadius += x164_radiusIncrement;
  }

  x168_particle->Update(dt);
  x15c_curTime += dt;
}

void CPowerBomb::AddToRenderer(const zeus::CFrustum&, CStateManager&) {
  g_Renderer->AddParticleGen(*x168_particle);
}

void CPowerBomb::ApplyDynamicDamage(const zeus::CVector3f& pos, urde::CStateManager& mgr) {
  mgr.ApplyDamageToWorld(xec_ownerId, *this, pos, x12c_curDamageInfo, xf8_filter);
}

} // namespace urde
