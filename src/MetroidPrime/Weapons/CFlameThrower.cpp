#include "MetroidPrime/Weapons/CFlameThrower.hpp"

#include "Collision/CCollidableAABox.hpp"
#include "Collision/COBBox.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"

const CVector3f CFlameThrower::kLightOffset = CVector3f(0.f, 3.f, 2.f);

CFlameThrower::CFlameThrower(const TToken< CWeaponDescription >& wDesc, const rstl::string& name,
                             const EWeaponType wType, const CFlameInfo& flameInfo,
                             const CTransform4f& xf, const EMaterialTypes matType,
                             const CDamageInfo& dInfo, const TUniqueId uid, const TAreaId aId,
                             const TUniqueId owner, EProjectileAttrib attribs,
                             const CAssetId playerSteamTxtr, const ushort playerHitSfx,
                             const CAssetId playerIceTxtr)
: CGameProjectile(false, wDesc, name, wType, xf, matType, dInfo, uid, aId, owner, kInvalidUniqueId,
                  attribs, false, CVector3f(1.f, 1.f, 1.f), rstl::optional_object_null(),
                  CSfxManager::kInternalInvalidSfxId, false)
, x2e8_flameXf(xf)
, x318_flameBounds(CAABox::MakeNullBox())
, x330_particleWaitDelayTimer(0.f)
, x334_fireStopTimer(0.f)
, x338_flame(flameInfo.GetX10())
, x33c_flamethrowerDesc(gpSimplePool->GetObj(SObjectTag('PART', flameInfo.GetFlameFxId())))
, x348_flameGen(rs_new CElementGen(x33c_flamethrowerDesc))
, x34c_flameWarp(float(flameInfo.GetLength()), xf.GetTranslation(),
                 (flameInfo.GetAttributes() & 4) != 0)
, x3f0_flameState(kFS_Default)
, x3f4_playerSteamTextureId(playerSteamTxtr)
, x3f8_playerHitSfx(playerHitSfx)
, x3fc_playerIceTextureId(playerIceTxtr)
, x400_24_active(false)
, x400_25_particlesActive(false)
, x400_26_zTest((flameInfo.GetAttributes() & 1) == 0)
, x400_27_coneCollision((flameInfo.GetAttributes() & 2) != 0) {}

rstl::optional_object< CAABox > CFlameThrower::GetTouchBounds() const {
  // Retail retains this impossible state condition.
  if (!GetActive() || x3f0_flameState != kFS_FireActive || x3f0_flameState != kFS_FireStopTimer) {
    return rstl::optional_object_null();
  }
  const CVector3f translation = GetTranslation();
  return CAABox(
      translation.GetX() - kProjectileBoxAllowance, translation.GetY() - kProjectileBoxAllowance,
      translation.GetZ() - kProjectileBoxAllowance, kProjectileBoxAllowance + translation.GetX(),
      kProjectileBoxAllowance + translation.GetY(), kProjectileBoxAllowance + translation.GetZ());
}

ENTITY_ACCEPT_IMPL(CFlameThrower)

void CFlameThrower::Touch(CActor&, CStateManager&) {}

void CFlameThrower::Think(float dt, CStateManager& mgr) {
  CWeapon::Think(dt, mgr);
  if (!GetActive()) {
    return;
  }
  const CTransform4f& xf = x2e8_flameXf;
  UpdateFlameState(dt, mgr);

  const CVector3f flamePoint = xf.GetTranslation();
  const bool firing = x3f0_flameState == kFS_FireActive || x3f0_flameState == kFS_FireStopTimer;
  switch (x3f0_flameState) {
  case kFS_FireActive:
  case kFS_FireStopTimer:
    x34c_flameWarp.Activate(true);
    x34c_flameWarp.SetWarpPoint(flamePoint);
    x34c_flameWarp.SetStateManager(mgr);
    x348_flameGen->SetTranslation(flamePoint);
    x348_flameGen->SetOrientation(xf.GetRotation());
    break;
  default:
    x34c_flameWarp.Activate(false);
    break;
  }
  x348_flameGen->Update(dt);
  x34c_flameWarp.SetMaxDistSq(0.f);
  x34c_flameWarp.SetFloatingPoint(flamePoint);

  if (firing && x34c_flameWarp.IsProcessed()) {
    const rstl::reserved_vector< CVector3f, 9 >& points = x34c_flameWarp.GetCollisionPoints();
    CVector3f min = points.front();
    CVector3f max = points.front();
    for (AUTO(it, points.begin() + 1); it != points.end(); ++it) {
      if (it->GetX() < min.GetX())
        min.SetX(it->GetX());
      if (it->GetY() < min.GetY())
        min.SetY(it->GetY());
      if (it->GetZ() < min.GetZ())
        min.SetZ(it->GetZ());
      if (it->GetX() > max.GetX())
        max.SetX(it->GetX());
      if (it->GetY() > max.GetY())
        max.SetY(it->GetY());
      if (it->GetZ() > max.GetZ())
        max.SetZ(it->GetZ());
    }
    TUniqueId id = kInvalidUniqueId;
    x318_flameBounds = CAABox(min, max);
    const CRayCastResult result = DoCollisionCheck(id, x318_flameBounds, mgr);
    if (const CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(id))) {
      ApplyDamageToActor(mgr, id, dt);
    } else if (result.IsValid()) {
      mgr.ApplyDamageToWorld(GetOwnerId(), *this, result.GetPoint(),
                             x12c_curDamageInfo.MakeScaledForTime(dt), GetFilter());
    }
  }

  CActor::SetTransform(xf.GetRotation());
  CActor::SetTranslation(xf.GetTranslation());
  if (GetProjectileLightId() != kInvalidUniqueId) {
    if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(GetProjectileLightId()))) {
      light->SetTransform(GetTransform());
      light->SetTranslation(x34c_flameWarp.GetFloatingPoint());
      if (x348_flameGen.get() && x348_flameGen->SystemHasLight()) {
        light->SetLight(x348_flameGen->GetLight());
      }
    }
  }
}

float CFlameThrower::UpdateFlameState(float dt, CStateManager& mgr) {
  switch (x3f0_flameState) {
  case kFS_FireStart:
    x3f0_flameState = kFS_FireActive;
    break;
  case kFS_FireStopTimer:
    x334_fireStopTimer += 4.f * dt;
    if (x334_fireStopTimer > 1.f) {
      x334_fireStopTimer = 1.f;
      x3f0_flameState = kFS_FireWaitForParticlesDone;
      x400_24_active = false;
    }
    break;
  case kFS_FireWaitForParticlesDone:
    x330_particleWaitDelayTimer += dt;
    if (x330_particleWaitDelayTimer > 0.1f && x348_flameGen.get() &&
        x348_flameGen->GetParticleCountAll() == 0) {
      x3f0_flameState = kFS_Default;
      Reset(mgr, true);
    }
    break;
  default:
    break;
  }
  return 0.f;
}

void CFlameThrower::SetTransform(const CTransform4f& xf, CStateManager&, float) {
  x2e8_flameXf = xf;
}

void CFlameThrower::AddToRenderer(const CFrustumPlanes&, const CStateManager& mgr) const {
  gpRender->AddParticleGen(*x348_flameGen);
  EnsureRendered(mgr, x2e8_flameXf.GetTranslation(), GetSortingBounds(mgr));
}

void CFlameThrower::Render(const CStateManager&) const {
  if (!GetActive()) {
    return;
  }
}

void CFlameThrower::Fire(const CTransform4f&, CStateManager& mgr, bool) {
  SetActive(true);
  x400_25_particlesActive = true;
  x400_24_active = true;
  x3f0_flameState = kFS_FireStart;
  CreateFlameParticles(mgr);
}

void CFlameThrower::Reset(CStateManager& mgr, bool resetWarp) {
  SetFlameLightActive(false, mgr);
  if (resetWarp) {
    SetActive(false);
    x400_25_particlesActive = false;
    x3f0_flameState = kFS_Default;
    x330_particleWaitDelayTimer = 0.f;
    x334_fireStopTimer = 0.f;
    x318_flameBounds = CAABox::MakeNullBox();
    x348_flameGen->SetParticleEmission(false);
    x34c_flameWarp.ResetPosition(x2e8_flameXf.GetTranslation());
  } else {
    x348_flameGen->SetParticleEmission(false);
    x400_25_particlesActive = false;
    x3f0_flameState = kFS_FireStopTimer;
  }
}

void CFlameThrower::CreateFlameParticles(CStateManager& mgr) {
  DeleteProjectileLight(mgr);
  x348_flameGen = rs_new CElementGen(x33c_flamethrowerDesc);
  if (x348_flameGen.get()) {
    x348_flameGen->SetZTest(x400_26_zTest);
    x348_flameGen->AddModifier(&x34c_flameWarp);
    if (x348_flameGen->SystemHasLight() && x2c8_projectileLight == kInvalidUniqueId) {
      CreateProjectileLight(rstl::string_l("FlameThrower_Light"), x348_flameGen->GetLight(), mgr);
    }
  }
}

void CFlameThrower::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case kSM_Registered:
    SetThermalFlags(kTF_Hot);
    mgr.AddWeaponId(GetOwnerId(), GetType());
    break;
  case kSM_Deleted:
    mgr.RemoveWeaponId(GetOwnerId(), GetType());
    DeleteProjectileLight(mgr);
    break;
  default:
    break;
  }
  CGameProjectile::AcceptScriptMsg(msg, uid, mgr);
}

CRayCastResult CFlameThrower::DoCollisionCheck(TUniqueId& idOut, const CAABox& bounds,
                                               CStateManager& mgr) {
  CRayCastResult result = CRayCastResult::MakeInvalid();
  TEntityList nearList;
  mgr.BuildNearList(nearList, bounds, CMaterialFilter::skPassEverything, this);
  const rstl::reserved_vector< CVector3f, 9 >& points = x34c_flameWarp.GetCollisionPoints();
  if (x400_27_coneCollision && points.size() > 0) {
    const float radiusPitch =
        (x34c_flameWarp.GetMaxSize() - x34c_flameWarp.GetMinSize()) / float(points.size()) * 0.5f;
    float radius = radiusPitch;
    for (int i = 1; i < points.size(); ++i) {
      const CVector3f delta = points[i] - points[i - 1];
      CTransform4f xf = CTransform4f::LookAt(points[i - 1], points[i]);
      xf.SetTranslation(points[i - 1] + delta * 0.5f);
      const COBBox obb(xf, CVector3f(radius, delta.Magnitude() * 0.5f, radius));
      for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
        if (CActor* actor = static_cast< CActor* >(mgr.ObjectById(*it))) {
          const CProjectileTouchResult touch = CanCollideWith(*actor, mgr);
          if (touch.GetActorId() == kInvalidUniqueId) {
            continue;
          }
          const rstl::optional_object< CAABox > touchBounds = actor->GetTouchBounds();
          if (!touchBounds) {
            continue;
          }
          if (obb.AABoxIntersectsBox(*touchBounds)) {
            const CCollidableAABox box = CCollidableAABox(*touchBounds, actor->GetMaterialList());
            const CVector3f delta = actor->GetAimPosition(mgr, 0.f) - x2e8_flameXf.GetTranslation();
            result =
                box.CastRay(x2e8_flameXf.GetTranslation(), delta.AsNormalized(), delta.Magnitude(),
                            CMaterialFilter::skPassEverything, CTransform4f::Identity());
            if (result.IsInvalid()) {
              continue;
            }
            return result;
          }
        }
      }
      radius += radiusPitch;
    }
  } else {
    for (int i = 0; i < points.size() - 1; ++i) {
      const float magnitude = (points[i + 1] - points[i]).Magnitude();
      if (magnitude <= 0.f) {
        break;
      }
      result =
          RayCollisionCheckWithWorld(idOut, points[i], points[i + 1], magnitude, nearList, mgr);
      if (result.IsValid()) {
        return result;
      }
    }
  }
  return result;
}

void CFlameThrower::SetFlameLightActive(bool active, CStateManager& mgr) {
  if (GetProjectileLightId() == kInvalidUniqueId) {
    return;
  }
  if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(GetProjectileLightId()))) {
    light->SetActive(active);
  }
}

void CFlameThrower::ApplyDamageToActor(CStateManager& mgr, TUniqueId id, float dt) {
  if (id == mgr.GetPlayer()->GetUniqueId() && x3f4_playerSteamTextureId != kInvalidAssetId &&
      x3fc_playerIceTextureId != kInvalidAssetId) {
    mgr.Player()->SetFrozenState(mgr, x3f4_playerSteamTextureId, x3f8_playerHitSfx,
                                 x3fc_playerIceTextureId);
  }
  ApplyDamageToActors(mgr, x12c_curDamageInfo.MakeScaledForTime(dt));
}
