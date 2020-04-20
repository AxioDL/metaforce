#include "Runtime/World/CFishCloud.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Graphics/CSkinnedModel.hpp"
#include "Runtime/Graphics/CVertexMorphEffect.hpp"
#include "Runtime/Weapon/CWeapon.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

CFishCloud::CFishCloud(TUniqueId uid, bool active, std::string_view name, const CEntityInfo& info,
                       const zeus::CVector3f& scale, const zeus::CTransform& xf, CModelData&& mData,
                       const CAnimRes& aRes, u32 numBoids, float speed, float separationRadius, float cohesionMagnitude,
                       float alignmentWeight, float separationMagnitude, float weaponRepelMagnitude,
                       float playerRepelMagnitude, float containmentMagnitude, float scatterVel, float maxScatterAngle,
                       float weaponRepelDampingSpeed, float playerRepelDampingSpeed, float containmentRadius,
                       u32 updateShift, const zeus::CColor& color, bool killable, float weaponKillRadius,
                       CAssetId part1, u32 partCount1, CAssetId part2, u32 partCount2, CAssetId part3, u32 partCount3,
                       CAssetId part4, u32 partCount4, u32 deathSfx, bool repelFromThreats, bool hotInThermal)
: CActor(uid, active, name, info, xf, std::move(mData), {EMaterialTypes::NoStepLogic},
         CActorParameters::None().HotInThermal(hotInThermal), kInvalidUniqueId)
, x11c_updateMask(u32((1 << updateShift) - 1))
, x120_scale(scale)
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
, x234_deathSfx(deathSfx != 0xffffffff ? CSfxManager::TranslateSFXID(u16(deathSfx & 0xffff)) : u16(0xffff))
, x250_28_killable(killable)
, x250_29_repelFromThreats(repelFromThreats) {
  x108_modifierSources.reserve(10);
  if (aRes.GetId().IsValid()) {
    x1b0_models.emplace_back(std::make_unique<CModelData>(aRes));
    x1b0_models.emplace_back(std::make_unique<CModelData>(aRes));
    x1b0_models.emplace_back(std::make_unique<CModelData>(aRes));
    x1b0_models.emplace_back(std::make_unique<CModelData>(aRes));
    x250_27_validModel = true;
  }
  if (part1.IsValid()) {
    x1c4_particleDescs.push_back(g_SimplePool->GetObj({FOURCC('PART'), part1}));
  }
  if (part2.IsValid()) {
    x1c4_particleDescs.push_back(g_SimplePool->GetObj({FOURCC('PART'), part2}));
  }
  if (part3.IsValid()) {
    x1c4_particleDescs.push_back(g_SimplePool->GetObj({FOURCC('PART'), part3}));
  }
  if (part4.IsValid()) {
    x1c4_particleDescs.push_back(g_SimplePool->GetObj({FOURCC('PART'), part4}));
  }
  for (const auto& p : x1c4_particleDescs) {
    x1f8_particleGens.emplace_back(std::make_unique<CElementGen>(p));
    x1f8_particleGens.back()->SetParticleEmission(false);
  }
  x21c_deathParticleCounts.push_back(partCount1);
  x21c_deathParticleCounts.push_back(partCount2);
  x21c_deathParticleCounts.push_back(partCount3);
  x21c_deathParticleCounts.push_back(partCount4);
  const zeus::CAABox aabb = GetBoundingBox();
  x238_partitionPitch = (aabb.max - aabb.min) / 7.f;
  x244_ooPartitionPitch = 1.f / x238_partitionPitch;
}

void CFishCloud::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CFishCloud::UpdateParticles(float dt) {
  for (auto& p : x1f8_particleGens) {
    p->Update(dt);
  }
}

void CFishCloud::UpdatePartitionList() {
  xf8_boidPartitionLists.clear();
  xf8_boidPartitionLists.resize(xf8_boidPartitionLists.capacity());
  const auto aabb = GetBoundingBox();
  for (auto& b : xe8_boids) {
    const zeus::CVector3f idxs = (b.x0_pos - aabb.min) * x244_ooPartitionPitch;
    const int idx = int(idxs.x()) + int(idxs.y()) * 7 + int(idxs.z()) * 49;
    if (idx >= 0 && idx < 343) {
      b.x1c_next = xf8_boidPartitionLists[idx];
      xf8_boidPartitionLists[idx] = &b;
    }
  }
}

bool CFishCloud::PointInBox(const zeus::CAABox& aabb, const zeus::CVector3f& point) const {
  if (!x250_25_worldSpace) {
    return aabb.pointInside(point);
  }
  return GetUntransformedBoundingBox().pointInside(GetTransform().transposeRotate(point - GetTranslation()));
}

zeus::CPlane CFishCloud::FindClosestPlane(const zeus::CAABox& aabb, const zeus::CVector3f& point) const {
  if (!x250_25_worldSpace) {
    float minDist = FLT_MAX;
    auto minFace = zeus::CAABox::EBoxFaceId::YMin;
    for (int i = 0; i < 6; ++i) {
      const auto tri = aabb.getTri(zeus::CAABox::EBoxFaceId(i), 0);
      const float dist = zeus::CPlane(tri.x10_verts[0], tri.x10_verts[2], tri.x10_verts[1]).pointToPlaneDist(point);
      if (dist >= 0.f && dist < minDist) {
        minDist = dist;
        minFace = zeus::CAABox::EBoxFaceId(i);
      }
    }
    const auto tri = aabb.getTri(minFace, 0);
    return zeus::CPlane(tri.x10_verts[0], tri.x10_verts[2], tri.x10_verts[1]);
  } else {
    const auto unPoint = GetTransform().transposeRotate(point - GetTranslation());
    const auto unAabb = GetUntransformedBoundingBox();
    float minDist = FLT_MAX;
    auto minFace = zeus::CAABox::EBoxFaceId::YMin;
    for (int i = 0; i < 6; ++i) {
      const auto tri = unAabb.getTri(zeus::CAABox::EBoxFaceId(i), 0);
      const float dist = zeus::CPlane(tri.x10_verts[0], tri.x10_verts[2], tri.x10_verts[1]).pointToPlaneDist(unPoint);
      if (dist >= 0.f && dist < minDist) {
        minDist = dist;
        minFace = zeus::CAABox::EBoxFaceId(i);
      }
    }

    const auto tri = unAabb.getTri(minFace, 0);
    return zeus::CPlane(GetTransform() * tri.x10_verts[0],
                        GetTransform() * tri.x10_verts[2],
                        GetTransform() * tri.x10_verts[1]);
  }
}

CFishCloud::CBoid* CFishCloud::GetListAt(const zeus::CVector3f& pos) {
  const zeus::CAABox aabb = GetBoundingBox();
  const zeus::CVector3f ints = (pos - aabb.min) * x244_ooPartitionPitch;
  const int idx = int(ints.x()) + int(ints.y()) * 7 + int(ints.z()) * 49;

  if (idx < 0 || idx >= 343) {
    return nullptr;
  }

  return xf8_boidPartitionLists[idx];
}

void CFishCloud::BuildBoidNearList(const zeus::CVector3f& pos, float radius,
                                   rstl::reserved_vector<CBoid*, 25>& nearList) {
  const float radiusSq = radius * radius;
  CBoid* b = GetListAt(pos);

  while (b != nullptr && nearList.size() < 25) {
    if (b->x20_active) {
      const float distSq = (b->GetTranslation() - pos).magSquared();
      if (distSq != 0.f && distSq < radiusSq) {
        nearList.push_back(b);
      }
    }
    b = b->x1c_next;
  }
}

void CFishCloud::BuildBoidNearPartitionList(const zeus::CVector3f& pos, float radius,
                                            rstl::reserved_vector<CBoid*, 25>& nearList) {
  const float radiusSq = radius * radius;
  const zeus::CAABox aabb = GetBoundingBox();
  const float x = std::max(radius * x244_ooPartitionPitch.x(), float(x238_partitionPitch.x()));
  const float y = std::max(radius * x244_ooPartitionPitch.y(), float(x238_partitionPitch.y()));
  const float z = std::max(radius * x244_ooPartitionPitch.z(), float(x238_partitionPitch.z()));
  const float nx = 0.01f - x;
  const float ny = 0.01f - y;
  const float nz = 0.01f - z;

  for (float lnx = nx; lnx < x; lnx += x238_partitionPitch.x()) {
    const float cx = lnx + pos.x();
    if (cx < aabb.min.x()) {
      continue;
    }
    if (cx >= aabb.max.x()) {
      break;
    }
    for (float lny = ny; lny < y; lny += x238_partitionPitch.y()) {
      const float cy = lny + pos.y();
      if (cy < aabb.min.y()) {
        continue;
      }
      if (cy >= aabb.max.y()) {
        break;
      }
      for (float lnz = nz; lnz < z; lnz += x238_partitionPitch.z()) {
        const float cz = lnz + pos.z();
        if (cz < aabb.min.z()) {
          continue;
        }
        if (cz >= aabb.max.z()) {
          break;
        }
        const zeus::CVector3f ints = (zeus::CVector3f(cx, cy, cz) - aabb.min) * x244_ooPartitionPitch;
        const int idx = int(ints.x()) + int(ints.y()) * 7 + int(ints.z()) * 49;
        if (idx < 0) {
          continue;
        }
        if (idx < 343) {
          CBoid* boid = xf8_boidPartitionLists[idx];
          while (boid != nullptr) {
            if (boid->x20_active) {
              const float distSq = (boid->x0_pos - pos).magSquared();
              if (distSq != 0.f && distSq < radiusSq) {
                nearList.push_back(boid);
                if (nearList.size() == 25) {
                  return;
                }
              }
            }
            boid = boid->x1c_next;
          }
        }
      }
    }
  }
}

void CFishCloud::PlaceBoid(CStateManager& mgr, CBoid& boid, const zeus::CAABox& aabb) const {
  const auto plane = FindClosestPlane(aabb, boid.x0_pos);
  boid.x0_pos -= plane.pointToPlaneDist(boid.x0_pos) * plane.normal() + 0.0001f * plane.normal();
  boid.xc_vel.y() = mgr.GetActiveRandom()->Float() - 0.5f;
  boid.xc_vel.x() = mgr.GetActiveRandom()->Float() - 0.5f;
  boid.xc_vel.z() = 0.f;
  if (!x250_25_worldSpace) {
    if (!aabb.pointInside(boid.x0_pos)) {
      boid.x0_pos.z() = mgr.GetActiveRandom()->Float() * (aabb.max.z() - aabb.min.z()) + aabb.min.z();
      boid.x0_pos.y() = mgr.GetActiveRandom()->Float() * (aabb.max.y() - aabb.min.y()) + aabb.min.y();
      boid.x0_pos.x() = mgr.GetActiveRandom()->Float() * (aabb.max.x() - aabb.min.x()) + aabb.min.x();
    }
  } else {
    if (!PointInBox(aabb, boid.x0_pos)) {
      const auto unAabb = GetUntransformedBoundingBox();
      boid.x0_pos.z() = mgr.GetActiveRandom()->Float() * (unAabb.max.z() - unAabb.min.z()) + unAabb.min.z();
      boid.x0_pos.y() = mgr.GetActiveRandom()->Float() * (unAabb.max.y() - unAabb.min.y()) + unAabb.min.y();
      boid.x0_pos.x() = mgr.GetActiveRandom()->Float() * (unAabb.max.x() - unAabb.min.x()) + unAabb.min.x();
      boid.x0_pos = GetTransform() * boid.x0_pos;
    }
  }
}

void CFishCloud::ApplySeparation(CBoid& boid, const rstl::reserved_vector<CBoid*, 25>& nearList) const {
  if (nearList.empty()) {
    return;
  }

  float minDist = FLT_MAX;
  zeus::CVector3f pos;
  for (const CBoid* b : nearList) {
    const float dist = (boid.GetTranslation() - b->GetTranslation()).magSquared();
    if (dist < minDist) {
      minDist = dist;
      pos = b->GetTranslation();
    }
  }

  ApplySeparation(boid, pos, x138_separationRadius, x144_separationMagnitude);
}

void CFishCloud::ApplySeparation(CBoid& boid, const zeus::CVector3f& separateFrom,
                                 float separationRadius, float separationMagnitude) const {
  const zeus::CVector3f delta = boid.GetTranslation() - separateFrom;
  if (!delta.canBeNormalized()) {
    return;
  }

  const float deltaDistSq = delta.magSquared();
  const float capDeltaDistSq = separationRadius * separationRadius;
  if (deltaDistSq >= capDeltaDistSq) {
    return;
  }

  boid.xc_vel += (1.f - deltaDistSq / capDeltaDistSq) * delta.normalized() * separationMagnitude;
}

void CFishCloud::ApplyCohesion(CBoid& boid, const rstl::reserved_vector<CBoid*, 25>& nearList) const {
  if (nearList.empty()) {
    return;
  }

  zeus::CVector3f avg;
  for (const CBoid* b : nearList) {
    avg += b->GetTranslation();
  }

  avg = avg / float(nearList.size());
  ApplyCohesion(boid, avg, x138_separationRadius, x13c_cohesionMagnitude);
}

void CFishCloud::ApplyCohesion(CBoid& boid, const zeus::CVector3f& cohesionFrom,
                               float cohesionRadius, float cohesionMagnitude) const {
  const zeus::CVector3f delta = cohesionFrom - boid.GetTranslation();
  if (!delta.canBeNormalized()) {
    return;
  }

  const float distSq = delta.magSquared();
  const float capDistSq = cohesionRadius * cohesionRadius;
  boid.xc_vel += ((distSq > capDistSq) ? 1.f : distSq / capDistSq) * delta.normalized() * cohesionMagnitude;
}

void CFishCloud::ApplyAlignment(CBoid& boid, const rstl::reserved_vector<CBoid*, 25>& nearList) const {
  if (nearList.empty()) {
    return;
  }

  zeus::CVector3f avg;
  for (const CBoid* b : nearList) {
    avg += b->xc_vel;
  }

  avg = avg / float(nearList.size());
  boid.xc_vel += zeus::CVector3f::getAngleDiff(boid.xc_vel, avg) / M_PIF *
    (avg * x140_alignmentWeight);
}

void CFishCloud::ApplyAttraction(CBoid& boid, const zeus::CVector3f& attractTo,
                                 float attractionRadius, float attractionMagnitude) const {
  const zeus::CVector3f delta = attractTo - boid.GetTranslation();
  if (!delta.canBeNormalized()) {
    return;
  }

  const float distSq = delta.magSquared();
  const float capDistSq = attractionRadius * attractionRadius;
  boid.xc_vel += ((distSq > capDistSq) ? 0.f : (1.f - distSq / capDistSq)) * delta.normalized() * attractionMagnitude;
}

void CFishCloud::ApplyRepulsion(CBoid& boid, const zeus::CVector3f& attractTo,
                                float repulsionRadius, float repulsionMagnitude) const {
  ApplySeparation(boid, attractTo, repulsionRadius, repulsionMagnitude);
}

void CFishCloud::ApplySwirl(CBoid& boid, const zeus::CVector3f& swirlPoint, bool clockwise,
                            float magnitude, float radius) const {
  const zeus::CVector3f delta = boid.x0_pos - swirlPoint;
  const float deltaMag = delta.magnitude();

  zeus::CVector3f alignVec;
  if (clockwise) {
    alignVec = delta.normalized().cross(zeus::skUp);
  } else {
    alignVec = zeus::skUp.cross(delta / deltaMag);
  }

  const float weight = deltaMag > radius ? 0.f : 1.f - deltaMag / radius;
  boid.xc_vel += zeus::CVector3f::getAngleDiff(boid.xc_vel, alignVec) / M_PIF *
    weight * (magnitude * alignVec);
}

void CFishCloud::ApplyContainment(CBoid& boid, const zeus::CAABox& aabb) const {
  if (!boid.xc_vel.canBeNormalized()) {
    return;
  }

  if (PointInBox(aabb, boid.xc_vel.normalized() * x130_speed * x174_containmentRadius + boid.x0_pos)) {
    return;
  }

  ApplyAttraction(boid, aabb.center(), 100000.f, x158_containmentMagnitude);
}

void CFishCloud::ScatterBoid(CStateManager& mgr, CBoid& b) const {
  const float angle = (mgr.GetActiveRandom()->Float() - 0.5f) * M_PIF * x154_maxScatterAngle;
  const float cosAngle = std::cos(angle);
  const float sinAngle = std::sin(angle);
  b.xc_vel.x() += x150_scatterVel * (b.xc_vel.y() * sinAngle + b.xc_vel.x() * cosAngle);
  b.xc_vel.y() += x150_scatterVel * (b.xc_vel.y() * cosAngle + b.xc_vel.x() * sinAngle);
}

void CFishCloud::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  const CGameArea* area = mgr.GetWorld()->GetAreaAlways(x4_areaId);
  const auto occState = area->IsPostConstructed() ? area->GetOcclusionState() : CGameArea::EOcclusionState::Occluded;
  if (occState == CGameArea::EOcclusionState::Visible) {
    x168_weaponRepelDamping = std::max(0.f, x168_weaponRepelDamping - x160_weaponRepelDampingSpeed * dt * 0.1f);
    if (x250_26_enableWeaponRepelDamping) {
      x168_weaponRepelDamping =
          std::min(x160_weaponRepelDampingSpeed * dt + x168_weaponRepelDamping, x148_weaponRepelMagnitude);
    }

    x164_playerRepelDamping = std::max(0.f, x164_playerRepelDamping - x15c_playerRepelDampingSpeed * dt * 0.1f);
    if (x250_30_enablePlayerRepelDamping) {
      x164_playerRepelDamping =
          std::min(x15c_playerRepelDampingSpeed * dt + x164_playerRepelDamping, x14c_playerRepelMagnitude);
    }

    x250_26_enableWeaponRepelDamping = false;
    x250_30_enablePlayerRepelDamping = false;
    ++x118_thinkCounter;

    UpdateParticles(dt);
    UpdatePartitionList();

    const zeus::CAABox aabb = GetBoundingBox();
    int idx = 0;
    for (auto& b : xe8_boids) {
      if (b.x20_active && (idx & x11c_updateMask) == (x118_thinkCounter & x11c_updateMask)) {
        rstl::reserved_vector<CBoid*, 25> nearList;
        if (x250_31_updateWithoutPartitions) {
          BuildBoidNearList(b.x0_pos, x138_separationRadius, nearList);
        } else {
          BuildBoidNearPartitionList(b.x0_pos, x138_separationRadius, nearList);
        }

        for (int i = 0; i < 5; ++i) {
          switch (i) {
          case 1:
            ApplySeparation(b, nearList);
            break;
          case 2:
            if (!x250_24_randomMovement || mgr.GetActiveRandom()->Float() > x12c_randomMovementTimer) {
              ApplyCohesion(b, nearList);
            }
            break;
          case 3:
            if (!x250_24_randomMovement || mgr.GetActiveRandom()->Float() > x12c_randomMovementTimer) {
              ApplyAlignment(b, nearList);
            }
            break;
          case 4:
            ScatterBoid(mgr, b);
            break;
          default:
            break;
          }
          if (b.xc_vel.magSquared() > 3.2f) {
            break;
          }
        }

        if (!x250_24_randomMovement && b.xc_vel.magSquared() < 3.2f) {
          for (const auto& m : x108_modifierSources) {
            if (const TCastToConstPtr<CActor> act = mgr.ObjectById(m.x0_source)) {
              if (m.xd_isSwirl) {
                ApplySwirl(b, act->GetTranslation(), m.xc_isRepulsor, m.x8_priority, m.x4_radius);
              } else if (m.xc_isRepulsor) {
                ApplyRepulsion(b, act->GetTranslation(), m.x4_radius, m.x8_priority);
              } else {
                ApplyAttraction(b, act->GetTranslation(), m.x4_radius, m.x8_priority);
              }
            } else {
              if (m.xc_isRepulsor)
                RemoveRepulsor(m.x0_source);
              else
                RemoveAttractor(m.x0_source);
              break;
            }
          }
        }
      }
      ++idx;
    }

    for (auto& b : xe8_boids) {
      if (b.x20_active) {
        ApplyContainment(b, aabb);
        const float velMag = b.xc_vel.magnitude();
        if (!zeus::close_enough(velMag, 0.f)) {
          b.xc_vel = b.xc_vel / velMag;
        }
        b.xc_vel.z() *= 0.99f;
      }
    }

    if (x12c_randomMovementTimer > 0.f) {
      x12c_randomMovementTimer -= dt;
    } else {
      x12c_randomMovementTimer = 0.f;
      x250_24_randomMovement = false;
    }

    for (auto& b : xe8_boids) {
      if (b.x20_active) {
        b.x0_pos += b.xc_vel * dt * x130_speed;
        if (!PointInBox(aabb, b.x0_pos)) {
          PlaceBoid(mgr, b, aabb);
        }
      }
    }

    if (x250_27_validModel) {
      for (auto& m : x1b0_models) {
        m->GetAnimationData()->SetPlaybackRate(1.f);
        m->AdvanceAnimation(dt, mgr, x4_areaId, true);
      }
    }
  }
}

void CFishCloud::CreatePartitionList() {
  xf8_boidPartitionLists.reserve(343);
}

void CFishCloud::AllocateSkinnedModels(CStateManager& mgr, CModelData::EWhichModel which) {
  int idx = 0;
  for (auto& m : x1b0_models) {
    m->EnableLooping(true);
    m->AdvanceAnimation(m->GetAnimationData()->GetAnimTimeRemaining("Whole Body"sv) * 0.25f * float(idx), mgr,
                        x4_areaId, true);
    ++idx;
  }
  x230_whichModel = which;
}

void CFishCloud::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, sender, mgr);
  switch (msg) {
  case EScriptObjectMessage::Registered: {
    xe8_boids.reserve(x134_numBoids);
    const zeus::CAABox aabb = GetUntransformedBoundingBox();
    const zeus::CVector3f extent = aabb.max - aabb.min;
    zeus::CVector3f randPoint;
    for (u32 i = 0; i < x134_numBoids; ++i) {
      randPoint.z() = mgr.GetActiveRandom()->Float() * extent.z() + aabb.min.z();
      randPoint.y() = mgr.GetActiveRandom()->Float() * extent.y() + aabb.min.y();
      randPoint.x() = mgr.GetActiveRandom()->Float() * extent.x() + aabb.min.x();
      zeus::CVector3f vel;
      vel.y() = mgr.GetActiveRandom()->Float() - 0.5f;
      vel.x() = mgr.GetActiveRandom()->Float() - 0.5f;
      xe8_boids.emplace_back(x34_transform * randPoint, vel,
                             0.2f * std::pow(mgr.GetActiveRandom()->Float(), 7.f) + 0.9f);
    }
    CreatePartitionList();
    if (x250_27_validModel) {
      AllocateSkinnedModels(mgr, CModelData::EWhichModel::Normal);
    }
    break;
  }
  default:
    break;
  }
}

void CFishCloud::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) {
  CActor::PreRender(mgr, frustum);
  if (x250_27_validModel) {
    for (auto& m : x1b0_models) {
      m->GetAnimationData()->PreRender();
    }
  }
  xe4_30_outOfFrustum = false;
}

void CFishCloud::AddParticlesToRenderer() const {
  for (const auto& p : x1f8_particleGens) {
    g_Renderer->AddParticleGen(*p);
  }
}

void CFishCloud::RenderBoid(int idx, const CBoid& boid, u32& drawMask,
                            bool thermalHot, const CModelFlags& flags) const {
  const u32 modelIndex = idx & 0x3;
  CModelData& mData = *x1b0_models[modelIndex];
  CSkinnedModel& model = mData.PickAnimatedModel(CModelData::EWhichModel::Normal);
  if (!model.GetModelInst()->TryLockTextures()) {
    return;
  }

  const u32 thisDrawMask = 1u << modelIndex;
  if ((drawMask & thisDrawMask) != 0) {
    drawMask &= ~thisDrawMask;
    mData.GetAnimationData()->BuildPose();
  }

  model.GetModelInst()->SetAmbientColor(zeus::skWhite);
  CGraphics::SetModelMatrix(zeus::lookAt(boid.x0_pos, boid.x0_pos + boid.xc_vel));
  if (thermalHot) {
    constexpr CModelFlags thermFlags(0, 0, 3, zeus::skWhite);
    mData.RenderThermal(zeus::skWhite, zeus::CColor(0.f, 0.25f), thermFlags);
  } else {
    mData.GetAnimationData()->Render(model, flags, std::nullopt, nullptr);
  }
}

void CFishCloud::Render(CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  SCOPED_GRAPHICS_DEBUG_GROUP(fmt::format(FMT_STRING("CFishCloud::Render {} {} {}"),
                                          x8_uid, xc_editorId, x10_name).c_str(), zeus::skOrange);

  const bool thermalHot = mgr.GetThermalDrawFlag() == EThermalDrawFlag::Hot;
  CModelFlags flags(0, 0, 3, zeus::skWhite);
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::XRay) {
    flags = CModelFlags(5, 0, 3, x16c_color);
  } else {
    flags = CModelFlags(1, 0, 3, x16c_color);
  }

  AddParticlesToRenderer();

  if (x250_27_validModel) {
    // Ambient white
    int idx = 0;
    u32 drawMask = 0xffffffff;
    for (const auto& b : xe8_boids) {
      if (b.x20_active) {
        RenderBoid(idx, b, drawMask, thermalHot, flags);
      }
      ++idx;
    }
  } else {
    CGraphics::SetModelMatrix(zeus::CTransform());
    for (const auto& b : xe8_boids) {
      if (b.x20_active) {
        x64_modelData->SetScale(zeus::CVector3f(b.x18_scale));
        x64_modelData->Render(mgr, zeus::lookAt(b.x0_pos, b.x0_pos + b.xc_vel), nullptr, flags);
      }
    }
  }
}

void CFishCloud::CalculateRenderBounds() {
  x9c_renderBounds = GetBoundingBox();
}

std::optional<zeus::CAABox> CFishCloud::GetTouchBounds() const {
  return {GetBoundingBox()};
}

void CFishCloud::CreateBoidDeathParticle(CBoid& b) const {
  auto it = x21c_deathParticleCounts.begin();
  for (auto& p : x1f8_particleGens) {
    p->SetParticleEmission(true);
    p->SetTranslation(b.x0_pos);
    p->ForceParticleCreation(*it);
    p->SetParticleEmission(false);
    ++it;
  }
}

void CFishCloud::KillBoid(CBoid& b) const {
  b.x20_active = false;
  CreateBoidDeathParticle(b);
  CAudioSys::C3DEmitterParmData parmData = {};
  parmData.x0_pos = b.x0_pos;
  parmData.x18_maxDist = 250.f;
  parmData.x1c_distComp = 0.1f;
  parmData.x20_flags = 0x1;
  parmData.x24_sfxId = x234_deathSfx;
  parmData.x26_maxVol = 1.f;
  parmData.x27_minVol = 0.157f;
  parmData.x29_prio = 0x7f;
  CSfxManager::AddEmitter(parmData, true, 0x7f, false, x4_areaId);
}

void CFishCloud::Touch(CActor& other, CStateManager& mgr) {
  CActor::Touch(other, mgr);

  if (const TCastToConstPtr<CWeapon> weap = other) {
    if (!x250_26_enableWeaponRepelDamping && x250_29_repelFromThreats) {
      int idx = 0;
      for (auto& b : xe8_boids) {
        if ((idx & 0x3) == (x118_thinkCounter & 0x3)) {
          ApplyRepulsion(b, weap->GetTranslation(), 8.f, x148_weaponRepelMagnitude - x168_weaponRepelDamping);
        }
        ++idx;
      }
    }

    x250_26_enableWeaponRepelDamping = true;

    if (x250_28_killable) {
      if (const auto tb = weap->GetTouchBounds()) {
        for (auto& b : xe8_boids) {
          if (b.x20_active && tb->intersects(zeus::CAABox(weap->GetTranslation() - x170_weaponKillRadius,
                                                          weap->GetTranslation() + x170_weaponKillRadius))) {
            KillBoid(b);
          }
        }
      }
    }
  }

  if (x250_29_repelFromThreats) {
    if (const TCastToConstPtr<CPlayer> player = other) {
      const zeus::CVector3f playerPos = player->GetTranslation();
      for (auto& b : xe8_boids) {
        zeus::CVector3f adjPlayerPos = playerPos;
        const float zDelta = b.x0_pos.z() - adjPlayerPos.z();
        if (zDelta > 0.f && zDelta < 2.3f) {
          adjPlayerPos.z() = float(b.x0_pos.z());
        }
        adjPlayerPos.x() += mgr.GetActiveRandom()->Float() * 0.2f - 0.1f;
        adjPlayerPos.y() += mgr.GetActiveRandom()->Float() * 0.2f - 0.1f;
        ApplyRepulsion(b, adjPlayerPos, 8.f, x14c_playerRepelMagnitude - x164_playerRepelDamping);
      }
    }
    x250_30_enablePlayerRepelDamping = true;
  }
}

zeus::CAABox CFishCloud::GetUntransformedBoundingBox() const {
  const zeus::CVector3f extent = x120_scale * 0.75f;
  return zeus::CAABox(-extent, extent);
}

zeus::CAABox CFishCloud::GetBoundingBox() const {
  return GetUntransformedBoundingBox().getTransformedAABox(x34_transform);
}

void CFishCloud::RemoveRepulsor(TUniqueId sourceId) {
  const CModifierSource source(sourceId, true, false, 0.f, 0.f);
  const auto it = rstl::binary_find(x108_modifierSources.begin(), x108_modifierSources.end(), source);

  if (it == x108_modifierSources.end()) {
    return;
  }

  x108_modifierSources.erase(it);
}

void CFishCloud::RemoveAttractor(TUniqueId sourceId) {
  const CModifierSource source(sourceId, false, false, 0.f, 0.f);
  const auto it = rstl::binary_find(x108_modifierSources.begin(), x108_modifierSources.end(), source);

  if (it == x108_modifierSources.end()) {
    return;
  }

  x108_modifierSources.erase(it);
}

bool CFishCloud::AddRepulsor(TUniqueId sourceId, bool swirl, float radius, float priority) {
  const CModifierSource source(sourceId, true, swirl, radius, priority);
  const auto it = rstl::binary_find(x108_modifierSources.begin(), x108_modifierSources.end(), source);

  if (it != x108_modifierSources.end()) {
    it->x4_radius = radius;
    it->x8_priority = priority;
    return true;
  }

  if (x108_modifierSources.size() < x108_modifierSources.capacity()) {
    x108_modifierSources.insert(std::lower_bound(x108_modifierSources.begin(), x108_modifierSources.end(), source),
                                source);
    return true;
  }

  return false;
}

bool CFishCloud::AddAttractor(TUniqueId sourceId, bool swirl, float radius, float priority) {
  const CModifierSource source(sourceId, false, swirl, radius, priority);
  const auto it = rstl::binary_find(x108_modifierSources.begin(), x108_modifierSources.end(), source);

  if (it != x108_modifierSources.end()) {
    it->x4_radius = radius;
    it->x8_priority = priority;
    return true;
  }

  if (x108_modifierSources.size() < x108_modifierSources.capacity()) {
    x108_modifierSources.insert(std::lower_bound(x108_modifierSources.begin(), x108_modifierSources.end(), source),
                                source);
    return true;
  }

  return false;
}

} // namespace urde
