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
, mFlameXf(xf)
, mFlameBounds(CAABox::MakeNullBox())
, mParticleWaitDelayTimer(0.f)
, mFireStopTimer(0.f)
, mFlame(flameInfo.GetX10())
, mFlamethrowerDesc(gpSimplePool->GetObj(SObjectTag('PART', flameInfo.GetFlameFxId())))
, mFlameGen(rs_new CElementGen(mFlamethrowerDesc))
, mFlameWarp(float(flameInfo.GetLength()), xf.GetTranslation(),
                 (flameInfo.GetAttributes() & 4) != 0)
, mFlameState(kFS_Default)
, mPlayerSteamTextureId(playerSteamTxtr)
, mPlayerHitSfx(playerHitSfx)
, mPlayerIceTextureId(playerIceTxtr)
, mActive(false)
, mParticlesActive(false)
, mZTest((flameInfo.GetAttributes() & 1) == 0)
, mConeCollision((flameInfo.GetAttributes() & 2) != 0)
#if VERSION >= VERSION_GM8P_00
, x428_28_((flameInfo.GetAttributes() & 8) != 0)
#endif
{
}

rstl::optional_object< CAABox > CFlameThrower::GetTouchBounds() const {
  // Retail retains this impossible state condition.
  if (!GetActive() || mFlameState != kFS_FireActive || mFlameState != kFS_FireStopTimer) {
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
  const CTransform4f& xf = mFlameXf;
  UpdateFlameState(dt, mgr);

  const CVector3f flamePoint = xf.GetTranslation();
  const bool firing = mFlameState == kFS_FireActive || mFlameState == kFS_FireStopTimer;
  switch (mFlameState) {
  case kFS_FireActive:
  case kFS_FireStopTimer:
    mFlameWarp.Activate(true);
    mFlameWarp.SetWarpPoint(flamePoint);
    mFlameWarp.SetStateManager(mgr);
    mFlameGen->SetTranslation(flamePoint);
    mFlameGen->SetOrientation(xf.GetRotation());
    break;
  default:
    mFlameWarp.Activate(false);
    break;
  }
  mFlameGen->Update(dt);
  mFlameWarp.SetMaxDistSq(0.f);
  mFlameWarp.SetFloatingPoint(flamePoint);

  if (firing && mFlameWarp.IsProcessed()) {
    const rstl::reserved_vector< CVector3f, 9 >& points = mFlameWarp.GetCollisionPoints();
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
    mFlameBounds = CAABox(min, max);
    const CRayCastResult result = DoCollisionCheck(id, mFlameBounds, mgr);
    if (TCastToPtr< CActor >(mgr.ObjectById(id))) {
      ApplyFlameDamageToActors(mgr, id, dt);
    } else if (result.IsValid()) {
#if VERSION >= VERSION_GM8P_00
      const CDamageInfo scaledDamage = mCurDamageInfo.MakeScaledForTime(dt);
      if (x428_28_ && !mgr.GetPlayer()->GetFrozenState()) {
        DoRadialFreeze(result.GetPoint(), scaledDamage, mgr);
      }
      ApplyDamageToWorld(mgr, GetOwnerId(), result.GetPoint(), scaledDamage, GetFilter());
#else
      ApplyDamageToWorld(mgr, GetOwnerId(), result.GetPoint(),
                         mCurDamageInfo.MakeScaledForTime(dt), GetFilter());
#endif
    }
  }

  CActor::SetTransform(xf.GetRotation());
  CActor::SetTranslation(xf.GetTranslation());
  if (GetProjectileLightId() != kInvalidUniqueId) {
    if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(GetProjectileLightId()))) {
      light->SetTransform(GetTransform());
      light->SetTranslation(mFlameWarp.GetFloatingPoint());
      if (mFlameGen.get() && mFlameGen->SystemHasLight()) {
        light->SetLight(mFlameGen->GetLight());
      }
    }
  }
}

float CFlameThrower::UpdateFlameState(float dt, CStateManager& mgr) {
  switch (mFlameState) {
  case kFS_FireStart:
    mFlameState = kFS_FireActive;
    break;
  case kFS_FireStopTimer:
    mFireStopTimer += 4.f * dt;
    if (mFireStopTimer > 1.f) {
      mFireStopTimer = 1.f;
      mFlameState = kFS_FireWaitForParticlesDone;
      mActive = false;
    }
    break;
  case kFS_FireWaitForParticlesDone:
    mParticleWaitDelayTimer += dt;
    if (mParticleWaitDelayTimer > 0.1f && mFlameGen.get() &&
        mFlameGen->GetParticleCountAll() == 0) {
      mFlameState = kFS_Default;
      Reset(mgr, true);
    }
    break;
  default:
    break;
  }
  return 0.f;
}

void CFlameThrower::SetTransform(const CTransform4f& xf, CStateManager&, float) {
  mFlameXf = xf;
}

void CFlameThrower::AddToRenderer(const CFrustumPlanes&, const CStateManager& mgr) const {
  gpRender->AddParticleGen(*mFlameGen);
  EnsureRendered(mgr, mFlameXf.GetTranslation(), GetSortingBounds(mgr));
}

void CFlameThrower::Render(const CStateManager&) const {
  if (!GetActive()) {
    return;
  }
}

void CFlameThrower::Fire(const CTransform4f&, CStateManager& mgr, bool) {
  SetActive(true);
  mParticlesActive = true;
  mActive = true;
  mFlameState = kFS_FireStart;
  CreateFlameParticles(mgr);
}

void CFlameThrower::Reset(CStateManager& mgr, bool resetWarp) {
  SetFlameLightActive(false, mgr);
  if (resetWarp) {
    SetActive(false);
    mParticlesActive = false;
    mFlameState = kFS_Default;
    mParticleWaitDelayTimer = 0.f;
    mFireStopTimer = 0.f;
    mFlameBounds = CAABox::MakeNullBox();
    mFlameGen->SetParticleEmission(false);
    mFlameWarp.ResetPosition(mFlameXf.GetTranslation());
  } else {
    mFlameGen->SetParticleEmission(false);
    mParticlesActive = false;
    mFlameState = kFS_FireStopTimer;
  }
}

void CFlameThrower::CreateFlameParticles(CStateManager& mgr) {
  DeleteProjectileLight(mgr);
  mFlameGen = rs_new CElementGen(mFlamethrowerDesc);
  if (mFlameGen.get()) {
    mFlameGen->SetZTest(mZTest);
    mFlameGen->AddModifier(&mFlameWarp);
    if (mFlameGen->SystemHasLight() && mProjectileLight == kInvalidUniqueId) {
      CreateProjectileLight(rstl::string_l("FlameThrower_Light"), mFlameGen->GetLight(), mgr);
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
  const rstl::reserved_vector< CVector3f, 9 >& points = mFlameWarp.GetCollisionPoints();
  if (mConeCollision && points.size() > 0) {
    const float radiusPitch =
        (mFlameWarp.GetMaxSize() - mFlameWarp.GetMinSize()) / float(points.size()) * 0.5f;
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
            const CVector3f delta = actor->GetAimPosition(mgr, 0.f) - mFlameXf.GetTranslation();
            result =
                box.CastRay(mFlameXf.GetTranslation(), delta.AsNormalized(), delta.Magnitude(),
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

void CFlameThrower::ApplyFlameDamageToActors(CStateManager& mgr, TUniqueId id, float dt) {

#if VERSION >= VERSION_GM8P_00
  ApplyFlameDamageToActor(id, mgr);
#else
  if (id == mgr.GetPlayer()->GetUniqueId() && mPlayerSteamTextureId != kInvalidAssetId &&
      mPlayerIceTextureId != kInvalidAssetId) {
    mgr.Player()->SetFrozenState(mgr, mPlayerSteamTextureId, mPlayerHitSfx,
                                 mPlayerIceTextureId);
  }
#endif
  ApplyDamageToActors(mgr, mCurDamageInfo.MakeScaledForTime(dt));
}
#if VERSION >= VERSION_GM8P_00
void CFlameThrower::ApplyFlameDamageToActor(TUniqueId id, CStateManager& mgr) {
  if (id == mgr.GetPlayer()->GetUniqueId() && mPlayerSteamTextureId != kInvalidAssetId &&
      mPlayerIceTextureId != kInvalidAssetId) {
    mgr.Player()->SetFrozenState(mgr, mPlayerSteamTextureId, mPlayerHitSfx,
                                 mPlayerIceTextureId);
  }
}
#endif

inline void CFlameThrower::ApplyDamageToWorld(CStateManager& mgr, TUniqueId id,
                                              const CVector3f& point, const CDamageInfo& dInfo,
                                              const CMaterialFilter& filter) {
  mgr.ApplyDamageToWorld(id, *this, point, dInfo, filter);
}

#if VERSION >= VERSION_GM8P_00
void CFlameThrower::DoRadialFreeze(const CVector3f&, const CDamageInfo&, CStateManager& mgr) {}

// Needed to force destructor generation;
static void hack() {
  CFlameThrower* t;
  t->~CFlameThrower();
}
#endif
