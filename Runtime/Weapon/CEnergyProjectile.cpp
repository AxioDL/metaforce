#include "GameGlobalObjects.hpp"
#include "CEnergyProjectile.hpp"
#include "CStateManager.hpp"
#include "World/CWorld.hpp"
#include "World/CGameLight.hpp"
#include "World/CPlayer.hpp"
#include "Particle/CDecalManager.hpp"
#include "Camera/CGameCamera.hpp"
#include "World/CExplosion.hpp"
#include "World/CScriptPlatform.hpp"
#include "World/CIceImpact.hpp"
#include "CSimplePool.hpp"
#include "TCastTo.hpp"

namespace urde {

CEnergyProjectile::CEnergyProjectile(bool active, const TToken<CWeaponDescription>& desc, EWeaponType type,
                                     const zeus::CTransform& xf, EMaterialTypes excludeMat, const CDamageInfo& damage,
                                     TUniqueId uid, TAreaId aid, TUniqueId owner, TUniqueId homingTarget,
                                     EProjectileAttrib attribs, bool underwater, const zeus::CVector3f& scale,
                                     const std::experimental::optional<TLockedToken<CGenDescription>>& visorParticle,
                                     u16 visorSfx, bool sendCollideMsg)
: CGameProjectile(active, desc, "GameProjectile", type, xf, excludeMat, damage, uid, aid, owner, homingTarget, attribs,
                  underwater, scale, visorParticle, visorSfx, sendCollideMsg)
, x2ec_dir(xf.basis[1])
, x2f8_mag(x2ec_dir.magnitude())
, x2fc_camShake(CCameraShakeData::BuildProjectileCameraShake(0.5f, 0.75f)) {
  xe6_27_thermalVisorFlags = 2;
}

void CEnergyProjectile::PlayImpactSound(const zeus::CVector3f& pos, EWeaponCollisionResponseTypes type) {
  s32 sfxId = x170_projectile.GetSoundIdForCollision(type);
  if (sfxId >= 0) {
    CAudioSys::C3DEmitterParmData parmData = {};
    parmData.x18_maxDist = x170_projectile.GetAudibleRange();
    parmData.x1c_distComp = x170_projectile.GetAudibleFallOff();
    parmData.x20_flags = 0x1; // Continuous parameter update
    parmData.x24_sfxId = CSfxManager::TranslateSFXID(u16(sfxId));
    parmData.x26_maxVol = 1.f;
    parmData.x27_minVol = 0.16f;
    parmData.x29_prio = 0x7f;
    CSfxHandle hnd = CSfxManager::AddEmitter(parmData, true, 0x7f, false, kInvalidAreaId);
    if (x2e4_26_waterUpdate)
      CSfxManager::PitchBend(hnd, -1.f);
  }
}

void CEnergyProjectile::ChangeProjectileOwner(TUniqueId owner, CStateManager& mgr) {
  if (TCastToConstPtr<CActor> act = mgr.GetObjectById(owner)) {
    float rDam = g_tweakPlayerGun->GetRichochetDamage(u32(x110_origDamageInfo.GetWeaponMode().GetType()));
    x110_origDamageInfo.MultiplyDamageAndRadius(rDam);
    mgr.RemoveWeaponId(xec_ownerId, xf0_weaponType);
    xec_ownerId = owner;
    mgr.AddWeaponId(xec_ownerId, xf0_weaponType);

    /* Can now damage Player */
    xf8_filter.ExcludeList().Add(EMaterialTypes::Character);
    xf8_filter.ExcludeList().Remove(EMaterialTypes::Player);
    xf8_filter = CMaterialFilter::MakeIncludeExclude(xf8_filter.GetIncludeList(), xf8_filter.GetExcludeList());
  }
}

void CEnergyProjectile::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  switch (msg) {
  case EScriptObjectMessage::Deleted:
    if (x2e4_24_active)
      mgr.RemoveWeaponId(xec_ownerId, xf0_weaponType);
    if (x2e8_sfx) {
      CSfxManager::RemoveEmitter(x2e8_sfx);
      x2e8_sfx.reset();
    }
    break;
  case EScriptObjectMessage::Registered: {
    if (CElementGen* ps1 = x170_projectile.GetAttachedPS1())
      if (ps1->SystemHasLight())
        CreateProjectileLight("ProjectileLight_GameProjectile", ps1->GetLight(), mgr);
    TLockedToken<CWeaponDescription> desc = x170_projectile.GetWeaponDescription();
    s32 sfx = desc->xa8_PJFX;
    if (sfx != -1) {
      float range = 50.f;
      float falloff = 0.2f;
      if (CRealElement* rnge = desc->xac_RNGE.get())
        rnge->GetValue(0, range);
      if (CRealElement* foff = desc->xb0_FOFF.get())
        foff->GetValue(0, falloff);

      CAudioSys::C3DEmitterParmData parmData = {};
      parmData.x0_pos = x170_projectile.GetTranslation();
      parmData.xc_dir = x170_projectile.GetVelocity();
      parmData.x18_maxDist = range;
      parmData.x1c_distComp = falloff;
      parmData.x20_flags = 0x9; // Continuous parameter update, doppler
      parmData.x24_sfxId = CSfxManager::TranslateSFXID(sfx);
      parmData.x26_maxVol = 1.f;
      parmData.x27_minVol = 0.16f;
      parmData.x29_prio = 0x7f;
      x2e8_sfx = CSfxManager::AddEmitter(parmData, true, 0x7f, false, kInvalidAreaId);
    }
    mgr.AddWeaponId(xec_ownerId, xf0_weaponType);
    break;
  }
  default:
    break;
  }
  CGameProjectile::AcceptScriptMsg(msg, sender, mgr);
}

void CEnergyProjectile::Accept(IVisitor& visitor) { visitor.Visit(this); }

/* Material surface types only (not meta flags) */
static constexpr u64 kCheckMaterial = 0xE3FFFE;

void CEnergyProjectile::ResolveCollisionWithWorld(const CRayCastResult& res, CStateManager& mgr) {
  EWeaponCollisionResponseTypes crType = CCollisionResponseData::GetWorldCollisionResponseType(
      CMaterialList::BitPosition((res.GetMaterial().GetValue() & 0xffffffff) & kCheckMaterial));
  if ((xe8_projectileAttribs & (EProjectileAttrib::Wave | EProjectileAttrib::ComboShot)) !=
      (EProjectileAttrib::Wave | EProjectileAttrib::ComboShot)) {
    /* Not wavebuster */
    if (Explode(res.GetPoint(), res.GetPlane().normal(), crType, mgr, CDamageVulnerability::NormalVulnerabilty(),
                kInvalidUniqueId))
      mgr.ApplyDamageToWorld(xec_ownerId, *this, res.GetPoint(), x12c_curDamageInfo, xf8_filter);
    x2c2_lastResolvedObj = kInvalidUniqueId;
  }
}

void CEnergyProjectile::ResolveCollisionWithActor(const CRayCastResult& res, CActor& act, CStateManager& mgr) {
  x2c2_lastResolvedObj = act.GetUniqueId();
  EWeaponCollisionResponseTypes crType = act.GetCollisionResponseType(
      res.GetPoint(), x34_transform.basis[1].normalized(), x12c_curDamageInfo.GetWeaponMode(), xe8_projectileAttribs);
  act.Touch(*this, mgr);
  const CDamageVulnerability* dVuln = act.GetDamageVulnerability();
  if (!Explode(res.GetPoint(), res.GetPlane().normal(), crType, mgr, *dVuln, act.GetUniqueId())) {
    mgr.SendScriptMsg(&act, GetUniqueId(), EScriptObjectMessage::Touched);
    act.SendScriptMsgs(EScriptObjectState::ReflectedDamage, mgr, EScriptObjectMessage::None);
  } else {
    CGameProjectile::ResolveCollisionWithActor(res, act, mgr);
    ApplyDamageToActors(mgr, x12c_curDamageInfo);
  }
  if (TCastToPtr<CEnergyProjectile> proj = act) {
    proj->Explode(GetTranslation(), x34_transform.basis[1], EWeaponCollisionResponseTypes::OtherProjectile, mgr,
                  *GetDamageVulnerability(), GetUniqueId());
  }
}

void CEnergyProjectile::Think(float dt, CStateManager& mgr) {
  CWeapon::Think(dt, mgr);
  if (mgr.GetWorld()->GetCurrentAreaId() != GetAreaIdAlways() &&
      (xe8_projectileAttribs & EProjectileAttrib::ArmCannon) == EProjectileAttrib::ArmCannon)
    mgr.SetActorAreaId(*this, mgr.GetWorld()->GetCurrentAreaId());

  UpdateProjectileMovement(dt, mgr);
  TUniqueId id = kInvalidUniqueId;
  CRayCastResult res = DoCollisionCheck(id, mgr);
  if (res.IsValid()) {
    if (TCastToPtr<CActor> act = mgr.ObjectById(id))
      ResolveCollisionWithActor(res, *act, mgr);
    else
      ResolveCollisionWithWorld(res, mgr);
  }

  x170_projectile.UpdateParticleFX();
  if (x2e4_24_active && x3d0_26_)
    Explode(GetTranslation(), zeus::CVector3f::skUp, EWeaponCollisionResponseTypes::Default, mgr,
            CDamageVulnerability::NormalVulnerabilty(), kInvalidUniqueId);

  if (x2c8_projectileLight != kInvalidUniqueId) {
    if (TCastToPtr<CGameLight> light = mgr.ObjectById(x2c8_projectileLight)) {
      light->SetTransform(GetTransform());
      light->SetTranslation(GetTranslation());
      if (CElementGen* ps1 = x170_projectile.GetAttachedPS1())
        if (ps1->SystemHasLight())
          light->SetLight(ps1->GetLight());
    }
  }

  if (x2e8_sfx) {
    CSfxManager::UpdateEmitter(x2e8_sfx, x170_projectile.GetTranslation(), x170_projectile.GetVelocity(), 1.f);
    CSfxManager::PitchBend(x2e8_sfx, x2e4_26_waterUpdate ? -1.f : 0.f);
  }

  x3d4_curTime += dt;
  if (x3d4_curTime > 45.f || x170_projectile.IsSystemDeletable() || x3d0_24_dead)
    mgr.FreeScriptObject(GetUniqueId());
}

void CEnergyProjectile::Render(const CStateManager& mgr) const {
  CPlayerState::EPlayerVisor visor = mgr.GetPlayerState()->GetActiveVisor(mgr);
  if (visor == CPlayerState::EPlayerVisor::Combat) {
    if ((xe8_projectileAttribs & EProjectileAttrib::Charged) == EProjectileAttrib::Charged ||
        (xe8_projectileAttribs & EProjectileAttrib::ComboShot) == EProjectileAttrib::ComboShot) {
      float warpTime = 1.f - float(x170_projectile.GameTime());
      if (warpTime > 0.f)
        mgr.DrawSpaceWarp(GetTranslation(), warpTime * 0.75f);
    }
  }

  if (visor == CPlayerState::EPlayerVisor::XRay) {
    CElementGen::SetSubtractBlend((xe8_projectileAttribs & EProjectileAttrib::Ice) != EProjectileAttrib::Ice);
    CGraphics::SetFog(ERglFogMode::PerspLin, 0.f, 75.f, zeus::CColor::skBlack);
    x170_projectile.RenderParticles();
    CGameProjectile::Render(mgr);
    mgr.SetupFogForArea(GetAreaIdAlways());
    CElementGen::SetSubtractBlend(false);
  } else if ((xe8_projectileAttribs & EProjectileAttrib::Ice) == EProjectileAttrib::Ice &&
             mgr.GetThermalDrawFlag() == EThermalDrawFlag::Hot) {
    CElementGen::SetSubtractBlend(true);
    x170_projectile.RenderParticles();
    CGameProjectile::Render(mgr);
    mgr.SetupFogForArea(GetAreaIdAlways());
    CElementGen::SetSubtractBlend(false);
  } else {
    CGameProjectile::Render(mgr);
  }
}

void CEnergyProjectile::AddToRenderer(const zeus::CFrustum& frustum, const CStateManager& mgr) const {
  auto bounds = x170_projectile.GetBounds();
  if (bounds && !frustum.aabbFrustumTest(*bounds))
    return;

  CPlayerState::EPlayerVisor visor = mgr.GetPlayerState()->GetActiveVisor(mgr);
  if (visor != CPlayerState::EPlayerVisor::XRay &&
      ((xe8_projectileAttribs & EProjectileAttrib::Ice) != EProjectileAttrib::Ice ||
       mgr.GetThermalDrawFlag() != EThermalDrawFlag::Hot))
    x170_projectile.AddToRenderer();
  EnsureRendered(mgr);
}

void CEnergyProjectile::Touch(CActor& act, CStateManager& mgr) {
  // Empty
}

bool CEnergyProjectile::Explode(const zeus::CVector3f& pos, const zeus::CVector3f& normal,
                                EWeaponCollisionResponseTypes type, CStateManager& mgr,
                                const CDamageVulnerability& dVuln, TUniqueId hitActor) {
  zeus::CVector3f offsetPos = pos + normal * 0.01f;
  bool done = true;
  bool retargetPlayer = false;
  bool deflect = false;
  zeus::CVector3f targetPos;
  EVulnerability vulnType = dVuln.GetVulnerability(x12c_curDamageInfo.GetWeaponMode(), false);
  if (vulnType == EVulnerability::Deflect) {
    deflect = true;
    EVulnerability deflectType = dVuln.GetDeflectionType(x12c_curDamageInfo.GetWeaponMode());
    switch (deflectType) {
    case EVulnerability::Weak:
      deflect = false;
      break;
    case EVulnerability::Deflect:
    case EVulnerability::Immune:
      if (deflectType != EVulnerability::Deflect ||
          (xf0_weaponType != EWeaponType::Missile &&
           (xe8_projectileAttribs & EProjectileAttrib::ComboShot) != EProjectileAttrib::ComboShot))
        if (xf8_filter.GetExcludeList().HasMaterial(EMaterialTypes::Player))
          retargetPlayer = true;
      break;
    default:
      break;
    }
    if (retargetPlayer) {
      float ang = mgr.GetActiveRandom()->Range(0.f, 2.f * M_PIF);
      float y = std::sin(ang);
      float x = std::cos(ang);
      targetPos = mgr.GetPlayer().GetAimPosition(mgr, 0.f) + zeus::CVector3f(x, 0.f, y);
      ChangeProjectileOwner(hitActor, mgr);
    }
  }

  if (vulnType != EVulnerability::Immune && !deflect) {
    deflect =
        (type == EWeaponCollisionResponseTypes::Unknown15 || type == EWeaponCollisionResponseTypes::EnemyShielded ||
         (type >= EWeaponCollisionResponseTypes::Unknown69 && type <= EWeaponCollisionResponseTypes::Unknown93));
  }

  SetTranslation(offsetPos);

  if (deflect) {
    done = false;
    x2c0_homingTargetId = kInvalidUniqueId;
    x3d0_25_ = false;
  } else {
    StopProjectile(mgr);
    if (x3d0_27_camShakeDirty) {
      x2fc_camShake.SetSfxPositionAndDistance(pos, 50.f);
      mgr.GetCameraManager()->AddCameraShaker(x2fc_camShake, false);
    }
  }

  PlayImpactSound(pos, type);
  mgr.InformListeners(pos, EListenNoiseType::ProjectileExplode);
  if (auto particle = x170_projectile.CollisionOccured(type, !done, retargetPlayer, offsetPos, normal, targetPos)) {
    zeus::CTransform particleXf = zeus::lookAt(zeus::CVector3f::skZero, normal);
    particleXf.origin = offsetPos;
    if (xf0_weaponType != EWeaponType::Power || !xf8_filter.GetExcludeList().HasMaterial(EMaterialTypes::Player) ||
        !x2e4_27_inWater) {
      if (auto decal = x170_projectile.GetDecalForCollision(type)) {
        CDecalManager::AddDecal(*decal, particleXf,
                                (xe8_projectileAttribs & EProjectileAttrib::Ice) != EProjectileAttrib::Ice, mgr);
      }
      zeus::CVector3f scale = zeus::CVector3f::skOne;
      bool camClose = false;
      if (mgr.GetPlayer().GetCameraState() == CPlayer::EPlayerCameraState::FirstPerson) {
        float mag = (offsetPos - mgr.GetCameraManager()->GetCurrentCamera(mgr)->GetTranslation()).magnitude();
        if (mag < 4.f) {
          scale = zeus::CVector3f(0.75f * (mag * 0.25f) + 0.25f);
          camClose = true;
        }
      }
      u32 explodeFlags = 0x8;
      if ((xe8_projectileAttribs & EProjectileAttrib::Ice) == EProjectileAttrib::Ice)
        explodeFlags |= 0x4;
      if (camClose)
        explodeFlags |= 0x2;
      CEntityInfo explosionInfo(GetAreaIdAlways(), CEntity::NullConnectionList);
      CExplosion* explosion =
          new CExplosion(*particle, mgr.AllocateUniqueId(), true, explosionInfo, "Projectile collision response",
                         particleXf, explodeFlags, scale, zeus::CColor::skWhite);
      mgr.AddObject(explosion);
      if (TCastToPtr<CActor> hActor = mgr.ObjectById(hitActor)) {
        bool validPlat = false;
        CScriptPlatform* plat = TCastToPtr<CScriptPlatform>(hActor.GetPtr()).GetPtr();
        validPlat = plat != nullptr;
        if (!validPlat && hActor->GetMaterialList().HasMaterial(EMaterialTypes::Bomb)) {
          for (CEntity* ent : mgr.GetPlatformAndDoorObjectList()) {
            if (TCastToPtr<CScriptPlatform> otherPlat = ent) {
              if (otherPlat->IsSlave(hitActor)) {
                plat = otherPlat.GetPtr();
                validPlat = true;
                break;
              }
            }
          }
        }
        if (validPlat)
          plat->AddSlave(explosion->GetUniqueId(), mgr);
      }
    } else {
      x3d0_24_dead = true;
    }

    if ((xe8_projectileAttribs & (EProjectileAttrib::ComboShot | EProjectileAttrib::Ice)) ==
        (EProjectileAttrib::ComboShot | EProjectileAttrib::Ice)) {
      /* Ice Spreader */
      TLockedToken<CGenDescription> iceSpreadParticle = g_SimplePool->GetObj("IceSpread1");
      u32 flags = (xe6_27_thermalVisorFlags & 0x2) == 0 ? 1 : 0;
      flags |= 0x2;
      CIceImpact* iceImpact =
          new CIceImpact(iceSpreadParticle, mgr.AllocateUniqueId(), GetAreaIdAlways(), true, "Ice spread explosion",
                         particleXf, flags, zeus::CVector3f::skOne, zeus::CColor::skWhite);
      mgr.AddObject(iceImpact);
    }
  }

  return done;
}

void CEnergyProjectile::StopProjectile(CStateManager& mgr) {
  DeleteProjectileLight(mgr);
  mgr.RemoveWeaponId(xec_ownerId, xf0_weaponType);
  x2e4_24_active = false;
  x68_material = CMaterialList();
  mgr.UpdateActorInSortedLists(*this);
  if (x2e8_sfx) {
    CSfxManager::RemoveEmitter(x2e8_sfx);
    x2e8_sfx.reset();
  }
}

} // namespace urde
