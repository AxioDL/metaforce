#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "Collision/CCollidableAABox.hpp"
#include "Collision/CMaterialList.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Enemies/CPuddleToadGamma.hpp"
#include "MetroidPrime/Enemies/CWallCrawlerSwarm.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CHUDBillboardEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDoor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "Weapons/CProjectileWeapon.hpp"
#include "Weapons/CWeaponDescription.hpp"
#include "rstl/math.hpp"
#include "rstl/optional_object.hpp"
#include <math.h>

#include <Collision/CMaterialFilter.hpp>

const float CGameProjectile::kProjectileBoxAllowance = 0.1f;

static CTransform4f clear_transform(const CTransform4f& xf) {
  CTransform4f result(xf);
  result.SetTranslation(CVector3f::Zero());
  return result;
}

CGameProjectile::CGameProjectile(
    const bool active, const TToken< CWeaponDescription >& wDesc, const rstl::string& name,
    const EWeaponType wType, const CTransform4f& xf, const EMaterialTypes excludeMat,
    const CDamageInfo& dInfo, const TUniqueId uid, const TAreaId aid, const TUniqueId owner,
    const TUniqueId homingTarget, const uint attribs, const bool underwater, const CVector3f& scale,
    const rstl::optional_object< TLockedToken< CGenDescription > >& visorParticle,
    const ushort visorSfx, const bool sendCollideMsg)
: CWeapon(uid, aid, active, owner, wType, name, xf,
          CMaterialFilter::MakeIncludeExclude(
              CMaterialList(kMT_Solid, kMT_NonSolidDamageable),
              CMaterialList(kMT_Projectile, kMT_ProjectilePassthrough, excludeMat)),
          CMaterialList(kMT_Projectile), dInfo, attribs | GetBeamAttribType(wType),
          CModelData::CModelDataNull())
, mVisorParticle(visorParticle)
, mVisorSfx(visorSfx)
, mProjectile(wDesc, xf.GetTranslation(), clear_transform(xf), scale,
                  (attribs & kPA_ParticleOPTS) ? 1 : 0)
, mPreviousPos(xf.GetTranslation())
, mProjExtent((mProjectileAttribs & kPA_BigProjectile) == kPA_BigProjectile ? 0.25f : 0.1f)
, mHomingDt(0.03f)
, mTargetHomingTime(0.0)
, mCurHomingTime(mHomingDt)
, mHomingTargetId(homingTarget)
, mLastResolvedObj(kInvalidUniqueId)
, mHitProjectileOwner(kInvalidUniqueId)
, mPendingDamagee(kInvalidUniqueId)
, mProjectileLight(kInvalidUniqueId)
, mWpscId(wDesc.GetTag().GetId())
, mMinHomingDist(0.f)
, mActive(true)
, mStartedUnderwater(underwater)
, mWaterUpdate(underwater)
, mInWater(underwater)
, mSendProjectileCollideMsg(sendCollideMsg) {}

void CGameProjectile::Render(const CStateManager& mgr) const {
  mProjectile.Render();
  CWeapon::Render(mgr);
}

CAABox CGameProjectile::GetProjectileBounds() const {
  const CVector3f translation = GetTranslation();
  return CAABox(rstl::min_val(mPreviousPos.GetX(), translation.GetX()) - mProjExtent,
                rstl::min_val(mPreviousPos.GetY(), translation.GetY()) - mProjExtent,
                rstl::min_val(mPreviousPos.GetZ(), translation.GetZ()) - mProjExtent,
                rstl::max_val(mPreviousPos.GetX(), translation.GetX()) + mProjExtent,
                rstl::max_val(mPreviousPos.GetY(), translation.GetY()) + mProjExtent,
                rstl::max_val(mPreviousPos.GetZ(), translation.GetZ()) + mProjExtent);
}

rstl::optional_object< CAABox > CGameProjectile::GetTouchBounds() const {
  if (!mActive) {
    return rstl::optional_object_null();
  }

  return GetProjectileBounds();
}

CProjectileTouchResult CGameProjectile::CanCollideWithTrigger(CActor& act, CStateManager& mgr) {
  const bool isWater = TCastToPtr< CScriptWater >(act) != nullptr;
  if (isWater) {
    const bool enteredWater = (isWater && !mStartedUnderwater) &&
                              !mProjectile.GetWeaponDescription()->mEWTR;
    const bool leftWater = (!isWater && mStartedUnderwater) &&
                           !mProjectile.GetWeaponDescription()->mLWTR;
    const bool collide = enteredWater || leftWater;
    return CProjectileTouchResult(collide ? act.GetUniqueId() : kInvalidUniqueId,
                                  rstl::optional_object_null());
  }
  return CProjectileTouchResult(kInvalidUniqueId, rstl::optional_object_null());
}

CProjectileTouchResult CGameProjectile::CanCollideWithGameObject(CActor& act, CStateManager& mgr) {
  CGameProjectile* proj = TCastToPtr< CGameProjectile >(act);
  if (!proj) {
    if (!act.GetMaterialList().HasMaterial(kMT_Solid) && !act.HealthInfo(mgr)) {
      return CProjectileTouchResult(kInvalidUniqueId, rstl::optional_object_null());
    } else if (act.GetUniqueId() == GetOwnerId()) {
      return CProjectileTouchResult(kInvalidUniqueId, rstl::optional_object_null());
    } else if (act.GetUniqueId() == mLastResolvedObj) {
      return CProjectileTouchResult(kInvalidUniqueId, rstl::optional_object_null());
    } else if (act.GetMaterialList().SharesMaterials(GetFilter().GetExcludeList())) {
      return CProjectileTouchResult(kInvalidUniqueId, rstl::optional_object_null());
    } else if (CPatterned* ai = TCastToPtr< CPatterned >(act)) {
      if (!ai->CanBeShot(mgr, GetAttribField())) {
        return CProjectileTouchResult(kInvalidUniqueId, rstl::optional_object_null());
      }
    }
  } else if (HasAttrib(kPA_PartialCharge) || proj->HasAttrib(kPA_PartialCharge)) {
    return CProjectileTouchResult(act.GetUniqueId(), rstl::optional_object_null());
  } else if (!HasAttrib(kPA_PartialCharge) && !proj->HasAttrib(kPA_PartialCharge)) {
    return CProjectileTouchResult(kInvalidUniqueId, rstl::optional_object_null());
  }
  return CProjectileTouchResult(act.GetUniqueId(), rstl::optional_object_null());
}

CProjectileTouchResult CGameProjectile::CanCollideWithComplexCollision(CActor& act,
                                                                       CStateManager& mgr) {
  CPhysicsActor* useAct = nullptr;
  if (CScriptPlatform* const platform = TCastToPtr< CScriptPlatform >(act)) {
    if (platform->HasComplexCollision()) {
      useAct = platform;
    }
  } else if (PATTERNED_CAST_TO(CPuddleToadGamma, &act)) {
    useAct = static_cast< CPhysicsActor* >(&act);
  } else if (CCollisionActor* const collisionActor = TCastToPtr< CCollisionActor >(act)) {
    if (collisionActor->GetOwnerId() == GetOwnerId()) {
      return CProjectileTouchResult(kInvalidUniqueId, rstl::optional_object_null());
    }
    useAct = collisionActor;
  }
  if (useAct) {
    const CCollisionPrimitive* prim = useAct->GetCollisionPrimitive();
    const CTransform4f xf = useAct->GetPrimitiveTransform();
    const CVector3f delta = GetTranslation() - mPreviousPos;
    if (delta.CanBeNormalized()) {
      const CVector3f dir = delta.AsNormalized();
      const float mag = delta.Magnitude();
      const CRayCastResult res =
          prim->CastRay(mPreviousPos, dir, mag,
                        CMaterialFilter::MakeIncludeExclude(
                            CMaterialList(kMT_Solid), CMaterialList(kMT_ProjectilePassthrough)),
                        xf);
      if (!res.IsValid()) {
        if (prim->GetPrimType() != 'SPHR') {
          const CRayCastResult second =
              prim->CastRay(mPreviousPos - 1.f * (mag * dir), dir, 2.f * mag,
                            CMaterialFilter::MakeIncludeExclude(
                                CMaterialList(kMT_Solid), CMaterialList(kMT_ProjectilePassthrough)),
                            xf);
          if (second.IsValid()) {
            return CProjectileTouchResult(act.GetUniqueId(), second);
          }
        } else if (CCollisionActor* const collisionActor = TCastToPtr< CCollisionActor >(act)) {
          const float radius = collisionActor->GetSphereRadius();
          const CVector3f delta = mPreviousPos - collisionActor->GetTranslation();
          if (CVector3f::Dot(delta, delta) < radius * radius) {
            const CVector3f point = mPreviousPos - 1.125f * (radius * dir);
            const CPlane plane(point, CUnitVector3f(-dir));
            return CProjectileTouchResult(act.GetUniqueId(),
                                          CRayCastResult(0.f, point, plane, act.GetMaterialList()));
          }
        }
        return CProjectileTouchResult(kInvalidUniqueId, rstl::optional_object_null());
      } else {
        return CProjectileTouchResult(act.GetUniqueId(), res);
      }
    } else {
      return CProjectileTouchResult(kInvalidUniqueId, rstl::optional_object_null());
    }
  } else {
    return CProjectileTouchResult(act.GetUniqueId(), rstl::optional_object_null());
  }
}

CProjectileTouchResult CGameProjectile::CanCollideWith(CActor& act, CStateManager& mgr) {
  if (act.GetDamageVulnerability()->GetVulnerability(
          mCurDamageInfo.GetWeaponMode(), CDamageVulnerability::kRD_No) == kVN_PassThrough) {
    return CProjectileTouchResult(kInvalidUniqueId, rstl::optional_object_null());
  }
  if (TCastToPtr< CScriptTrigger >(act)) {
    return CanCollideWithTrigger(act, mgr);
  } else if (TCastToPtr< CScriptPlatform >(act) || TCastToPtr< CCollisionActor >(act) ||
             PATTERNED_CAST_TO(CPuddleToadGamma, &act)) {
    return CanCollideWithComplexCollision(act, mgr);
  } else {
    return CanCollideWithGameObject(act, mgr);
  }
}

CRayCastResult CGameProjectile::RayCollisionCheckWithWorld(TUniqueId& idOut, const CVector3f& start,
                                                           const CVector3f& end, float mag,
                                                           TEntityList& nearList,
                                                           CStateManager& mgr) {
  mTouchResults.clear();
  idOut = kInvalidUniqueId;
  mPendingDamagee = kInvalidUniqueId;
  CRayCastResult result = CRayCastResult();
  const CVector3f delta = end - start;
  if (!delta.CanBeNormalized()) {
    return result;
  }
  const CVector3f dir = delta.AsNormalized();
  float bestMag = mag;
  const CRayCastResult worldResult = mgr.RayStaticIntersection(start, dir, mag, GetFilter());
  if (worldResult.IsValid()) {
    bestMag = worldResult.GetTime();
    result = worldResult;
  }
  for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
    if (CActor* actor = static_cast< CActor* >(mgr.ObjectById(*it))) {
      CProjectileTouchResult touch = CanCollideWith(*actor, mgr);
      if (touch.GetActorId() == kInvalidUniqueId) {
        continue;
      }
      if (touch.HasRayCastResult()) {
        if (touch.GetRayCastResult().GetTime() < bestMag) {
          actor->Touch(*this, mgr);
          result = touch.GetRayCastResult();
          bestMag = result.GetTime();
          mPendingDamagee = idOut = touch.GetActorId();
        }
      } else {
        rstl::optional_object< CAABox > bounds = actor->GetTouchBounds();
        const CScriptDoor* door = TCastToConstPtr< CScriptDoor >(actor);
        const CGameProjectile* projectile = nullptr;
        if (door) {
          bounds = door->GetProjectileBounds();
        } else if ((projectile = TCastToConstPtr< CGameProjectile >(actor))) {
          bounds = projectile->GetProjectileBounds();
        }
        if (!bounds) {
          continue;
        }
        CCollidableAABox prim = CCollidableAABox(*bounds, actor->GetMaterialList());
        CRayCastResult ray = prim.CastRay(start, dir, mag, CMaterialFilter::skPassEverything,
                                          CTransform4f::Identity());
        if (ray.IsValid()) {
          if (ray.GetTime() < bestMag) {
            bestMag = ray.GetTime();
            result = ray;
            mPendingDamagee = idOut = touch.GetActorId();
          }
        } else if (bounds->PointInside(start) ||
                   (projectile && GetProjectileBounds().DoBoundsOverlap(*bounds))) {
          const CPlane plane(start, CUnitVector3f(-dir));
          result = CRayCastResult(0.f, start, plane, actor->GetMaterialList());
          mPendingDamagee = idOut = actor->GetUniqueId();

          break;
        }
      }
    }
  }
  if (mInWater && idOut == kInvalidUniqueId) {
    mInWater = false;
  }
  return result;
}

ENTITY_ACCEPT_IMPL(CGameProjectile)

void CGameProjectile::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case kSM_Deleted:
    DeleteProjectileLight(mgr);
    break;
  case kSM_AddSplashInhabitant:
    if (mInWater != true) {
      mInWater = true;
      mWaterUpdate = true;
    }
    break;
  case kSM_UpdateSplashInhabitant:
    if (!mWaterUpdate) {
      mWaterUpdate = true;
    }
    break;
  case kSM_RemoveSplashInhabitant:
    if (mWaterUpdate) {
      mWaterUpdate = false;
      mInWater = false;
    }
    break;
  }
}

void CGameProjectile::FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr) {
  if (mProjectile.GetWeaponDescription()->mSWTR) {
    CWeapon::FluidFXThink(state, water, mgr);
  }
}

void CGameProjectile::ApplyDamageToActors(CStateManager& mgr, const CDamageInfo& dInfo) {
  const CVector3f forward = GetTransform().GetForward();
  if (mPendingDamagee != kInvalidUniqueId) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.ObjectById(mPendingDamagee))) {
      mgr.ApplyDamage(GetUniqueId(), actor->GetUniqueId(), GetOwnerId(), dInfo, GetFilter(),
                      forward);
      if (HasAttrib(kPA_PlayerUnFreeze) && actor->GetUniqueId() == mgr.GetPlayer()->GetUniqueId() &&
          mgr.GetPlayer()->GetFrozenState()) {
        mgr.Player()->BreakFrozenState(mgr);
      }
    }
    mPendingDamagee = kInvalidUniqueId;
  }
  for (rstl::vector< CProjectileTouchResult >::const_iterator it = mTouchResults.begin();
       it != mTouchResults.end(); ++it) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(it->GetActorId()))) {
      mgr.ApplyDamage(GetUniqueId(), actor->GetUniqueId(), GetOwnerId(), dInfo, GetFilter(),
                      forward);
      if (HasAttrib(kPA_PlayerUnFreeze) && actor->GetUniqueId() == mgr.GetPlayer()->GetUniqueId() &&
          mgr.GetPlayer()->GetFrozenState()) {
        mgr.Player()->BreakFrozenState(mgr);
      }
    }
  }
  mTouchResults.clear();
}

CRayCastResult CGameProjectile::DoCollisionCheck(TUniqueId& idOut, CStateManager& mgr) {
  CRayCastResult result = CRayCastResult();
  if (mActive) {
    const CVector3f delta = GetTranslation() - mPreviousPos;
    TEntityList nearList;
    mgr.BuildNearList(nearList, GetProjectileBounds(),
                      CMaterialFilter::MakeExclude(CMaterialList(kMT_ProjectilePassthrough)), this);
    result = RayCollisionCheckWithWorld(idOut, mPreviousPos, GetTranslation(),
                                        delta.Magnitude(), nearList, mgr);
  }
  return result;
}

void CGameProjectile::UpdateProjectileMovement(float dt, CStateManager& mgr) {
  float useDt = dt;
  if (mWaterUpdate) {
    useDt = 37.5f * (dt * dt);
  }
  mPreviousPos = GetTranslation();
  mProjectile.Update(useDt);
  SetTransform(mProjectile.GetTransform());
  SetTranslation(mProjectile.GetTranslation());
  UpdateHoming(dt, mgr);
}

void CGameProjectile::UpdateHoming(float dt, CStateManager& mgr) {
  if (mActive && mHomingTargetId != kInvalidUniqueId && mHomingDt > 0.f) {
    mTargetHomingTime += dt;
    while (mTargetHomingTime >= mCurHomingTime) {
      Chase(mHomingDt, mgr);
      mCurHomingTime += mHomingDt;
    }
  }
}

void CGameProjectile::Chase(float dt, CStateManager& mgr) {
  if (mProjectile.IsProjectileActive() && mHomingTargetId != kInvalidUniqueId) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mHomingTargetId))) {
      if (!actor->GetMaterialList().HasMaterial(kMT_Target) &&
          !actor->GetMaterialList().HasMaterial(kMT_Player)) {
        mHomingTargetId = kInvalidUniqueId;
      } else {
        CVector3f homingPos = actor->GetHomingPosition(mgr, 0.f);
        const CWallCrawlerSwarm* swarm = TCastToConstPtr< CWallCrawlerSwarm >(actor);
        if (swarm) {
          const int lockOnId = swarm->GetCurrentLockOnId();
          if (swarm->GetLockOnLocationValid(lockOnId)) {
            homingPos = swarm->GetLockOnLocation(lockOnId);
          } else {
            mHomingTargetId = kInvalidUniqueId;
            return;
          }
        }
        CVector3f delta = homingPos - mProjectile.GetTranslation();
        if (mMinHomingDist > 0.f && delta.Magnitude() < mMinHomingDist) {
          mHomingTargetId = kInvalidUniqueId;
          return;
        }
        const CPhysicsActor* physicsActor = TCastToConstPtr< CPhysicsActor >(actor);
        if (!swarm && !physicsActor) {
          rstl::optional_object< CAABox > bounds = actor->GetTouchBounds();
          if (bounds) {
            delta[kDZ] += (bounds->GetMaxPoint().GetZ() - bounds->GetMinPoint().GetZ()) * 0.5f;
          }
        }
        const CVector3f forward = mProjectile.GetTransform().GetForward();
        CQuaternion rotation = CQuaternion::ShortestRotationArc(forward, delta);
        const float threshold = 2.f * rotation.GetScalar() * rotation.GetScalar() - 1.f;
        if (threshold > 0.99f) {
          return;
        }
        float turnRate;
        if (mWaterUpdate) {
          turnRate = mProjectile.GetMaxTurnRate() * 0.5f;
        } else {
          turnRate = mProjectile.GetMaxTurnRate();
        }
        const CRelAngle maxTurn = CRelAngle::FromDegrees(dt * turnRate);
        const CRelAngle turn = CRelAngle::FromRadians(acosf(threshold));
        if (maxTurn < turn) {
          const double sinHalfTurn = sin(turn.AsRadians() * 0.5f);
          rotation = CQuaternion::ScalarVector(
              cosf(maxTurn.AsRadians() * 0.5f),
              (sinf(maxTurn.AsRadians() * 0.5f) / static_cast< float >(sinHalfTurn)) *
                  rotation.GetVector());
        }
        CTransform4f xf = rotation.BuildTransform4f() * mProjectile.GetTransform();
        xf.Orthonormalize();
        mProjectile.SetWorldSpaceOrientation(xf);
      }
    }
  }
}

void CGameProjectile::CreateProjectileLight(const rstl::string& name, const CLight& light,
                                            CStateManager& mgr) {
  DeleteProjectileLight(mgr);
  mProjectileLight = mgr.AllocateUniqueId();
  const CAssetId sourceId = mWpscId;
  mgr.AddObject(rs_new CGameLight(mProjectileLight, GetAreaId(), GetActive(), name,
                                  GetTransform(), GetUniqueId(), light, sourceId, 0, 0.f));
}

void CGameProjectile::DeleteProjectileLight(CStateManager& mgr) {
  if (mProjectileLight != kInvalidUniqueId) {
    mgr.DeleteObjectRequest(mProjectileLight);
    mProjectileLight = kInvalidUniqueId;
  }
}

CWeapon::EProjectileAttrib CGameProjectile::GetBeamAttribType(EWeaponType type) {
  switch (type) {
  case kWT_Ice:
    return kPA_Ice;
  case kWT_Wave:
    return kPA_Wave;
  case kWT_Plasma:
    return kPA_Plasma;
  case kWT_Phazon:
    return kPA_Phazon;
  default:
    return kPA_None;
  }
}

void CGameProjectile::ResolveCollisionWithActor(const CRayCastResult& res, CActor& act,
                                                CStateManager& mgr) {
  const CVector3f reverseDir = -GetTransform().GetForward().AsNormalized();
  CPlayer* player = TCastToPtr< CPlayer >(act);
  if (player && mVisorParticle &&
      mgr.GetPlayer()->GetCameraState() == CPlayer::kCS_FirstPerson) {
    const float angle =
        360.f *
        CMath::Rad2Rev(CMath::FastArcCosR(CVector3f::Dot(
            reverseDir,
            mgr.GetCameraManager()->GetCurrentCameraTransform(mgr).GetForward().AsNormalized())));
    if (angle <= 45.f) {
      mgr.AddObject(rs_new CHUDBillboardEffect(
          rstl::optional_object< TToken< CGenDescription > >(*mVisorParticle),
          rstl::optional_object_null(), mgr.AllocateUniqueId(), true, rstl::string_l("VisorAcid"),
          CHUDBillboardEffect::GetNearClipDistance(mgr), CHUDBillboardEffect::GetScaleForPOV(mgr),
          CColor(1.f, 1.f, 1.f, 1.f), CVector3f(1.f, 1.f, 1.f), CVector3f(0.f, 0.f, 0.f)));
      CSfxManager::SfxStart(mVisorSfx);
      if (mSendProjectileCollideMsg) {
        mgr.DeliverScriptMsg(mgr.Player(), GetUniqueId(), kSM_ProjectileCollide);
      }
    }
  }
}

CGameProjectile::~CGameProjectile() {}
