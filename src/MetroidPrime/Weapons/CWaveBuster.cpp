#include "MetroidPrime/Weapons/CWaveBuster.hpp"

#include "Collision/CCollidableAABox.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleSwoosh.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "dolphin/gx.h"
#include "dolphin/os/OSCache.h"

static const CVector3f kTargetNodePosition(0.f, -3.f, -1.5f);
static const CVector3f kSourceNodePosition(0.f, 2.f, 1.5f);

CWaveBuster::CWaveBuster(const TToken< CWeaponDescription >& desc, EWeaponType type,
                         const CTransform4f& xf, EMaterialTypes matType, const CDamageInfo& dInfo,
                         TUniqueId uid, TAreaId aid, TUniqueId owner, TUniqueId homingTarget,
                         uint attribs)
: CGameProjectile(true, desc, rstl::string_l("WaveBuster"), type, xf, matType, dInfo, uid, aid,
                  owner, homingTarget, attribs, false, CVector3f(1.f, 1.f, 1.f),
                  rstl::optional_object_null(), CSfxManager::kInternalInvalidSfxId, false)
, mOriginalXf(xf)
, mBezierB(CVector3f::Zero())
, mBezierC(CVector3f::Zero())
, mPreviousBezierC(CVector3f::Zero())
, mHomingTargetPoint(CVector3f::Zero())
, mTargetPoint(mOriginalXf.GetTranslation() +
                   25.f * mOriginalXf.GetForward().AsNormalized())
, mBusterSwoosh1(gpSimplePool->GetObj("BusterSwoosh1"))
, mBusterSwoosh2(gpSimplePool->GetObj("BusterSwoosh2"))
, mBusterSparks(gpSimplePool->GetObj("BusterSparks"))
, mBusterLight(gpSimplePool->GetObj("BusterLight"))
, mBusterSwoosh1Gen(rs_new CParticleSwoosh(mBusterSwoosh1, 0))
, mBusterSwoosh2Gen(rs_new CParticleSwoosh(mBusterSwoosh2, 0))
, mBusterSparksGen(rs_new CElementGen(mBusterSparks))
, mBusterLightGen(rs_new CElementGen(mBusterLight))
, mRand(99)
, mSpiralOffset(2.f * M_PIF)
, x39c_(0.5f)
, mBezierBlend(0.5f)
, mSourceAngleRate(0.f)
, mSourceAngleTimer(0.f)
, mSourceAngle(0.f)
, mSourceRadius(0.f)
, mSourceRadiusRate(0.f)
, mSourceRadiusTimer(0.f)
, mTargetAngleRate(0.f)
, mTargetAngleTimer(0.f)
, mTargetAngle(0.f)
, mInnerSwooshColorT(0.f)
, mInnerSwooshColorIdx(0)
, mFiring(true)
, mSeeking(true)
, mTrackingTarget(false)
, mCollided(false)
, mCollidedWithWorld(true) {
  const rstl::vector< CParticleSwoosh::SSwooshData >& swooshes =
      mBusterSwoosh1Gen->GetSwooshes();
  for (int i = 0; i < swooshes.size() - 1; ++i) {
    mBusterSwoosh1Gen->SetWarmUp();
    mBusterSwoosh1Gen->Update(0.0);
    mBusterSwoosh2Gen->SetWarmUp();
    mBusterSwoosh2Gen->Update(0.0);
  }
}

ENTITY_ACCEPT_IMPL(CWaveBuster)

void CWaveBuster::Touch(CActor& actor, CStateManager& mgr) { CActor::Touch(actor, mgr); }

void CWaveBuster::SetNewTarget(TUniqueId uid, CStateManager& mgr) {
  SetHomingTargetId(uid);
  if (uid != kInvalidUniqueId) {
    mTrackingTarget = true;
    mBezierBlend = 0.f;
    CSfxManager::AddEmitter(0x6ff, GetTranslation(), CVector3f::Zero(), true, false,
                            CSfxManager::kMaxPriority, CSfxManager::kAllAreas);
    mgr.GetRumbleManager()->Rumble(mgr, kRFX_PlayerBump, 0.5f, kRP_Three);
  } else {
    mTrackingTarget = false;
  }
}

float CWaveBuster::GetViewAngleToTarget(CVector3f& direction, const CActor& actor) const {
  direction = actor.GetTranslation() - mOriginalXf.GetTranslation();
  if (direction.CanBeNormalized()) {
    direction.Normalize();
  } else {
    direction = GetTransform().GetForward();
  }
  return CVector2f::GetAngleDiff(mOriginalXf.GetForward().DropZ(), direction.DropZ());
}

bool CWaveBuster::UpdateBeamFrame(CStateManager& mgr, float dt) {
  CVector3f direction = CVector3f::Forward();
  float viewAngle = 0.f;
  if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(GetHomingTargetId()))) {
    if (actor->GetHealthInfo(mgr) != nullptr && actor->GetHealthInfo(mgr)->GetHP() > 0.f) {
      const CVector3f delta = actor->GetTranslation() - mOriginalXf.GetTranslation();
      if (delta.MagSquared() > 10000.f) {
        return true;
      }
      viewAngle = GetViewAngleToTarget(direction, *actor);
    }
  }
  mSourceAngleTimer -= dt;
  if (mSourceAngleTimer <= 0.f) {
    const float angle = mgr.Random()->Range(0.f, 2.f * M_PIF);
    const float duration = mgr.Random()->Range(0.05f, 0.25f);
    mSourceAngleRate = (1.f / duration) * (angle - mSourceAngle);
    mSourceAngleTimer = duration;
  }
  mSourceRadiusTimer -= dt;
  if (mSourceRadiusTimer <= 0.f) {
    const float radius = mgr.Random()->Range(0.f, 0.5f);
    const float duration = mgr.Random()->Range(0.1f, 0.5f);
    mSourceRadiusRate = (1.f / duration) * (radius - mSourceRadius);
    mSourceRadiusTimer = duration;
  }
  mTargetAngleTimer -= dt;
  if (mTargetAngleTimer <= 0.f) {
    const float angle = mgr.Random()->Range(0.f, 2.f * M_PIF);
    const float duration = mgr.Random()->Range(0.05f, 0.25f);
    mTargetAngleRate = (1.f / duration) * (angle - mSourceAngle);
    mTargetAngleTimer = duration;
  }
  mSourceAngle += mSourceAngleRate * dt;
  mSourceRadius += mSourceRadiusRate * dt;
  mTargetAngle += mTargetAngleRate * dt;
  CVector3f sourceNode = kSourceNodePosition;
  sourceNode[kDZ] *= (GetHomingTargetId() != kInvalidUniqueId ? 1.25f : 1.f) -
                     mSourceRadius * mSourceRadius;
  mBezierB = mOriginalXf * CTransform4f::RotateY(CRelAngle::FromRadians(mSourceAngle)) *
                 sourceNode;
  return viewAngle > M_PIF / 2.f;
}

void CWaveBuster::UpdateTargetDamage(float dt, CStateManager& mgr) {
  if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(GetHomingTargetId()))) {
    if (actor->GetHealthInfo(mgr) != nullptr && actor->GetHealthInfo(mgr)->GetHP() > 0.f) {
      mHomingTargetPoint = actor->GetAimPosition(mgr, 0.f);
      SetTranslation(mHomingTargetPoint);
      const CMaterialFilter& filter = GetFilter();
      const CDamageInfo& damage = GetCurrentDamageInfo().MakeScaledForTime(dt);
      mgr.ApplyDamage(GetUniqueId(), GetHomingTargetId(), GetOwnerId(), damage, filter,
                      CVector3f::Zero());
      return;
    }
  }
  SetTransform(CTransform4f::LookAt(
      GetTranslation(), mTargetPoint + 0.001f * mOriginalXf.GetForward().AsNormalized(),
      CVector3f::Up()));
  SetHomingTargetId(kInvalidUniqueId);
  x39c_ = 0.f;
  mBezierBlend = 0.f;
}

CRayCastResult CWaveBuster::SeekTarget(TUniqueId& uid, float dt, CStateManager& mgr) {
  const CVector3f delta = GetTranslation() - GetPreviousPos();
  TEntityList nearList;
  mgr.BuildNearList(
      nearList, GetProjectileBounds(),
      CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid),
                                          CMaterialList(kMT_ProjectilePassthrough, kMT_Player)),
      this);
  const CRayCastResult result = RayCollisionCheckWithWorld(uid, GetPreviousPos(), GetTranslation(),
                                                           delta.Magnitude(), nearList, mgr);
  if (result.IsValid()) {
    mSeeking = false;
    if (uid != kInvalidUniqueId && uid == GetHomingTargetId()) {
      mTrackingTarget = true;
      CSfxManager::AddEmitter(0x6ff, result.GetPoint(), CVector3f::Zero(), true, false,
                              CSfxManager::kMaxPriority, CSfxManager::kAllAreas);
    } else {
      SetHomingTargetId(kInvalidUniqueId);
    }
  } else {
    UpdateProjectileMovement(dt, mgr);
  }
  return result;
}

void CWaveBuster::UpdateTargetSeek(float dt, CStateManager& mgr) {
  TUniqueId uid = kInvalidUniqueId;
  SeekTarget(uid, dt, mgr);
  if (GetHomingTargetId() != kInvalidUniqueId) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(GetHomingTargetId()))) {
      CVector3f direction = CVector3f::Forward();
      if (GetViewAngleToTarget(direction, *actor) > M_PIF / 2.f) {
        SetHomingTargetId(kInvalidUniqueId);
        mTrackingTarget = false;
      } else if (mTrackingTarget) {
        mSeeking = false;
      }
    }
  } else if (!mTrackingTarget) {
    const CVector3f delta = GetTranslation() - mOriginalXf.GetTranslation();
    if (delta.MagSquared() > 625.f) {
      mSeeking = false;
    }
  }
}

void CWaveBuster::Think(float dt, CStateManager& mgr) {
  CWeapon::Think(dt, mgr);
  if (!GetActive()) {
    return;
  }
  if (GetCurrentAreaId() != mgr.GetWorld()->GetCurrentAreaId()) {
    mgr.SetActorAreaId(*this, mgr.GetWorld()->GetCurrentAreaId());
  }
  mCollided = false;
  mCollidedWithWorld = false;
  float beamDistance = 25.f;
  const CVector3f origin = mOriginalXf.GetTranslation();
  const CVector3f forward = mOriginalXf.GetForward().AsNormalized();
  if (!mSeeking && !mTrackingTarget) {
    TUniqueId uid = kInvalidUniqueId;
    const CRayCastResult result = CollideWithWorld(uid, origin, forward, mgr, dt);
    if (result.IsValid() && result.GetTime() < 25.f) {
      beamDistance = result.GetTime();
      if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(uid))) {
        actor->Touch(*this, mgr);
        const CVector3f& direction = GetTransform().GetForward();
        const CMaterialFilter& filter = GetFilter();
        const CDamageInfo& damage = GetCurrentDamageInfo().MakeScaledForTime(dt);
        mgr.ApplyDamage(GetUniqueId(), actor->GetUniqueId(), GetOwnerId(), damage, filter,
                        direction);
      } else {
        mCollidedWithWorld = true;
      }
      mCollided = true;
    }
  }
  if (GetHomingTargetId() != kInvalidUniqueId && mTrackingTarget) {
    UpdateTargetDamage(dt, mgr);
  } else {
    beamDistance = CMath::FastFSel(beamDistance - 1.f, beamDistance, 1.f);
    mTargetPoint = origin + beamDistance * forward;
    if (mSeeking) {
      UpdateTargetSeek(dt, mgr);
    } else {
      const float x = mgr.Random()->Range(-1.f, 1.f);
      const float z = mgr.Random()->Range(-1.f, 1.f);
      mTargetPoint += CVector3f(x, 0.f, z);
      SetTranslation(mTargetPoint);
    }
  }
  if (UpdateBeamFrame(mgr, dt)) {
    ResetBeam(true);
  }
  const CVector3f target =
      mHomingTargetId != kInvalidUniqueId && mTrackingTarget
          ? GetTransform() * CTransform4f::RotateY(CRelAngle::FromRadians(mTargetAngle)) *
                kTargetNodePosition
          : GetTranslation() +
                kTargetNodePosition.GetZ() * GetTransform().GetForward().AsNormalized();
  if (mBezierBlend < 0.5f && GetHomingTargetId() != kInvalidUniqueId) {
    mBezierC = CVector3f::Lerp(mPreviousBezierC, target, mBezierBlend / 0.5f);
    mBezierBlend += 0.125f * dt;
  } else {
    mPreviousBezierC = mBezierC;
    mBezierC = target;
    mBezierBlend = 0.5f;
  }
  if (mProjectileLight != kInvalidUniqueId) {
    mBusterLightGen->Update(dt);
    if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(GetProjectileLightId()))) {
      light->SetTransform(GetTransform());
      light->SetTranslation(GetTranslation());
      if (!mBusterLightGen.null() && mBusterLightGen->SystemHasLight()) {
        light->SetLight(mBusterLightGen->GetLight());
      }
    }
  }
  mInnerSwooshColorT += 20.f * dt;
  if (mInnerSwooshColorT > 1.f) {
    ++mInnerSwooshColorIdx;
    if (mInnerSwooshColorIdx > 2) {
      mInnerSwooshColorIdx = 0;
    }
    mInnerSwooshColorT = 0.f;
  }
  mBusterSparksGen->Update(dt);
}

void CWaveBuster::UpdateFx(const CTransform4f& xf, float dt, CStateManager& mgr) {
  if (GetActive()) {
    mOriginalXf = xf;
    mSpiralOffset -= 60.f * dt;
    if (mSpiralOffset < 0.f) {
      mSpiralOffset = 2.f * M_PIF;
    }
    mProjectile.SetVelocity(CVector3f(0.f, mSeeking ? 1.6f : 0.f, 0.f));
  }
}

void CWaveBuster::AddToRenderer(const CFrustumPlanes& planes, const CStateManager& mgr) const {
  EnsureRendered(mgr, mOriginalXf.GetTranslation(), GetSortingBounds(mgr));
}

void CWaveBuster::Render(const CStateManager& mgr) const {
  RenderSwooshes();
  RenderElectricSpiral();
  CWeapon::Render(mgr);
}

void CWaveBuster::ResetBeam(bool deactivate) {
  if (deactivate) {
    SetActive(false);
    mFiring = false;
    mBusterSparksGen->SetParticleEmission(false);
    mSpiralOffset = 2.f * M_PIF;
  } else {
    mBusterSparksGen->SetParticleEmission(false);
    mFiring = false;
  }
}

void CWaveBuster::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  switch (msg) {
  case kSM_Registered:
    if (!mBusterLightGen.null() && mBusterLightGen->SystemHasLight()) {
      CreateProjectileLight(rstl::string_l("WaveBuster_Light"), mBusterLightGen->GetLight(),
                            mgr);
    }
    SetThermalFlags(kTF_Hot);
    mBezierB = mOriginalXf.GetTranslation();
    mBezierC = GetTranslation();
    mPreviousBezierC = mBezierC;
    break;
  case kSM_Deleted:
    DeleteProjectileLight(mgr);
    break;
  }
  CGameProjectile::AcceptScriptMsg(msg, sender, mgr);
}

inline void DrawLineList(const CVector3f* vertices, const CColor& color, int count, int width) {
  CGX::SetLineWidth(width, GX_TO_ZERO);
  CGX::Begin(GX_LINESTRIP, GX_VTXFMT0, count);
  for (int i = 0; i < count; ++i) {
    GXPosition3f32(vertices[i][kDX], vertices[i][kDY], vertices[i][kDZ]);
    GXColor1u32(color.GetColor_u32());
  }
  CGX::End();
}

void CWaveBuster::RenderElectricSpiral() const {
  const CTransform4f inverse = mOriginalXf.GetInverse();
  const CVector3f a = inverse * mOriginalXf.GetTranslation();
  const CVector3f b = inverse * mBezierB;
  const CVector3f c = inverse * mBezierC;
  const CVector3f d = inverse * GetTranslation();
  float radius = 0.f;
  CVector3f* vertices = reinterpret_cast< CVector3f* >(LCGetBase());
  CVector3f previous = a;
  for (float t = 0.16f; t <= 1.f; t += 0.16f) {
    const CVector3f point = CMath::GetBezierPoint(a, b, c, d, t);
    float angle = 0.f;
    for (int i = 0; i < 36; ++i) {
      const float randX = mRand.Range(-0.041667f, 0.041667f);
      const float randZ = mRand.Range(-0.041667f, 0.041667f);
      const float x = radius * CMath::FastCosR(angle + mSpiralOffset) + randX;
      const float z = radius * CMath::FastSinR(angle + mSpiralOffset) + randZ;
      *vertices++ = CVector3f::Lerp(previous, point, angle / (2.f * M_PIF)) + CVector3f(x, 0.f, z);
      angle += CMath::Deg2Rad(10.f);
    }
    radius = 0.25f;
    previous = point;
  }
  GXVtxDescList vtxDesc[] = {
      {GX_VA_POS, GX_DIRECT}, {GX_VA_CLR0, GX_DIRECT}, {GX_VA_NULL, GX_NONE}};
  CGX::SetVtxDescv(vtxDesc);
  CGX::SetNumChans(1);
  CGX::SetNumTevStages(1);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_CLEAR);
  CGX::SetNumTexGens(0);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  gpRender->SetModelMatrix(mOriginalXf);
  DrawLineList(reinterpret_cast< CVector3f* >(LCGetBase()), CColor::White(), 216, 12);
  DrawLineList(reinterpret_cast< CVector3f* >(LCGetBase()), CColor(1.f, 0.f, 1.f, 0.5f), 216, 48);
  CGX::SetLineWidth(6, GX_TO_ZERO);
}

void CWaveBuster::RenderSwooshes() const {
  static const uint colors[] = {0xffffffff, 0xff00ffff, 0xff0000ff, 0x0000ffff};
  const CTransform4f rotation = mOriginalXf.GetRotation();
  const CVector3f origin = mOriginalXf.GetTranslation();
  mBusterSparksGen->SetParticleEmission(true);
  const CColor color(CColor::Lerp(colors[mInnerSwooshColorIdx],
                                  colors[mInnerSwooshColorIdx + 1], mInnerSwooshColorT));
  rstl::vector< CParticleSwoosh::SSwooshData >& swooshes1 = mBusterSwoosh1Gen->Swooshes();
  rstl::vector< CParticleSwoosh::SSwooshData >& swooshes2 = mBusterSwoosh2Gen->Swooshes();
  float t = 0.f;
  float previousRot1 = swooshes1[swooshes1.size() - 1].mInitialRot;
  float previousRot2 = swooshes2[swooshes2.size() - 1].mInitialRot;
  for (int i = 0; i < swooshes1.size(); ++i) {
    CParticleSwoosh::SSwooshData& swoosh1 = swooshes1[i];
    CParticleSwoosh::SSwooshData& swoosh2 = swooshes2[i];
    const CVector3f point =
        CMath::GetBezierPoint(GetTranslation(), mBezierC, mBezierB, origin, t);
    swoosh1.mTranslation = point;
    swoosh2.mTranslation = point;
    swoosh1.mOrientation = rotation;
    swoosh2.mOrientation = rotation;
    swoosh2.mColor = color;
    const float rot1 = swoosh1.mInitialRot;
    const float rot2 = swoosh2.mInitialRot;
    swoosh1.mInitialRot = previousRot1;
    previousRot1 = rot1;
    swoosh2.mInitialRot = previousRot2;
    previousRot2 = rot2;
    mBusterSparksGen->SetTranslation(point);
    mBusterSparksGen->ForceParticleCreation(1);
    t += 0.04f;
  }
  mBusterSparksGen->SetParticleEmission(false);
  mBusterSwoosh1Gen->Render();
  mBusterSwoosh2Gen->Render();
  mBusterSparksGen->Render();
}

CRayCastResult CWaveBuster::CollideWithWorld(TUniqueId& uid, const CVector3f& pos,
                                             const CVector3f& dir, CStateManager& mgr, float dt) {
  const CRayCastResult staticResult = mgr.RayStaticIntersection(pos, dir, 25.f, GetFilter());
  CRayCastResult physicsResult;
  CRayCastResult actorResult = physicsResult;
  TUniqueId physicsId = kInvalidUniqueId;
  TUniqueId actorId = kInvalidUniqueId;
  CollideWithObject(mgr, physicsId, actorId, pos, dir, 25.f, physicsResult, actorResult);
  if (physicsResult.IsValid()) {
    if (IsNearest(actorId, physicsResult, actorResult, staticResult, dt, mgr)) {
      uid = physicsId;
      return physicsResult;
    }
  } else if (actorResult.IsValid() &&
             IsNearest(physicsId, actorResult, physicsResult, staticResult, dt, mgr)) {
    uid = actorId;
    return actorResult;
  }
  return staticResult;
}

void CWaveBuster::CollideWithObject(const CStateManager& mgr, TUniqueId& physicsId,
                                    TUniqueId& actorId, const CVector3f& pos, const CVector3f& dir,
                                    float length, CRayCastResult& physicsResult,
                                    CRayCastResult& actorResult) {
  TEntityList nearList;
  const CAABox box = CAABox(CVector3f(-0.5f, 0.f, -0.5f), CVector3f(0.5f, 25.f, 0.5f))
                         .GetTransformedAABox(mOriginalXf);
  mgr.BuildNearList(
      nearList, box,
      CMaterialFilter::MakeExclude(CMaterialList(kMT_ProjectilePassthrough, kMT_Player)), this);
  const float maxLength = length > 0.f ? length : 100000.f;
  float physicsLength = maxLength;
  float actorLength = maxLength;
  for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
    if (const CPhysicsActor* actor = TCastToConstPtr< CPhysicsActor >(mgr.GetObjectById(*it))) {
      const CRayCastResult result = actor->GetCollisionPrimitive()->CastRay(
          pos, dir, physicsLength, GetFilter(), actor->GetPrimitiveTransform());
      if (result.IsValid() && result.GetTime() < physicsLength) {
        physicsResult = result;
        physicsId = actor->GetUniqueId();
        physicsLength = result.GetTime();
      }
    } else if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(*it))) {
      const rstl::optional_object< CAABox > bounds = actor->GetTouchBounds();
      if (!bounds) {
        continue;
      }
      {
        CCollidableAABox primitive = CCollidableAABox(*bounds, actor->GetMaterialList());
        const CRayCastResult result =
            primitive.CastRay(pos, dir, actorLength, GetFilter(), CTransform4f::Identity());
        if (!result.IsValid()) {
          continue;
        }
        if (result.GetTime() < actorLength) {
          actorResult = result;
          actorId = actor->GetUniqueId();
          actorLength = result.GetTime();
        }
      }
    }
  }
}

bool CWaveBuster::IsNearest(const TUniqueId& otherId, const CRayCastResult& result,
                            const CRayCastResult& otherResult, const CRayCastResult& staticResult,
                            float dt, CStateManager& mgr) {
  bool nearest = false;
  if (!staticResult.IsValid()) {
    nearest = true;
  } else if (staticResult.GetTime() >= result.GetTime()) {
    nearest = true;
  }
  if (nearest) {
    if (otherResult.IsValid()) {
      if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(otherId))) {
        actor->Touch(*this, mgr);
        const CVector3f direction = GetTransform().GetForward();
        const CMaterialFilter& filter = GetFilter();
        const CDamageInfo& damage = GetCurrentDamageInfo().MakeScaledForTime(dt);
        mgr.ApplyDamage(GetUniqueId(), actor->GetUniqueId(), GetOwnerId(), damage, filter,
                        direction);
      }
    }
    return true;
  }
  return false;
}

rstl::optional_object< CAABox > CWaveBuster::GetTouchBounds() const {
  if (mCollidedWithWorld) {
    return rstl::optional_object_null();
  }
  return GetProjectileBounds();
}

CWaveBuster::~CWaveBuster() {}
