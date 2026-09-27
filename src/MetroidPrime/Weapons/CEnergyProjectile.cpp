#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CRealElement.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CDecalManager.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerGun.hpp"
#include "MetroidPrime/Weapons/CIceImpact.hpp"
#include "Weapons/CCollisionResponseData.hpp"
#include "Weapons/CWeaponDescription.hpp"

#define MATERIAL_FLAG(x) (1 << x)

const CMaterialList CEnergyProjectile::kCheckMaterial(
    MATERIAL_FLAG(kMT_Stone) | MATERIAL_FLAG(kMT_Metal) | MATERIAL_FLAG(kMT_Grass) |
    MATERIAL_FLAG(kMT_Ice) | MATERIAL_FLAG(kMT_Pillar) | MATERIAL_FLAG(kMT_MetalGrating) |
    MATERIAL_FLAG(kMT_Phazon) | MATERIAL_FLAG(kMT_Dirt) | MATERIAL_FLAG(kMT_Lava) |
    MATERIAL_FLAG(kMT_LavaStone) | MATERIAL_FLAG(kMT_Snow) | MATERIAL_FLAG(kMT_MudSlow) |
    MATERIAL_FLAG(kMT_HalfPipe) | MATERIAL_FLAG(kMT_Mud) | MATERIAL_FLAG(kMT_Glass) |
    MATERIAL_FLAG(kMT_Shield) | MATERIAL_FLAG(kMT_Sand) | MATERIAL_FLAG(kMT_CameraPassthrough) |
    MATERIAL_FLAG(kMT_Wood) | MATERIAL_FLAG(kMT_Organic));
#undef MATERIAL_FLAG

CEnergyProjectile::CEnergyProjectile(
    const bool active, const TToken< CWeaponDescription >& desc, const EWeaponType type,
    const CTransform4f& xf, const EMaterialTypes excludeMat, const CDamageInfo& damage,
    const TUniqueId uid, const TAreaId aid, const TUniqueId owner, const TUniqueId homingTarget,
    const uint attribs, const bool underwater, const CVector3f& scale,
    const rstl::optional_object< TLockedToken< CGenDescription > >& visorParticle,
    const ushort visorSfx, const bool sendCollideMsg)
: CGameProjectile(active, desc, rstl::string_l("GameProjectile"), type, xf, excludeMat, damage, uid,
                  aid, owner, homingTarget, attribs, underwater, scale, visorParticle, visorSfx,
                  sendCollideMsg)
, mDir(xf.GetForward())
, mMag(mDir.Magnitude())
, mCamShake(CCameraShakeData::SoftHorizShake(0.5f, 0.75f))
, mDead(false)
, x3d0_25_(false)
, x3d0_26_(false)
, mCamShakeDirty(false)
, mCurTime(0.f) {
  SetThermalFlags(kTF_Hot);
}

void CEnergyProjectile::StopProjectile(CStateManager& mgr) {
  DeleteProjectileLight(mgr);
  mgr.RemoveWeaponId(GetOwnerId(), GetType());
  mActive = false;
  SetMaterial(CMaterialList());
  mgr.UpdateActorInSortedLists(*this);
  if (mSfx) {
    CSfxManager::RemoveEmitter(mSfx);
    mSfx.Clear();
  }
}

const bool CEnergyProjectile::Explode(const CVector3f& pos, const CVector3f& normal,
                                      const EWeaponCollisionResponseTypes type, CStateManager& mgr,
                                      const CDamageVulnerability& dVuln, const TUniqueId hitActor) {
  const CVector3f offsetPos = pos + 0.01f * normal;
  bool done = true;
  bool retargetPlayer = false;
  bool deflect = false;
  CVector3f targetPos = CVector3f::Zero();
  const EVulnerability vulnerability =
      dVuln.GetVulnerability(GetCurrentDamageInfo().GetWeaponMode(), CDamageVulnerability::kRD_No);
  if (vulnerability == kVN_Deflect) {
    const EDeflectionType deflection =
        dVuln.GetDeflectionType(GetCurrentDamageInfo().GetWeaponMode());
    deflect = true;
    switch (deflection) {
    case kDT_None:
      deflect = false;
      break;
    case kDT_RetargetPlayer:
    case kDT_RetargetPlayerCombo: {
      bool canRetarget = false;
      if (deflection == kDT_RetargetPlayer) {
        if (GetType() != kWT_Missile && !HasAttrib(kPA_ComboShot)) {
          canRetarget = true;
        }
      } else {
        canRetarget = true;
      }
      if (canRetarget) {
        if (GetFilter().GetExcludeList().HasMaterial(kMT_Player)) {
          retargetPlayer = true;
        }
      }
      break;
    }
    default:
      break;
    }
    if (retargetPlayer) {
      const float angle = mgr.Random()->Range(0.f, 2.f * M_PIF);
      const float sinAngle = CMath::FastSinR(angle);
      const float cosAngle = CMath::FastCosR(angle);
      targetPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f) + CVector3f(cosAngle, 0.f, sinAngle);
      ChangeProjectileOwner(hitActor, mgr);
    }
  }
  if (vulnerability != kVN_Immune && !deflect) {
    deflect = type == kWCR_Unknown15 || type == kWCR_EnemyShielded ||
              (type >= kWCR_Unknown69 && type <= kWCR_AtomicAlphaReflect);
  }
  SetTranslation(offsetPos);
  if (deflect) {
    done = false;
    SetHomingTargetId(kInvalidUniqueId);
    x3d0_25_ = false;
  } else {
    StopProjectile(mgr);
    if (mCamShakeDirty) {
      mCamShake.SetSfxPositionAndDistance(50.f, pos);
      mgr.CameraManager()->AddCameraShaker(mCamShake, false);
    }
  }
  PlayImpactSound(pos, type);
  mgr.InformListeners(pos, kLNT_ProjectileExplode);
  CProjectileWeapon& projectile = Projectile();
  const rstl::optional_object< TLockedToken< CGenDescription > > particle =
      projectile.CollisionOccured(type, !done, retargetPlayer, offsetPos, normal, targetPos);
  if (particle.valid()) {
    CTransform4f particleXf = CTransform4f::LookAt(CVector3f::Zero(), normal);
    particleXf.SetTranslation(offsetPos);
    const bool underwaterPower =
        (GetType() == kWT_Power && GetFilter().GetExcludeList().HasMaterial(kMT_Player)) &&
        mInWater;
    if (!underwaterPower) {
      const rstl::optional_object< TLockedToken< CDecalDescription > > decal =
          projectile.GetDecalForCollision(type);
      if (decal.valid()) {
        CDecalManager::AddDecal(*decal, particleXf,
                               HasAttrib(kPA_Ice) ? CDecalManager::kTH_Cold : CDecalManager::kTH_Hot,
                               mgr);
      }
      CVector3f scale(1.f, 1.f, 1.f);
      bool cameraClose = false;
      if (mgr.GetPlayer()->GetCameraState() == CPlayer::kCS_FirstPerson) {
        const CVector3f delta = particleXf.GetTranslation() -
                                mgr.GetCameraManager()->GetCurrentCamera(mgr).GetTranslation();
        const float distance = delta.Magnitude();
        if (distance < 4.f) {
          const float factor = 0.75f * (distance / 4.f) + 0.25f;
          scale = CVector3f(factor, factor, factor);
        }
        cameraClose = distance < 4.f;
      }
      if (!cameraClose && gpMain->GetAverageTickTime() + gpMain->GetAverageDrawTime() > 0.8f) {
        cameraClose = true;
      }
      uint flags = 8;
      if (HasAttrib(kPA_Ice)) {
        flags |= 4;
      }
      if (cameraClose) {
        flags |= 2;
      }
      CEntity* explosion =
          rs_new CExplosion(*particle, mgr.AllocateUniqueId(), true,
                            CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList),
                            rstl::string_l("Projectile collision response"), particleXf, flags,
                            scale, CColor::White());
      mgr.AddObject(explosion);
      if (CActor* hit = TCastToPtr< CActor >(mgr.ObjectById(hitActor))) {
        bool hasPlatform = false;
        CScriptPlatform* platform = TCastToPtr< CScriptPlatform >(hit);
        if (platform) {
          hasPlatform = true;
        } else if (hit->GetMaterialList().HasMaterial(kMT_PlatformSlave)) {
          CObjectList& platforms = mgr.ObjectListById(kOL_PlatformAndDoor);
          for (int i = platforms.GetFirstObjectIndex(); i != -1;
               i = platforms.GetNextObjectIndex(i)) {
            if (CScriptPlatform* const other = TCastToPtr< CScriptPlatform >(platforms[i])) {
              if (other->IsSlave(hitActor)) {
                platform = other;
                hasPlatform = true;
                break;
              }
            }
          }
        }
        if (hasPlatform) {
          platform->AddSlave(explosion->GetUniqueId(), mgr);
        }
      }
    } else {
      mDead = true;
    }
    if ((GetAttribField() & (kPA_ComboShot | kPA_Ice)) == (kPA_ComboShot | kPA_Ice)) {
      const TLockedToken< CGenDescription > iceParticle = gpSimplePool->GetObj("IceSpread1");
      mgr.AddObject(rs_new CIceImpact(iceParticle, mgr.AllocateUniqueId(), GetCurrentAreaId(), true,
                                      rstl::string_l("Ice spread explosion"), particleXf,
                                      ((GetThermalFlags() & 2) == 0 ? 1 : 0) | 2,
                                      CVector3f(1.f, 1.f, 1.f), CColor(1.f, 1.f, 1.f, 1.f)));
    }
  }
  return done;
}

void CEnergyProjectile::Touch(CActor&, CStateManager&) {}

void CEnergyProjectile::AddToRenderer(const CFrustumPlanes& frustum,
                                      const CStateManager& mgr) const {
  const rstl::optional_object< CAABox > bounds = GetProjectile().GetBounds();
  if (bounds.valid() && !frustum.BoxInFrustumPlanes(bounds)) {
    return;
  }

  const CPlayerState::EPlayerVisor visor = mgr.GetPlayerState()->GetActiveVisor(mgr);

  if (visor != CPlayerState::kPV_XRay &&
      ((GetAttribField() & kPA_Ice) != kPA_Ice || mgr.GetThermalDrawFlag() != kTD_Hot)) {
    GetProjectile().AddToRenderer();
  }

  EnsureRendered(mgr);
}

void CEnergyProjectile::Render(const CStateManager& mgr) const {
  const CPlayerState::EPlayerVisor visor = mgr.GetPlayerState()->GetActiveVisor(mgr);
  if (visor == CPlayerState::kPV_Combat && (HasAttrib(kPA_Charged) || HasAttrib(kPA_ComboShot))) {
    const float warpTime = 1.f - CCast::ToReal32(GetProjectile().GameTime());
    if (warpTime > 0.f) {
      mgr.DrawSpaceWarp(GetTranslation(), 0.75f * warpTime);
    }
  }
  if (visor == CPlayerState::kPV_XRay) {
    CElementGen::SetSubtractBlend(!HasAttrib(kPA_Ice));
    CGraphics::SetFog(kRFM_PerspLin, 0.f, 75.f, CColor::Black());
    GetProjectile().RenderParticles();
    CGameProjectile::Render(mgr);
    mgr.SetupFogForArea(GetCurrentAreaId());
    CElementGen::SetSubtractBlend(false);
  } else if (HasAttrib(kPA_Ice) && mgr.GetThermalDrawFlag() == kTD_Hot) {
    CElementGen::SetSubtractBlend(true);
    GetProjectile().RenderParticles();
    CGameProjectile::Render(mgr);
    mgr.SetupFogForArea(GetCurrentAreaId());
    CElementGen::SetSubtractBlend(false);
  } else {
    CGameProjectile::Render(mgr);
  }
}

void CEnergyProjectile::Think(float dt, CStateManager& mgr) {
  CWeapon::Think(dt, mgr);
  if (GetCurrentAreaId() != mgr.GetWorld()->GetCurrentAreaId() && HasAttrib(kPA_ArmCannon)) {
    mgr.SetActorAreaId(*this, mgr.GetWorld()->GetCurrentAreaId());
  }
  UpdateProjectileMovement(dt, mgr);
  TUniqueId id = kInvalidUniqueId;
  const CRayCastResult result = DoCollisionCheck(id, mgr);
  if (result.IsValid()) {
    if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(id))) {
      ResolveCollisionWithActor(result, *actor, mgr);
    } else {
      ResolveCollisionWithWorld(result, mgr);
    }
  }
  CProjectileWeapon& projectile = Projectile();
  projectile.UpdateParticleFX();
  if (mActive && x3d0_26_) {
    Explode(GetTranslation(), CVector3f::Up(), kWCR_Default, mgr,
            CDamageVulnerability::NormalVulnerability(), kInvalidUniqueId);
  }
  if (GetProjectileLightId() != kInvalidUniqueId) {
    if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(GetProjectileLightId()))) {
      light->SetTransform(GetTransform());
      light->SetTranslation(GetTranslation());
      if (projectile.GetAttachedPS1() && projectile.GetAttachedPS1()->SystemHasLight()) {
        light->SetLight(projectile.GetAttachedPS1()->GetLight());
      }
    }
  }
  if (mSfx) {
    CSfxManager::UpdateEmitter(mSfx, projectile.GetTranslation(), projectile.GetVelocity(),
                               255);
    CSfxManager::PitchBend(mSfx, mWaterUpdate ? 0 : 8192);
  }
  mCurTime += dt;
  if (mCurTime > 45.f) {
    mgr.DeleteObjectRequest(GetUniqueId());
  } else if (projectile.IsSystemDeletable() || mDead) {
    mgr.DeleteObjectRequest(GetUniqueId());
  }
}

void CEnergyProjectile::ResolveCollisionWithActor(const CRayCastResult& res, CActor& act,
                                                  CStateManager& mgr) {
  SetLastObjectId(act.GetUniqueId());
  const EWeaponCollisionResponseTypes type =
      act.GetCollisionResponseType(res.GetPoint(), GetTransform().GetForward().AsNormalized(),
                                   GetCurrentDamageInfo().GetWeaponMode(), GetAttribField());
  act.Touch(*this, mgr);
  if (!Explode(res.GetPoint(), res.GetPlane().GetNormal(), type, mgr, *act.GetDamageVulnerability(),
               act.GetUniqueId())) {
    mgr.DeliverScriptMsg(&act, GetUniqueId(), kSM_Touched);
    act.SendScriptMsgs(kSS_ReflectedDamage, mgr, kSM_None);
  } else {
    CGameProjectile::ResolveCollisionWithActor(res, act, mgr);
    ApplyDamageToActors(mgr, GetCurrentDamageInfo());
  }
  if (CEnergyProjectile* projectile = TCastToPtr< CEnergyProjectile >(act)) {
    projectile->SetHitProjectileOwner(GetOwnerId());
    projectile->Explode(GetTranslation(), GetTransform().GetForward(), kWCR_OtherProjectile, mgr,
                        *GetDamageVulnerability(), GetUniqueId());
  }
}

void CEnergyProjectile::ResolveCollisionWithWorld(const CRayCastResult& res, CStateManager& mgr) {
  const EWeaponCollisionResponseTypes type =
      CCollisionResponseData::GetWorldCollisionResponseType(CMaterialList::BitPosition(
          (kCheckMaterial.GetValue() & res.GetMaterial().GetValue()) & 0xffffffff));
  if ((GetAttribField() & (kPA_Wave | kPA_ComboShot)) != (kPA_Wave | kPA_ComboShot)) {
    if (Explode(res.GetPoint(), res.GetPlane().GetNormal(), type, mgr,
                CDamageVulnerability::NormalVulnerability(), kInvalidUniqueId)) {
      mgr.ApplyDamageToWorld(GetOwnerId(), *this, res.GetPoint(), GetCurrentDamageInfo(),
                             GetFilter());
    }
    SetLastObjectId(kInvalidUniqueId);
  }
}

ENTITY_ACCEPT_IMPL(CEnergyProjectile)

void CEnergyProjectile::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                        CStateManager& mgr) {
  switch (msg) {
  case kSM_Deleted:
    if (mActive) {
      mgr.RemoveWeaponId(GetOwnerId(), GetType());
    }
    if (mSfx) {
      CSfxManager::RemoveEmitter(mSfx);
      mSfx.Clear();
    }
    break;
  case kSM_Registered: {
    CProjectileWeapon& projectile = Projectile();
    if (projectile.GetAttachedPS1() && projectile.GetAttachedPS1()->SystemHasLight()) {
      CreateProjectileLight(rstl::string_l("ProjectileLight_GameProjectile"),
                            projectile.GetAttachedPS1()->GetLight(), mgr);
    }
    if (projectile.GetWeaponDescription()->mPJFX >= 0) {
      float range = 50.f;
      float falloff = 0.2f;
      if (projectile.GetWeaponDescription()->mRNGE) {
        projectile.GetWeaponDescription()->mRNGE->GetValue(0, range);
      }
      if (projectile.GetWeaponDescription()->mFOFF) {
        projectile.GetWeaponDescription()->mFOFF->GetValue(0, falloff);
      }
      CAudioSys::C3DEmitterParmData parms(range, falloff, 9, 255, 20);
      parms.mPos = GetProjectile().GetTranslation();
      parms.mDir = GetProjectile().GetVelocity();
      parms.mSfxId =
          CSfxManager::TranslateSFXID(GetProjectile().GetWeaponDescription()->mPJFX);
      mSfx = CSfxManager::AddEmitter(parms, true, CSfxManager::kMedPriority, true);
    }
    mgr.AddWeaponId(GetOwnerId(), GetType());
    break;
  }
  default:
    break;
  }
  CGameProjectile::AcceptScriptMsg(msg, uid, mgr);
}

void CEnergyProjectile::ChangeProjectileOwner(TUniqueId owner, CStateManager& mgr) {
  if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(owner))) {
    mOrigDamageInfo.MultiplyDamageAndRadius(
        gpTweakPlayerGun->GetRichochetDamage(mOrigDamageInfo.GetWeaponMode().GetType()));
    mgr.RemoveWeaponId(GetOwnerId(), GetType());
    SetOwnerId(owner);
    mgr.AddWeaponId(owner, GetType());
    const CMaterialFilter& filter = GetFilter();
    CMaterialList exclude = filter.GetExcludeList();
    exclude.Add(kMT_Character);
    exclude.Remove(kMT_Player);
    SetFilter(CMaterialFilter::MakeIncludeExclude(filter.GetIncludeList(), exclude));
  }
}

void CEnergyProjectile::PlayImpactSound(const CVector3f& pos, EWeaponCollisionResponseTypes type) {
  CProjectileWeapon& projectile = Projectile();
  const int sfx = projectile.GetSoundIdForCollision(type);
  if (sfx >= 0) {
    const float range = projectile.GetAudibleRange();
    const float falloff = projectile.GetAudibleFallOff();
    CAudioSys::C3DEmitterParmData parms(range, falloff, 1, CAudioSys::kMaxVolume, 20);
    parms.mPos = pos;
    parms.mSfxId = CSfxManager::TranslateSFXID(static_cast< ushort >(sfx));
    const CSfxHandle handle = CSfxManager::AddEmitter(parms, true);
    if (mWaterUpdate) {
      CSfxManager::PitchBend(handle, 0);
    }
  }
}

void CEnergyProjectile::SetCameraShake(const CCameraShakeData& data) {
  mCamShake = data;
  mCamShakeDirty = true;
}
