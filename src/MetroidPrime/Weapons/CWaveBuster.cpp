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
, x2e8_originalXf(xf)
, x318_bezierB(CVector3f::Zero())
, x324_bezierC(CVector3f::Zero())
, x330_previousBezierC(CVector3f::Zero())
, x33c_homingTargetPoint(CVector3f::Zero())
, x348_targetPoint(x2e8_originalXf.GetTranslation() +
                   25.f * x2e8_originalXf.GetForward().AsNormalized())
, x354_busterSwoosh1(gpSimplePool->GetObj("BusterSwoosh1"))
, x360_busterSwoosh2(gpSimplePool->GetObj("BusterSwoosh2"))
, x36c_busterSparks(gpSimplePool->GetObj("BusterSparks"))
, x378_busterLight(gpSimplePool->GetObj("BusterLight"))
, x384_busterSwoosh1Gen(rs_new CParticleSwoosh(x354_busterSwoosh1, 0))
, x388_busterSwoosh2Gen(rs_new CParticleSwoosh(x360_busterSwoosh2, 0))
, x38c_busterSparksGen(rs_new CElementGen(x36c_busterSparks))
, x390_busterLightGen(rs_new CElementGen(x378_busterLight))
, x394_rand(99)
, x398_spiralOffset(2.f * M_PIF)
, x39c_(0.5f)
, x3a0_bezierBlend(0.5f)
, x3a4_sourceAngleRate(0.f)
, x3a8_sourceAngleTimer(0.f)
, x3ac_sourceAngle(0.f)
, x3b0_sourceRadius(0.f)
, x3b4_sourceRadiusRate(0.f)
, x3b8_sourceRadiusTimer(0.f)
, x3bc_targetAngleRate(0.f)
, x3c0_targetAngleTimer(0.f)
, x3c4_targetAngle(0.f)
, x3c8_innerSwooshColorT(0.f)
, x3cc_innerSwooshColorIdx(0)
, x3d0_24_firing(true)
, x3d0_25_seeking(true)
, x3d0_26_trackingTarget(false)
, x3d0_27_collided(false)
, x3d0_28_collidedWithWorld(true) {
  const rstl::vector< CParticleSwoosh::SSwooshData >& swooshes =
      x384_busterSwoosh1Gen->GetSwooshes();
  for (int i = 0; i < swooshes.size() - 1; ++i) {
    x384_busterSwoosh1Gen->SetWarmUp();
    x384_busterSwoosh1Gen->Update(0.0);
    x388_busterSwoosh2Gen->SetWarmUp();
    x388_busterSwoosh2Gen->Update(0.0);
  }
}

ENTITY_ACCEPT_IMPL(CWaveBuster)

void CWaveBuster::Touch(CActor& actor, CStateManager& mgr) { CActor::Touch(actor, mgr); }

void CWaveBuster::SetNewTarget(TUniqueId uid, CStateManager& mgr) {
  SetHomingTargetId(uid);
  if (uid != kInvalidUniqueId) {
    x3d0_26_trackingTarget = true;
    x3a0_bezierBlend = 0.f;
    CSfxManager::AddEmitter(0x6ff, GetTranslation(), CVector3f::Zero(), true, false,
                            CSfxManager::kMaxPriority, CSfxManager::kAllAreas);
    mgr.GetRumbleManager()->Rumble(mgr, kRFX_PlayerBump, 0.5f, kRP_Three);
  } else {
    x3d0_26_trackingTarget = false;
  }
}

float CWaveBuster::GetViewAngleToTarget(CVector3f& direction, const CActor& actor) const {
  direction = actor.GetTranslation() - x2e8_originalXf.GetTranslation();
  if (direction.CanBeNormalized()) {
    direction.Normalize();
  } else {
    direction = GetTransform().GetForward();
  }
  return CVector2f::GetAngleDiff(x2e8_originalXf.GetForward().DropZ(), direction.DropZ());
}

bool CWaveBuster::UpdateBeamFrame(CStateManager& mgr, float dt) {
  CVector3f direction = CVector3f::Forward();
  float viewAngle = 0.f;
  if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(GetHomingTargetId()))) {
    if (actor->GetHealthInfo(mgr) != nullptr && actor->GetHealthInfo(mgr)->GetHP() > 0.f) {
      const CVector3f delta = actor->GetTranslation() - x2e8_originalXf.GetTranslation();
      if (delta.MagSquared() > 10000.f) {
        return true;
      }
      viewAngle = GetViewAngleToTarget(direction, *actor);
    }
  }
  x3a8_sourceAngleTimer -= dt;
  if (x3a8_sourceAngleTimer <= 0.f) {
    const float angle = mgr.Random()->Range(0.f, 2.f * M_PIF);
    const float duration = mgr.Random()->Range(0.05f, 0.25f);
    x3a4_sourceAngleRate = (1.f / duration) * (angle - x3ac_sourceAngle);
    x3a8_sourceAngleTimer = duration;
  }
  x3b8_sourceRadiusTimer -= dt;
  if (x3b8_sourceRadiusTimer <= 0.f) {
    const float radius = mgr.Random()->Range(0.f, 0.5f);
    const float duration = mgr.Random()->Range(0.1f, 0.5f);
    x3b4_sourceRadiusRate = (1.f / duration) * (radius - x3b0_sourceRadius);
    x3b8_sourceRadiusTimer = duration;
  }
  x3c0_targetAngleTimer -= dt;
  if (x3c0_targetAngleTimer <= 0.f) {
    const float angle = mgr.Random()->Range(0.f, 2.f * M_PIF);
    const float duration = mgr.Random()->Range(0.05f, 0.25f);
    x3bc_targetAngleRate = (1.f / duration) * (angle - x3ac_sourceAngle);
    x3c0_targetAngleTimer = duration;
  }
  x3ac_sourceAngle += x3a4_sourceAngleRate * dt;
  x3b0_sourceRadius += x3b4_sourceRadiusRate * dt;
  x3c4_targetAngle += x3bc_targetAngleRate * dt;
  CVector3f sourceNode = kSourceNodePosition;
  sourceNode[kDZ] *= (GetHomingTargetId() != kInvalidUniqueId ? 1.25f : 1.f) -
                     x3b0_sourceRadius * x3b0_sourceRadius;
  x318_bezierB = x2e8_originalXf * CTransform4f::RotateY(CRelAngle::FromRadians(x3ac_sourceAngle)) *
                 sourceNode;
  return viewAngle > M_PIF / 2.f;
}

void CWaveBuster::UpdateTargetDamage(float dt, CStateManager& mgr) {
  if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(GetHomingTargetId()))) {
    if (actor->GetHealthInfo(mgr) != nullptr && actor->GetHealthInfo(mgr)->GetHP() > 0.f) {
      x33c_homingTargetPoint = actor->GetAimPosition(mgr, 0.f);
      SetTranslation(x33c_homingTargetPoint);
      const CMaterialFilter& filter = GetFilter();
      const CDamageInfo& damage = GetCurrentDamageInfo().MakeScaledForTime(dt);
      mgr.ApplyDamage(GetUniqueId(), GetHomingTargetId(), GetOwnerId(), damage, filter,
                      CVector3f::Zero());
      return;
    }
  }
  SetTransform(CTransform4f::LookAt(
      GetTranslation(), x348_targetPoint + 0.001f * x2e8_originalXf.GetForward().AsNormalized(),
      CVector3f::Up()));
  SetHomingTargetId(kInvalidUniqueId);
  x39c_ = 0.f;
  x3a0_bezierBlend = 0.f;
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
    x3d0_25_seeking = false;
    if (uid != kInvalidUniqueId && uid == GetHomingTargetId()) {
      x3d0_26_trackingTarget = true;
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
        x3d0_26_trackingTarget = false;
      } else if (x3d0_26_trackingTarget) {
        x3d0_25_seeking = false;
      }
    }
  } else if (!x3d0_26_trackingTarget) {
    const CVector3f delta = GetTranslation() - x2e8_originalXf.GetTranslation();
    if (delta.MagSquared() > 625.f) {
      x3d0_25_seeking = false;
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
  x3d0_27_collided = false;
  x3d0_28_collidedWithWorld = false;
  float beamDistance = 25.f;
  const CVector3f origin = x2e8_originalXf.GetTranslation();
  const CVector3f forward = x2e8_originalXf.GetForward().AsNormalized();
  if (!x3d0_25_seeking && !x3d0_26_trackingTarget) {
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
        x3d0_28_collidedWithWorld = true;
      }
      x3d0_27_collided = true;
    }
  }
  if (GetHomingTargetId() != kInvalidUniqueId && x3d0_26_trackingTarget) {
    UpdateTargetDamage(dt, mgr);
  } else {
    beamDistance = CMath::FastFSel(beamDistance - 1.f, beamDistance, 1.f);
    x348_targetPoint = origin + beamDistance * forward;
    if (x3d0_25_seeking) {
      UpdateTargetSeek(dt, mgr);
    } else {
      const float x = mgr.Random()->Range(-1.f, 1.f);
      const float z = mgr.Random()->Range(-1.f, 1.f);
      x348_targetPoint += CVector3f(x, 0.f, z);
      SetTranslation(x348_targetPoint);
    }
  }
  if (UpdateBeamFrame(mgr, dt)) {
    ResetBeam(true);
  }
  const CVector3f target =
      x2c0_homingTargetId != kInvalidUniqueId && x3d0_26_trackingTarget
          ? GetTransform() * CTransform4f::RotateY(CRelAngle::FromRadians(x3c4_targetAngle)) *
                kTargetNodePosition
          : GetTranslation() +
                kTargetNodePosition.GetZ() * GetTransform().GetForward().AsNormalized();
  if (x3a0_bezierBlend < 0.5f && GetHomingTargetId() != kInvalidUniqueId) {
    x324_bezierC = CVector3f::Lerp(x330_previousBezierC, target, x3a0_bezierBlend / 0.5f);
    x3a0_bezierBlend += 0.125f * dt;
  } else {
    x330_previousBezierC = x324_bezierC;
    x324_bezierC = target;
    x3a0_bezierBlend = 0.5f;
  }
  if (x2c8_projectileLight != kInvalidUniqueId) {
    x390_busterLightGen->Update(dt);
    if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(GetProjectileLightId()))) {
      light->SetTransform(GetTransform());
      light->SetTranslation(GetTranslation());
      if (!x390_busterLightGen.null() && x390_busterLightGen->SystemHasLight()) {
        light->SetLight(x390_busterLightGen->GetLight());
      }
    }
  }
  x3c8_innerSwooshColorT += 20.f * dt;
  if (x3c8_innerSwooshColorT > 1.f) {
    ++x3cc_innerSwooshColorIdx;
    if (x3cc_innerSwooshColorIdx > 2) {
      x3cc_innerSwooshColorIdx = 0;
    }
    x3c8_innerSwooshColorT = 0.f;
  }
  x38c_busterSparksGen->Update(dt);
}

void CWaveBuster::UpdateFx(const CTransform4f& xf, float dt, CStateManager& mgr) {
  if (GetActive()) {
    x2e8_originalXf = xf;
    x398_spiralOffset -= 60.f * dt;
    if (x398_spiralOffset < 0.f) {
      x398_spiralOffset = 2.f * M_PIF;
    }
    x170_projectile.SetVelocity(CVector3f(0.f, x3d0_25_seeking ? 1.6f : 0.f, 0.f));
  }
}

void CWaveBuster::AddToRenderer(const CFrustumPlanes& planes, const CStateManager& mgr) const {
  EnsureRendered(mgr, x2e8_originalXf.GetTranslation(), GetSortingBounds(mgr));
}

void CWaveBuster::Render(const CStateManager& mgr) const {
  RenderSwooshes();
  RenderElectricSpiral();
  CWeapon::Render(mgr);
}

void CWaveBuster::ResetBeam(bool deactivate) {
  if (deactivate) {
    SetActive(false);
    x3d0_24_firing = false;
    x38c_busterSparksGen->SetParticleEmission(false);
    x398_spiralOffset = 2.f * M_PIF;
  } else {
    x38c_busterSparksGen->SetParticleEmission(false);
    x3d0_24_firing = false;
  }
}

void CWaveBuster::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  switch (msg) {
  case kSM_Registered:
    if (!x390_busterLightGen.null() && x390_busterLightGen->SystemHasLight()) {
      CreateProjectileLight(rstl::string_l("WaveBuster_Light"), x390_busterLightGen->GetLight(),
                            mgr);
    }
    SetThermalFlags(kTF_Hot);
    x318_bezierB = x2e8_originalXf.GetTranslation();
    x324_bezierC = GetTranslation();
    x330_previousBezierC = x324_bezierC;
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
  const CTransform4f inverse = x2e8_originalXf.GetInverse();
  const CVector3f a = inverse * x2e8_originalXf.GetTranslation();
  const CVector3f b = inverse * x318_bezierB;
  const CVector3f c = inverse * x324_bezierC;
  const CVector3f d = inverse * GetTranslation();
  float radius = 0.f;
  CVector3f* vertices = reinterpret_cast< CVector3f* >(LCGetBase());
  CVector3f previous = a;
  for (float t = 0.16f; t <= 1.f; t += 0.16f) {
    const CVector3f point = CMath::GetBezierPoint(a, b, c, d, t);
    float angle = 0.f;
    for (int i = 0; i < 36; ++i) {
      const float randX = x394_rand.Range(-0.041667f, 0.041667f);
      const float randZ = x394_rand.Range(-0.041667f, 0.041667f);
      const float x = radius * CMath::FastCosR(angle + x398_spiralOffset) + randX;
      const float z = radius * CMath::FastSinR(angle + x398_spiralOffset) + randZ;
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
  gpRender->SetModelMatrix(x2e8_originalXf);
  DrawLineList(reinterpret_cast< CVector3f* >(LCGetBase()), CColor::White(), 216, 12);
  DrawLineList(reinterpret_cast< CVector3f* >(LCGetBase()), CColor(1.f, 0.f, 1.f, 0.5f), 216, 48);
  CGX::SetLineWidth(6, GX_TO_ZERO);
}

void CWaveBuster::RenderSwooshes() const {
  static const uint colors[] = {0xffffffff, 0xff00ffff, 0xff0000ff, 0x0000ffff};
  const CTransform4f rotation = x2e8_originalXf.GetRotation();
  const CVector3f origin = x2e8_originalXf.GetTranslation();
  x38c_busterSparksGen->SetParticleEmission(true);
  const CColor color(CColor::Lerp(colors[x3cc_innerSwooshColorIdx],
                                  colors[x3cc_innerSwooshColorIdx + 1], x3c8_innerSwooshColorT));
  rstl::vector< CParticleSwoosh::SSwooshData >& swooshes1 = x384_busterSwoosh1Gen->Swooshes();
  rstl::vector< CParticleSwoosh::SSwooshData >& swooshes2 = x388_busterSwoosh2Gen->Swooshes();
  float t = 0.f;
  float previousRot1 = swooshes1[swooshes1.size() - 1].mInitialRot;
  float previousRot2 = swooshes2[swooshes2.size() - 1].mInitialRot;
  for (int i = 0; i < swooshes1.size(); ++i) {
    CParticleSwoosh::SSwooshData& swoosh1 = swooshes1[i];
    CParticleSwoosh::SSwooshData& swoosh2 = swooshes2[i];
    const CVector3f point =
        CMath::GetBezierPoint(GetTranslation(), x324_bezierC, x318_bezierB, origin, t);
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
    x38c_busterSparksGen->SetTranslation(point);
    x38c_busterSparksGen->ForceParticleCreation(1);
    t += 0.04f;
  }
  x38c_busterSparksGen->SetParticleEmission(false);
  x384_busterSwoosh1Gen->Render();
  x388_busterSwoosh2Gen->Render();
  x38c_busterSparksGen->Render();
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
                         .GetTransformedAABox(x2e8_originalXf);
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
  if (x3d0_28_collidedWithWorld) {
    return rstl::optional_object_null();
  }
  return GetProjectileBounds();
}

CWaveBuster::~CWaveBuster() {}
