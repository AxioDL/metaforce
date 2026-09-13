#include "MetroidPrime/ScriptObjects/CFishCloud.hpp"
#include "MetroidPrime/ScriptObjects/CFishCloudModifier.hpp"

#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/Animation/CVertexMorphEffect.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CTri.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "rstl/algorithm.hpp"

#include <float.h>

CFishCloudModifier::CFishCloudModifier(TUniqueId uid, const bool active, const rstl::string& name,
                                       const CEntityInfo& info, const CVector3f& pos,
                                       bool isRepulsor, bool swirl, float radius, float priority)
: CActor(uid, active, name, info, CTransform4f::Translate(pos), CModelData::CModelDataNull(),
         CMaterialList(kMT_NoStepLogic), CActorParameters::None(), kInvalidUniqueId)
, xe8_radius(radius)
, xec_priority(priority)
, xf0_isRepulsor(isRepulsor)
, xf1_swirl(swirl) {}

ENTITY_ACCEPT_IMPL(CFishCloudModifier)

void CFishCloudModifier::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender,
                                         CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, sender, mgr);
  switch (msg) {
  case kSM_Activate:
  case kSM_InitializedInArea:
    if (GetActive()) {
      AddSelf(mgr);
    }
    break;
  case kSM_Deactivate:
  case kSM_Deleted:
    RemoveSelf(mgr);
    break;
  }
}

void CFishCloudModifier::AddSelf(CStateManager& mgr) {
  const rstl::vector< SConnection >& connections = GetConnectionList();
  for (AUTO(it, connections.begin()); it != connections.end(); ++it) {
    if (it->x0_state != kSS_Modify || it->x4_msg != kSM_Follow) {
      continue;
    }
    const TUniqueId uid = mgr.GetIdForScript(it->x8_objId);
    if (uid == kInvalidUniqueId) {
      continue;
    }
    if (CFishCloud* cloud = TCastToPtr< CFishCloud >(mgr.ObjectById(uid))) {
      if (xf0_isRepulsor) {
        cloud->AddRepulsor(GetUniqueId(), xf1_swirl, xe8_radius, xec_priority);
      } else {
        cloud->AddAttractor(GetUniqueId(), xf1_swirl, xe8_radius, xec_priority);
      }
    }
  }
}

void CFishCloudModifier::RemoveSelf(CStateManager& mgr) {
  const rstl::vector< SConnection >& connections = GetConnectionList();
  for (AUTO(it, connections.begin()); it != connections.end(); ++it) {
    if (it->x0_state != kSS_Modify || it->x4_msg != kSM_Follow) {
      continue;
    }
    const TUniqueId uid = mgr.GetIdForScript(it->x8_objId);
    if (uid == kInvalidUniqueId) {
      continue;
    }
    if (CFishCloud* cloud = TCastToPtr< CFishCloud >(mgr.ObjectById(uid))) {
      if (xf0_isRepulsor) {
        cloud->RemoveRepulsor(GetUniqueId());
      } else {
        cloud->RemoveAttractor(GetUniqueId());
      }
    }
  }
}

CFishCloud::CModifierSource::CModifierSource(const TUniqueId& source, bool repulsor, bool swirl,
                                             float radius, float priority)
: x0_source(source)
, x4_radius(radius)
, x8_priority(priority)
, xc_isRepulsor(repulsor)
, xd_isSwirl(swirl) {}

bool CFishCloud::CModifierSource::operator<(const CModifierSource& other) const {
  if (x0_source == other.x0_source) {
    return xc_isRepulsor < other.xc_isRepulsor;
  }
  return x0_source < other.x0_source;
}

CFishCloud::CBoid::CBoid(const CVector3f& pos, const CVector3f& vel, float scale)
: x0_pos(pos), xc_vel(vel), x18_scale(scale), x1c_next(nullptr), x20_active(true) {}

CFishCloud::CFishCloud(TUniqueId uid, const bool active, const rstl::string& name,
                       const CEntityInfo& info, const CVector3f& scale, const CTransform4f& xf,
                       const CModelData& mData, const CAnimRes& aRes, int numBoids, float speed,
                       float separationRadius, float cohesionMagnitude, float alignmentWeight,
                       float separationMagnitude, float weaponRepelMagnitude,
                       float playerRepelMagnitude, float containmentMagnitude, float scatterVel,
                       float maxScatterAngle, float weaponRepelDampingSpeed,
                       float playerRepelDampingSpeed, float containmentRadius, int updateShift,
                       const CColor& color, bool killable, float weaponKillRadius, CAssetId part1,
                       int partCount1, CAssetId part2, int partCount2, CAssetId part3,
                       int partCount3, CAssetId part4, int partCount4, int deathSfx,
                       bool repelFromThreats, bool hotInThermal)
: CActor(uid, active, name, info, xf, mData, CMaterialList(kMT_NoStepLogic),
         CActorParameters::None().HotInThermal(hotInThermal), kInvalidUniqueId)
, x11c_updateMask((1 << updateShift) - 1)
, x120_scale(scale)
, x12c_randomMovementTimer(0.f)
, x130_speed(speed)
, x134_numBoids(numBoids)
, x138_separationRadius(separationRadius)
, x13c_cohesionMagnitude(cohesionMagnitude)
, x140_alignmentWeight(alignmentWeight)
, x144_separationMagnitude(separationMagnitude)
, x148_weaponRepelMagnitude(weaponRepelMagnitude)
, x14c_playerRepelMagnitude(playerRepelMagnitude)
, x150_scatterVel(scatterVel)
, x154_maxScatterAngle(maxScatterAngle)
, x158_containmentMagnitude(containmentMagnitude)
, x15c_playerRepelDampingSpeed(playerRepelDampingSpeed)
, x160_weaponRepelDampingSpeed(weaponRepelDampingSpeed)
, x164_playerRepelDamping(playerRepelDampingSpeed)
, x168_weaponRepelDamping(weaponRepelDampingSpeed)
, x16c_color(color)
, x170_weaponKillRadius(weaponKillRadius)
, x174_containmentRadius(containmentRadius)
, x234_deathSfx(
      CSfxManager::TranslateSFXID(deathSfx == -1 ? CSfxManager::kInternalInvalidSfxId : deathSfx))
, x238_partitionPitch(CVector3f::Zero())
, x244_ooPartitionPitch(CVector3f::Zero())
, x250_24_randomMovement(false)
, x250_25_worldSpace(false)
, x250_26_enableWeaponRepelDamping(false)
, x250_27_validModel(false)
, x250_28_killable(killable)
, x250_29_repelFromThreats(repelFromThreats)
, x250_30_enablePlayerRepelDamping(false)
, x250_31_updateWithoutPartitions(false) {
  x108_modifierSources.reserve(10);
  const CVector3f& forward = GetTransform().GetForward();
  const CVector3f& up = GetTransform().GetUp();
  const CVector3f& right = GetTransform().GetRight();
  x250_25_worldSpace = !(close_enough(right.GetX(), 1.f) && close_enough(right.GetX(), 0.f) &&
                         close_enough(right.GetX(), 0.f) && close_enough(forward.GetX(), 0.f) &&
                         close_enough(forward.GetX(), 1.f) && close_enough(forward.GetX(), 0.f) &&
                         close_enough(up.GetX(), 0.f) && close_enough(up.GetX(), 0.f) &&
                         close_enough(up.GetX(), 1.f));
  if (aRes.GetId() != kInvalidAssetId) {
    x1b0_models.push_back(rs_new CModelData(aRes));
    x1b0_models.push_back(rs_new CModelData(aRes));
    x1b0_models.push_back(rs_new CModelData(aRes));
    x1b0_models.push_back(rs_new CModelData(aRes));
    x250_27_validModel = true;
  }
  if (part1 != kInvalidAssetId) {
    x1c4_particleDescs.push_back(gpSimplePool->GetObj(SObjectTag('PART', part1)));
  }
  if (part2 != kInvalidAssetId) {
    x1c4_particleDescs.push_back(gpSimplePool->GetObj(SObjectTag('PART', part2)));
  }
  if (part3 != kInvalidAssetId) {
    x1c4_particleDescs.push_back(gpSimplePool->GetObj(SObjectTag('PART', part3)));
  }
  if (part4 != kInvalidAssetId) {
    x1c4_particleDescs.push_back(gpSimplePool->GetObj(SObjectTag('PART', part4)));
  }
  for (int i = 0; i < x1c4_particleDescs.size(); ++i) {
    x1f8_particleGens.push_back(rs_new CElementGen(x1c4_particleDescs[i]));
    x1f8_particleGens[i]->SetParticleEmission(false);
  }
  x21c_deathParticleCounts.push_back(partCount1);
  x21c_deathParticleCounts.push_back(partCount2);
  x21c_deathParticleCounts.push_back(partCount3);
  x21c_deathParticleCounts.push_back(partCount4);
  const CAABox& aabb = GetBoundingBox();
  x238_partitionPitch = (aabb.GetMaxPoint() - aabb.GetMinPoint()) / 7.f;
  x244_ooPartitionPitch =
      CVector3f(1.f / x238_partitionPitch.GetX(), 1.f / x238_partitionPitch.GetY(),
                1.f / x238_partitionPitch.GetZ());
}

void CFishCloud::InitAnimBoids(CStateManager& mgr, CModelData::EWhichModel which) {
  x178_posWorkspaces.clear();
  x19c_nrmWorkspaces.clear();
  for (int i = 0; i < 4; ++i) {
    float* normals;
    x178_posWorkspaces.push_back(
        x1b0_models[i]->PickAnimatedModel(which).AllocateNewWorkspace(&normals));
    x19c_nrmWorkspaces.push_back(normals);
    x1b0_models[i]->EnableLooping(true);
    x1b0_models[i]->AdvanceAnimation(
        (float(i) / 4.f) *
            x1b0_models[i]->GetAnimationData()->GetAnimTimeRemaining(rstl::string_l("Whole Body")),
        mgr, GetCurrentAreaId(), true);
  }
  x230_whichModel = which;
}

ENTITY_ACCEPT_IMPL(CFishCloud)

void CFishCloud::CalculateRenderBounds() {
  const CAABox aabb = GetBoundingBox();
  SetRenderBounds(aabb);
}

CAABox CFishCloud::GetBoundingBox() const {
  const CAABox aabb = GetUntransformedBoundingBox();
  return aabb.GetTransformedAABox(GetTransform());
}

CAABox CFishCloud::GetUntransformedBoundingBox() const {
  const CVector3f extent(0.75f * x120_scale.GetX(), 0.75f * x120_scale.GetY(),
                         0.75f * x120_scale.GetZ());
  return CAABox(-extent, extent);
}

bool CFishCloud::PointInBox(const CAABox& aabb, const CVector3f& point) const {
  if (!x250_25_worldSpace) {
    return aabb.PointInside(point);
  }
  const CVector3f localPoint = GetTransform().TransposeRotate(point - GetTranslation());
  return GetUntransformedBoundingBox().PointInside(localPoint);
}

CPlane CFishCloud::FindClosestPlane(const CAABox& aabb, const CVector3f& point) const {
  if (!x250_25_worldSpace) {
    float minDistance = FLT_MAX;
    CAABox::EBoxFaceId minFace = CAABox::kF_YMin;
    for (int i = 0; i < 6; ++i) {
      const CTri tri = aabb.GetTri(static_cast< CAABox::EBoxFaceId >(i), 0);
      const CPlane plane(tri.GetPointA(), tri.GetPointC(), tri.GetPointB());
      const float distance = plane.GetHeight(point);
      if (distance >= 0.f && distance < minDistance) {
        minFace = static_cast< CAABox::EBoxFaceId >(i);
        minDistance = distance;
      }
    }
    const CTri tri = aabb.GetTri(minFace, 0);
    return CPlane(tri.GetPointA(), tri.GetPointC(), tri.GetPointB());
  }
  const CTransform4f& xf = GetTransform();
  const CVector3f localPoint = xf.TransposeRotate(point - GetTranslation());
  const CAABox localBounds = GetUntransformedBoundingBox();
  float minDistance = FLT_MAX;
  CAABox::EBoxFaceId minFace = CAABox::kF_YMin;
  for (int i = 0; i < 6; ++i) {
    const CTri tri = localBounds.GetTri(static_cast< CAABox::EBoxFaceId >(i), 0);
    const CPlane plane(tri.GetPointA(), tri.GetPointC(), tri.GetPointB());
    const float distance = plane.GetHeight(localPoint);
    if (distance >= 0.f && distance < minDistance) {
      minFace = static_cast< CAABox::EBoxFaceId >(i);
      minDistance = distance;
    }
  }
  const CTri tri = localBounds.GetTri(minFace, 0);
  return CPlane(xf * tri.GetPointA(), xf * tri.GetPointC(), xf * tri.GetPointB());
}

void CFishCloud::PlaceBoid(CStateManager& mgr, CBoid& boid, const CAABox& aabb) {
  CRandom16& random = *mgr.Random();
  const CPlane plane = FindClosestPlane(aabb, boid.x0_pos);
  boid.x0_pos -= plane.GetHeight(boid.x0_pos) * plane.GetNormal() + 0.0001f * plane.GetNormal();
  boid.xc_vel = CVector3f(random.Float() - 0.5f, random.Float() - 0.5f, 0.f);
  if (!x250_25_worldSpace) {
    if (!aabb.PointInside(boid.x0_pos)) {
      const CVector3f min = aabb.GetMinPoint();
      boid.x0_pos = CVector3f(random.Float() * aabb.GetWidth() + min.GetX(),
                              random.Float() * aabb.GetHeight() + min.GetY(),
                              random.Float() * aabb.GetDepth() + min.GetZ());
    }
  } else if (!PointInBox(aabb, boid.x0_pos)) {
    const CAABox localBounds = GetUntransformedBoundingBox();
    const CVector3f min = localBounds.GetMinPoint();
    const CVector3f pos(random.Float() * localBounds.GetWidth() + min.GetX(),
                        random.Float() * localBounds.GetHeight() + min.GetY(),
                        random.Float() * localBounds.GetDepth() + min.GetZ());
    boid.x0_pos = GetTransform() * pos;
  }
}

void CFishCloud::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, sender, mgr);
  switch (msg) {
  case kSM_Activate:
  case kSM_Deactivate:
  case kSM_Deleted:
    break;
  case kSM_Registered: {
    xe8_boids.reserve(x134_numBoids);
    const CAABox bounds = GetUntransformedBoundingBox();
    CRandom16& random = *mgr.Random();
    const CVector3f& min = bounds.GetMinPoint();
    for (int i = 0; i < xe8_boids.capacity(); ++i) {
      const CVector3f pos(random.Float() * bounds.GetWidth() + min[kDX],
                          random.Float() * bounds.GetHeight() + min[kDY],
                          random.Float() * bounds.GetDepth() + min[kDZ]);
      xe8_boids.push_back(CBoid(GetTransform() * pos,
                                CVector3f(random.Float() - 0.5f, random.Float() - 0.5f, 0.f),
                                0.2f * CMath::PowF(random.Float(), 7.f) + 0.9f));
    }
    CreatePartitionList();
    if (x250_27_validModel) {
      InitAnimBoids(mgr, CModelData::kWM_Normal);
    }
    break;
  }
  default:
    break;
  }
}

bool CFishCloud::AddAttractor(TUniqueId source, bool swirl, float radius, float priority) {
  const CModifierSource modifier(source, false, swirl, radius, priority);
  AUTO(it, rstl::binary_find(x108_modifierSources.begin(), x108_modifierSources.end(), modifier));
  if (it != x108_modifierSources.end()) {
    it->SetAffectRadius(radius);
    it->SetAffectPriority(priority);
    return true;
  }
  if (x108_modifierSources.size() < x108_modifierSources.capacity()) {
    AUTO(insertIt,
         rstl::lower_bound(x108_modifierSources.begin(), x108_modifierSources.end(), modifier));
    x108_modifierSources.insert(insertIt, modifier);
    return true;
  }
  return false;
}

bool CFishCloud::AddRepulsor(TUniqueId source, bool swirl, float radius, float priority) {
  const CModifierSource modifier(source, true, swirl, radius, priority);
  AUTO(it, rstl::binary_find(x108_modifierSources.begin(), x108_modifierSources.end(), modifier));
  if (it != x108_modifierSources.end()) {
    it->SetAffectRadius(radius);
    it->SetAffectPriority(priority);
    return true;
  }
  if (x108_modifierSources.size() < x108_modifierSources.capacity()) {
    AUTO(insertIt,
         rstl::lower_bound(x108_modifierSources.begin(), x108_modifierSources.end(), modifier));
    x108_modifierSources.insert(insertIt, modifier);
    return true;
  }
  return false;
}

void CFishCloud::RemoveAttractor(TUniqueId source) {
  const CModifierSource modifier(source, false, false, 0.f, 0.f);
  AUTO(it, rstl::binary_find(x108_modifierSources.begin(), x108_modifierSources.end(), modifier));
  if (it != x108_modifierSources.end()) {
    x108_modifierSources.erase(it);
  }
}

void CFishCloud::RemoveRepulsor(TUniqueId source) {
  const CModifierSource modifier(source, true, false, 0.f, 0.f);
  AUTO(it, rstl::binary_find(x108_modifierSources.begin(), x108_modifierSources.end(), modifier));
  if (it != x108_modifierSources.end()) {
    x108_modifierSources.erase(it);
  }
}

rstl::optional_object< CAABox > CFishCloud::GetTouchBounds() const { return GetBoundingBox(); }

void CFishCloud::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  const CGameArea& area = mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId());
  if (area.GetOcclusionState() != CGameArea::kOS_Visible) {
    return;
  }
  x168_weaponRepelDamping =
      rstl::max_val(0.f, x168_weaponRepelDamping - x160_weaponRepelDampingSpeed * dt * 0.1f);
  if (x250_26_enableWeaponRepelDamping) {
    x168_weaponRepelDamping = rstl::min_val(
        x148_weaponRepelMagnitude, x160_weaponRepelDampingSpeed * dt + x168_weaponRepelDamping);
  }
  x164_playerRepelDamping =
      rstl::max_val(0.f, x164_playerRepelDamping - x15c_playerRepelDampingSpeed * dt * 0.1f);
  if (x250_30_enablePlayerRepelDamping) {
    x164_playerRepelDamping = rstl::min_val(
        x14c_playerRepelMagnitude, x15c_playerRepelDampingSpeed * dt + x164_playerRepelDamping);
  }
  x250_26_enableWeaponRepelDamping = false;
  x250_30_enablePlayerRepelDamping = false;
  ++x118_thinkCounter;
  UpdateParticles(dt);
  rstl::reserved_vector< CBoid*, 25 > nearList;
  UpdatePartitionList();
  CRandom16& random = *mgr.Random();
  const CAABox bounds = GetBoundingBox();
  int index = 0;
  for (AUTO(it, xe8_boids.begin()); it != xe8_boids.end(); ++it, ++index) {
    if (it->x20_active && (index & x11c_updateMask) == (x118_thinkCounter & x11c_updateMask)) {
      nearList.clear();
      if (x250_31_updateWithoutPartitions) {
        OldBuildBoidNearList(it->x0_pos, x138_separationRadius, nearList);
      } else {
        BuildBoidNearList(it->x0_pos, x138_separationRadius, nearList);
      }
      for (int i = 0; i != 5; ++i) {
        switch (i) {
        case 1:
          ApplySeparation(*it, nearList);
          break;
        case 2:
          if (!x250_24_randomMovement || random.Float() > x12c_randomMovementTimer) {
            ApplyCohesion(*it, nearList);
          }
          break;
        case 3:
          if (!x250_24_randomMovement || random.Float() > x12c_randomMovementTimer) {
            ApplyAlignment(*it, nearList);
          }
          break;
        case 4:
          ApplyWander(mgr, *it);
          break;
        }
        if (it->xc_vel.MagSquared() > 3.2f) {
          break;
        }
      }
      if (!x250_24_randomMovement && it->xc_vel.MagSquared() < 3.2f) {
        for (AUTO(mod, x108_modifierSources.begin()); mod != x108_modifierSources.end(); ++mod) {
          if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(mod->GetSource()))) {
            if (mod->IsSwirl()) {
              ApplyRotation(*it, mod->GetAffectPriority(), actor->GetTranslation(),
                            mod->GetAffectRadius(), mod->IsRepulsor());
            } else if (mod->IsRepulsor()) {
              ApplyRepulsion(*it, actor->GetTranslation(), mod->GetAffectRadius(),
                             mod->GetAffectPriority());
            } else {
              ApplyAttraction(*it, actor->GetTranslation(), mod->GetAffectRadius(),
                              mod->GetAffectPriority());
            }
          } else {
            if (mod->IsRepulsor()) {
              RemoveRepulsor(mod->GetSource());
            } else {
              RemoveAttractor(mod->GetSource());
            }
            break;
          }
        }
      }
    }
  }
  for (AUTO(it, xe8_boids.begin()); it != xe8_boids.end(); ++it) {
    if (it->x20_active) {
      ApplyContainment(*it, bounds);
      CVector3f& velocity = it->xc_vel;
      const float speed = velocity.Magnitude();
      if (!close_enough(speed, 0.f)) {
        const float inverseSpeed = 1.f / speed;
        velocity *= inverseSpeed;
      }
      velocity.SetZ(0.99f * velocity.GetZ());
    }
  }
  if (x12c_randomMovementTimer > 0.f) {
    x12c_randomMovementTimer -= dt;
  } else {
    x12c_randomMovementTimer = 0.f;
    x250_24_randomMovement = false;
  }
  for (AUTO(it, xe8_boids.begin()); it != xe8_boids.end(); ++it) {
    if (it->x20_active) {
      it->x0_pos += x130_speed * (dt * it->xc_vel);
      if (!PointInBox(bounds, it->x0_pos)) {
        PlaceBoid(mgr, *it, bounds);
      }
    }
  }
  if (x250_27_validModel) {
    for (int i = 0; i < 4; ++i) {
      x1b0_models[i]->AnimationData()->SetPlaybackRate(1.f);
      x1b0_models[i]->AdvanceAnimation(dt, mgr, GetCurrentAreaId(), true);
    }
  }
}

void CFishCloud::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  CActor::PreRender(mgr, frustum);
  if (x250_27_validModel) {
    for (int i = 0; i < 4; ++i) {
      x1b0_models[i]->AnimationData()->PreRender();
    }
  }
  SetPreRenderClipped(false);
}

void CFishCloud::RenderBoid(int idx, const CBoid& boid, uint& drawMask, const bool thermalHot,
                            const CModelFlags& flags) const {
  const uint modelIndex = idx & 3;
  uint mask = drawMask;
  CModelData& modelData = *x1b0_models[modelIndex];
  CAnimData& animData = *modelData.AnimationData();
  CSkinnedModel& model = modelData.PickAnimatedModel(CModelData::kWM_Normal);
  const uint bit = 1 << modelIndex;
  if (mask & bit) {
    mask &= ~bit;
    animData.BuildPose();
    model.Calculate(animData.GetPose(), rstl::optional_object< CVertexMorphEffect >(), nullptr,
                    x178_posWorkspaces[modelIndex].get());
  }
  gpRender->SetModelMatrix(CTransform4f::LookAt(boid.x0_pos, boid.x0_pos + boid.xc_vel));
  const float* const positions = x178_posWorkspaces[modelIndex].get();
  const float* const normals = x19c_nrmWorkspaces[modelIndex];
  if (thermalHot) {
    const CModelFlags thermalFlags = CModelFlags::Normal().DepthCompareUpdate(true, false);
    CModelData::ThermalDraw(model, positions, normals, CColor(0xffffffff),
                            CColor(static_cast< uchar >(0), static_cast< uchar >(0),
                                   static_cast< uchar >(0), static_cast< uchar >(64)),
                            thermalFlags);
  } else {
    model.Draw(positions, normals, flags);
  }
  drawMask = mask;
}

void CFishCloud::Render(const CStateManager& mgr) const {
  if (!GetActive()) {
    return;
  }
  const bool thermalHot = mgr.GetThermalDrawFlag() == kTD_Hot;
  CModelFlags flags = CModelFlags::Normal();
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_XRay) {
    CColor color = x16c_color;
    color.SetAlpha(static_cast< uchar >(76));
    flags = CModelFlags(CModelFlags::kT_Blend, color);
  } else {
    flags = CModelFlags(CModelFlags::kT_One, x16c_color);
  }
  RenderParticles();
  if (x250_27_validModel) {
    gpRender->SetAmbientColor(CColor::White());
    CGraphics::DisableAllLights();
    uint drawMask = ~0;
    int index = 0;
    for (AUTO(it, xe8_boids.begin()); it != xe8_boids.end(); ++it, ++index) {
      if (it->x20_active) {
        RenderBoid(index, *it, drawMask, thermalHot, flags);
      }
    }
  } else {
    CModelData& modelData = const_cast< CModelData& >(*GetModelData());
    gpRender->SetAmbientColor(CColor::White());
    CGraphics::DisableAllLights();
    gpRender->SetModelMatrix(CTransform4f::Identity());
    for (AUTO(it, xe8_boids.begin()); it != xe8_boids.end(); ++it) {
      if (it->x20_active) {
        modelData.SetScale(CVector3f(it->x18_scale, it->x18_scale, it->x18_scale));
        modelData.Render(mgr, CTransform4f::LookAt(it->x0_pos, it->x0_pos + it->xc_vel), nullptr,
                         flags);
      }
    }
  }
}

void CFishCloud::KillBoid(CBoid& boid) {
  boid.x20_active = false;
  AddParticles(boid.x0_pos);
  const int areaId = GetCurrentAreaId().Value();
  CAudioSys::C3DEmitterParmData parms(250.f, 0.1f, 1, 127, 20);
  parms.x0_pos = boid.x0_pos;
  parms.xc_dir = CVector3f::Up();
  parms.x24_sfxId = x234_deathSfx;
  CSfxManager::AddEmitter(parms, true, CSfxManager::kMedPriority, false, areaId);
}

void CFishCloud::Touch(CActor& other, CStateManager& mgr) {
  CActor::Touch(other, mgr);
  if (CWeapon* weapon = TCastToPtr< CWeapon >(other)) {
    if (!x250_26_enableWeaponRepelDamping && x250_29_repelFromThreats) {
      int index = 0;
      for (AUTO(it, xe8_boids.begin()); it != xe8_boids.end(); ++it, ++index) {
        if ((index & 3) == (x118_thinkCounter & 3)) {
          ApplyRepulsion(*it, weapon->GetTranslation(), 8.f,
                         x148_weaponRepelMagnitude - x168_weaponRepelDamping);
        }
      }
    }
    x250_26_enableWeaponRepelDamping = true;
    if (x250_28_killable) {
      const rstl::optional_object< CAABox > touchBounds = weapon->GetTouchBounds();
      if (touchBounds.valid()) {
        const CAABox bounds = *touchBounds;
        const float radius = x170_weaponKillRadius;
        for (AUTO(it, xe8_boids.begin()); it != xe8_boids.end(); ++it) {
          if (it->x20_active) {
            const CVector3f extent(radius, radius, radius);
            const CAABox boidBounds(it->x0_pos - extent, it->x0_pos + extent);
            if (bounds.DoBoundsOverlap(boidBounds)) {
              KillBoid(*it);
            }
          }
        }
      }
    }
  }
  if (x250_29_repelFromThreats) {
    if (CPlayer* player = TCastToPtr< CPlayer >(other)) {
      CRandom16& random = *mgr.Random();
      const CVector3f playerPos = player->GetTranslation();
      for (AUTO(it, xe8_boids.begin()); it != xe8_boids.end(); ++it) {
        CVector3f adjustedPos = playerPos;
        const CVector3f delta = it->GetTranslation() - adjustedPos;
        const float dz = delta.GetZ();
        if (dz > 0.f && dz < 2.3) {
          adjustedPos.SetZ(it->GetTranslation().GetZ());
        }
        adjustedPos[kDX] += 0.2f * random.Float() - 0.1f;
        adjustedPos[kDY] += 0.2f * random.Float() - 0.1f;
        ApplyRepulsion(*it, adjustedPos, 8.f, x14c_playerRepelMagnitude - x164_playerRepelDamping);
      }
    }
    x250_30_enablePlayerRepelDamping = true;
  }
}

void CFishCloud::CreatePartitionList() {
  const CAABox bounds = GetBoundingBox();
  xf8_boidPartitionLists.reserve(343);
}

void CFishCloud::UpdatePartitionList() {
  xf8_boidPartitionLists.clear();
  for (int i = 0; i < xf8_boidPartitionLists.capacity(); ++i) {
    xf8_boidPartitionLists.push_back(nullptr);
  }
  const CAABox bounds = GetBoundingBox();
  const CVector3f& min = bounds.GetMinPoint();
  for (AUTO(it, xe8_boids.begin()); it != xe8_boids.end(); ++it) {
    const CVector3f indices =
        CVector3f::ByElementMultiply(x244_ooPartitionPitch, it->GetTranslation() - min);
    const int index = int(indices.GetX()) + int(indices.GetY()) * 7 + int(indices.GetZ()) * 49;
    if (index >= 0 && index < 343) {
      it->x1c_next = xf8_boidPartitionLists[index];
      xf8_boidPartitionLists[index] = &*it;
    }
  }
}

CFishCloud::CBoid* CFishCloud::GetListAt(const CVector3f& pos) {
  const CAABox bounds = GetBoundingBox();
  const CVector3f indices =
      CVector3f::ByElementMultiply(x244_ooPartitionPitch, pos - bounds.GetMinPoint());
  const int index = int(indices.GetX()) + int(indices.GetY()) * 7 + int(indices.GetZ()) * 49;
  if (index < 0 || index >= 343) {
    return nullptr;
  }
  return xf8_boidPartitionLists[index];
}

void CFishCloud::BuildBoidNearList(const CVector3f& pos, float radius,
                                   rstl::reserved_vector< CBoid*, 25 >& nearList) {
  const float radiusSquared = radius * radius;
  const CAABox bounds = GetBoundingBox();
  const CVector3f& min = bounds.GetMinPoint();
  const CVector3f& max = bounds.GetMaxPoint();
  const float x = rstl::max_val(radius * x244_ooPartitionPitch.GetX(), x238_partitionPitch.GetX());
  const float y = rstl::max_val(radius * x244_ooPartitionPitch.GetY(), x238_partitionPitch.GetY());
  const float z = rstl::max_val(radius * x244_ooPartitionPitch.GetZ(), x238_partitionPitch.GetZ());
  int remaining = 25;
  for (float ox = 0.01f - x; ox < x; ox += x238_partitionPitch.GetX()) {
    const float px = ox + pos.GetX();
    if (px < min.GetX()) {
      continue;
    }
    if (px >= max.GetX()) {
      break;
    }
    for (float oy = 0.01f - y; oy < y; oy += x238_partitionPitch.GetY()) {
      const float py = oy + pos.GetY();
      if (py < min.GetY()) {
        continue;
      }
      if (py >= max.GetY()) {
        break;
      }
      for (float oz = 0.01f - z; oz < z; oz += x238_partitionPitch.GetZ()) {
        const float pz = oz + pos.GetZ();
        if (pz < min.GetZ()) {
          continue;
        }
        if (pz >= max.GetZ()) {
          break;
        }
        const CVector3f indices =
            CVector3f::ByElementMultiply(x244_ooPartitionPitch, CVector3f(px, py, pz) - min);
        const int index = int(indices.GetX()) + int(indices.GetY()) * 7 + int(indices.GetZ()) * 49;
        if (index < 0) {
          continue;
        }
        if (index >= 343) {
          break;
        }
        for (CBoid* boid = xf8_boidPartitionLists[index]; boid != nullptr; boid = boid->x1c_next) {
          if (boid->x20_active) {
            const CVector3f delta = boid->x0_pos - pos;
            const float distanceSquared = delta.MagSquared();
            if (distanceSquared != 0.f && distanceSquared < radiusSquared) {
              nearList.push_back(boid);
              if (--remaining == 0) {
                return;
              }
            }
          }
        }
      }
    }
  }
}

void CFishCloud::OldBuildBoidNearList(const CVector3f& pos, float radius,
                                      rstl::reserved_vector< CBoid*, 25 >& nearList) {
  const float radiusSquared = radius * radius;
  int remaining = 25;
  for (CBoid* boid = GetListAt(pos); boid != nullptr && remaining != 0; boid = boid->x1c_next) {
    if (boid->x20_active) {
      const CVector3f delta = boid->GetTranslation() - pos;
      const float distanceSquared = delta.MagSquared();
      if (distanceSquared != 0.f && distanceSquared < radiusSquared) {
        nearList.push_back(boid);
        --remaining;
      }
    }
  }
}

static inline CVector3f FishCloudCross(const CVector3f& lhs, const CVector3f& rhs) {
  const float lX = lhs.GetX();
  const float lY = lhs.GetY();
  const float lZ = lhs.GetZ();
  const float rX = rhs.GetX();
  const float rY = rhs.GetY();
  const float rZ = rhs.GetZ();
  const float z = lX * rY - rX * lY;
  const float y = lZ * rX - rZ * lX;
  const float x = lY * rZ - rY * lZ;
  return CVector3f(x, y, z);
}

void CFishCloud::ApplyRotation(CBoid& boid, float magnitude, const CVector3f& point, float radius,
                               bool clockwise) {
  CVector3f delta = boid.x0_pos - point;
  delta[kDZ] = 0.f;
  const float distance = delta.Magnitude();
  const CVector3f align = clockwise ? FishCloudCross(delta.AsNormalized(), CVector3f::Up())
                                    : FishCloudCross(CVector3f::Up(), delta / distance);
  const CVector3f velocity = boid.xc_vel;
  const float weight = distance > radius ? 0.f : 1.f - distance / radius;
  const float angle = CVector3f::GetAngleDiff(velocity, align) / M_PIF;
  const float weightedAngle = angle * weight;
  boid.xc_vel += weightedAngle * (magnitude * align);
}

void CFishCloud::ApplyAlignment(CBoid& boid, const rstl::reserved_vector< CBoid*, 25 >& nearList) {
  if (nearList.size() > 0) {
    CVector3f average(0.f, 0.f, 0.f);
    for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
      average += (*it)->xc_vel;
    }
    average = average / float(nearList.size());
    const CVector3f velocity = boid.xc_vel;
    const float angle = CVector3f::GetAngleDiff(velocity, average) / M_PIF;
    boid.xc_vel += angle * (x140_alignmentWeight * average);
  }
}

void CFishCloud::ApplyWander(CStateManager& mgr, CBoid& boid) {
  const float x = boid.xc_vel.GetX();
  const float y = boid.xc_vel.GetY();
  const float angle = x154_maxScatterAngle * (M_PIF * (mgr.Random()->Float() - 0.5f));
  const CVector3f scatter(x * CMath::FastCosR(angle) - y * CMath::FastSinR(angle),
                          x * CMath::FastSinR(angle) + y * CMath::FastCosR(angle), 0.f);
  boid.xc_vel += x150_scatterVel * scatter;
}

void CFishCloud::ApplyCohesion(CBoid& boid, const rstl::reserved_vector< CBoid*, 25 >& nearList) {
  if (nearList.size() > 0) {
    CVector3f average(0.f, 0.f, 0.f);
    for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
      average += (*it)->GetTranslation();
    }
    average = average / float(nearList.size());
    ApplyCohesion(boid, average, x138_separationRadius, x13c_cohesionMagnitude);
  }
}

void CFishCloud::ApplyCohesion(CBoid& boid, const CVector3f& point, float radius, float magnitude) {
  const CVector3f delta = point - boid.GetTranslation();
  if (delta.CanBeNormalized()) {
    const float distanceSquared = delta.MagSquared();
    const float weight = distanceSquared > radius ? 1.f : distanceSquared / radius;
    boid.xc_vel += weight * delta.AsNormalized() * magnitude;
  }
}

void CFishCloud::ApplySeparation(CBoid& boid, const rstl::reserved_vector< CBoid*, 25 >& nearList) {
  if (nearList.size() > 0) {
    CVector3f nearest(0.f, 0.f, 0.f);
    float minDistanceSquared = FLT_MAX;
    for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
      const CVector3f delta = boid.GetTranslation() - (*it)->GetTranslation();
      const float distanceSquared = delta.MagSquared();
      if (distanceSquared < minDistanceSquared) {
        minDistanceSquared = distanceSquared;
        nearest = (*it)->GetTranslation();
      }
    }
    ApplySeparation(boid, nearest, x138_separationRadius, x144_separationMagnitude);
  }
}

void CFishCloud::ApplySeparation(CBoid& boid, const CVector3f& point, float radius,
                                 float magnitude) {
  const CVector3f delta = boid.GetTranslation() - point;
  if (delta.CanBeNormalized()) {
    const float distanceSquared = delta.MagSquared();
    const float radiusSquared = radius * radius;
    if (distanceSquared < radiusSquared) {
      const float weight = 1.f - distanceSquared / radiusSquared;
      boid.xc_vel += weight * delta.AsNormalized() * magnitude;
    }
  }
}

void CFishCloud::ApplyAttraction(CBoid& boid, const CVector3f& point, float radius,
                                 float magnitude) {
  const CVector3f delta = point - boid.GetTranslation();
  if (delta.CanBeNormalized()) {
    const float distanceSquared = delta.MagSquared();
    const float radiusSquared = radius * radius;
    if (distanceSquared < radiusSquared) {
      const float weight = 1.f - distanceSquared / radiusSquared;
      boid.xc_vel += weight * delta.AsNormalized() * magnitude;
    }
  }
}

void CFishCloud::ApplyRepulsion(CBoid& boid, const CVector3f& point, float radius,
                                float magnitude) {
  ApplySeparation(boid, point, radius, magnitude);
}

void CFishCloud::ApplyContainment(CBoid& boid, const CAABox& aabb) {
  const float radius = x174_containmentRadius;
  if (boid.xc_vel.CanBeNormalized()) {
    const CVector3f futurePos = boid.x0_pos + radius * (x130_speed * boid.xc_vel.AsNormalized());
    if (!PointInBox(aabb, futurePos)) {
      ApplyAttraction(boid, aabb.GetCenterPoint(), 100000.f, x158_containmentMagnitude);
    }
  }
}

void CFishCloud::AddParticles(const CVector3f& pos) {
  for (int i = 0; i < x1f8_particleGens.size(); ++i) {
    x1f8_particleGens[i]->SetParticleEmission(true);
    x1f8_particleGens[i]->SetTranslation(pos);
    x1f8_particleGens[i]->ForceParticleCreation(x21c_deathParticleCounts[i]);
    x1f8_particleGens[i]->SetParticleEmission(false);
  }
}

void CFishCloud::UpdateParticles(float dt) {
  for (int i = 0; i < x1f8_particleGens.size(); ++i) {
    x1f8_particleGens[i]->Update(dt);
  }
}

void CFishCloud::RenderParticles() const {
  for (int i = 0; i < x1f8_particleGens.size(); ++i) {
    gpRender->AddParticleGen(*x1f8_particleGens[i]);
  }
}

CFishCloud::~CFishCloud() {}
