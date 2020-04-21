#include "Runtime/Character/CRagDoll.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/Collision/CCollidableSphere.hpp"
#include "Runtime/Collision/CCollisionInfo.hpp"
#include "Runtime/Collision/CGameCollision.hpp"
#include "Runtime/Collision/CMaterialFilter.hpp"
#include "Runtime/Collision/CMetroidAreaCollider.hpp"

namespace urde {

void CRagDoll::CRagDollLengthConstraint::Update() {
  zeus::CVector3f delta = x4_p2->x4_curPos - x0_p1->x4_curPos;
  float magSq = delta.magSquared();
  float lenSq = x8_length * x8_length;
  bool doSolve = true;
  switch (xc_ineqType) {
  case 1: // Min
    doSolve = magSq < lenSq;
    break;
  case 2: // Max
    doSolve = magSq > lenSq;
    break;
  default:
    break;
  }
  if (!doSolve)
    return;
  zeus::CVector3f solveVec = delta * (lenSq / (magSq + lenSq) - 0.5f);
  x0_p1->x4_curPos -= solveVec;
  x4_p2->x4_curPos += solveVec;
}

void CRagDoll::CRagDollJointConstraint::Update() {
  // L_hip, R_shoulder, L_shoulder, L_hip, L_knee, L_ankle
  zeus::CVector3f P4ToP5 = x10_p5->x4_curPos - xc_p4->x4_curPos; // L_hip->L_knee
  zeus::CVector3f cross =
      P4ToP5.cross((x8_p3->x4_curPos - x0_p1->x4_curPos).cross(x4_p2->x4_curPos - x0_p1->x4_curPos));
  // L_hip->L_knee X (L_hip->L_shoulder X L_hip->R_shoulder)
  if (cross.canBeNormalized()) {
    zeus::CVector3f hipUp = cross.cross(P4ToP5).normalized();
    float dot = (x14_p6->x4_curPos - x10_p5->x4_curPos).dot(hipUp);
    if (dot > 0.f) {
      zeus::CVector3f solveVec = 0.5f * dot * hipUp;
      x14_p6->x4_curPos -= solveVec;
      x10_p5->x4_curPos += solveVec;
    }
  }
}

void CRagDoll::CRagDollPlaneConstraint::Update() {
  zeus::CVector3f P1ToP2 = (x4_p2->x4_curPos - x0_p1->x4_curPos).normalized();
  float dot = P1ToP2.dot(xc_p4->x4_curPos - x8_p3->x4_curPos);
  if (dot < 0.f) {
    zeus::CVector3f solveVec = 0.5f * dot * P1ToP2;
    xc_p4->x4_curPos -= solveVec;
    x10_p5->x4_curPos += solveVec;
  }
}

CRagDoll::CRagDoll(float normalGravity, float floatingGravity, float overTime, u32 flags)
: x44_normalGravity(normalGravity)
, x48_floatingGravity(floatingGravity)
, x50_overTimer(overTime)
, x68_27_continueSmallMovements(bool(flags & 0x1))
, x68_28_noOverTimer(bool(flags & 0x2))
, x68_29_noAiCollision(bool(flags & 0x4)) {}

void CRagDoll::AccumulateForces(float dt, float waterTop) {
  float fps = 1.f / dt;
  x64_angTimer += dt;
  if (x64_angTimer > 4.f)
    x64_angTimer -= 4.f;
  float targetZ = std::sin(zeus::degToRad(90.f) * x64_angTimer) * 0.1f + (waterTop - 0.2f);
  zeus::CVector3f centerOfVolume;
  float totalVolume = 0.f;
  for (auto& particle : x4_particles) {
    float volume = particle.x10_radius * particle.x10_radius * particle.x10_radius;
    totalVolume += volume;
    centerOfVolume += particle.x4_curPos * volume;
    float fromTargetZ = particle.x4_curPos.z() - targetZ;
    float verticalAcc = x48_floatingGravity;
    float termVelCoefficient = 0.f;
    if (std::fabs(fromTargetZ) < 0.5f) {
      termVelCoefficient = 0.5f * fromTargetZ / 0.5f + 0.5f;
      verticalAcc = x48_floatingGravity * -fromTargetZ / 0.5f;
    } else if (fromTargetZ > 0.f) {
      verticalAcc = x44_normalGravity;
      termVelCoefficient = 1.f;
    }
    particle.x20_velocity.z() += verticalAcc;
    zeus::CVector3f vel = (particle.x4_curPos - particle.x14_prevPos) * fps;
    float velMag = vel.magnitude();
    if (velMag > FLT_EPSILON) {
      particle.x20_velocity -=
          vel * (1.f / velMag) *
          ((velMag * velMag * 0.75f * (1.2f * termVelCoefficient + 1000.f * (1.f - termVelCoefficient))) /
           (8000.f * particle.x10_radius));
    }
  }
  zeus::CVector3f averageTorque;
  centerOfVolume = centerOfVolume / totalVolume;
  for (const auto& particle : x4_particles) {
    float volume = particle.x10_radius * particle.x10_radius * particle.x10_radius;
    averageTorque += (particle.x4_curPos - centerOfVolume).cross(particle.x4_curPos - particle.x14_prevPos) * volume;
  }
  averageTorque = averageTorque * (fps / totalVolume);
  if (averageTorque.canBeNormalized())
    for (auto& particle : x4_particles)
      particle.x20_velocity -= averageTorque.cross(particle.x4_curPos - centerOfVolume) * 25.f;
}

void CRagDoll::AddParticle(CSegId id, const zeus::CVector3f& prevPos, const zeus::CVector3f& curPos, float radius) {
  x4_particles.emplace_back(id, curPos, radius, prevPos);
}

void CRagDoll::AddLengthConstraint(int i1, int i2) {
  x14_lengthConstraints.emplace_back(&x4_particles[i1], &x4_particles[i2],
                                     (x4_particles[i1].x4_curPos - x4_particles[i2].x4_curPos).magnitude(), 0);
}

void CRagDoll::AddMaxLengthConstraint(int i1, int i2, float length) {
  x14_lengthConstraints.emplace_back(&x4_particles[i1], &x4_particles[i2], length, 2);
}

void CRagDoll::AddMinLengthConstraint(int i1, int i2, float length) {
  x14_lengthConstraints.emplace_back(&x4_particles[i1], &x4_particles[i2], length, 1);
}

void CRagDoll::AddJointConstraint(int i1, int i2, int i3, int i4, int i5, int i6) {
  x24_jointConstraints.emplace_back(&x4_particles[i1], &x4_particles[i2], &x4_particles[i3], &x4_particles[i4],
                                    &x4_particles[i5], &x4_particles[i6]);
}

zeus::CQuaternion CRagDoll::BoneAlign(CHierarchyPoseBuilder& pb, const CCharLayoutInfo& charInfo, int i1, int i2,
                                      const zeus::CQuaternion& q) {
  zeus::CVector3f fromParent = charInfo.GetFromParentUnrotated(x4_particles[i2].x0_id);
  zeus::CVector3f delta = x4_particles[i2].x4_curPos - x4_particles[i1].x4_curPos;
  delta = q.inverse().transform(delta);
  zeus::CQuaternion ret = zeus::CQuaternion::shortestRotationArc(fromParent, delta);
  pb.GetTreeMap()[x4_particles[i1].x0_id].x4_rotation = ret;
  return ret;
}

zeus::CAABox CRagDoll::CalculateRenderBounds() const {
  zeus::CAABox aabb;
  for (const auto& particle : x4_particles) {
    aabb.accumulateBounds(
        zeus::CAABox(particle.x4_curPos - particle.x10_radius, particle.x4_curPos + particle.x10_radius));
  }
  return aabb;
}

void CRagDoll::CheckStatic(float dt) {
  x4c_impactCount = 0;
  x54_impactVel = 0.f;
  float halfDt = 0.5f * dt;
  float halfDeltaUnitSq = halfDt * halfDt;
  x58_averageVel = zeus::skZero3f;
  bool movingSlowly = true;
  for (auto& particle : x4_particles) {
    zeus::CVector3f delta = particle.x4_curPos - particle.x14_prevPos;
    x58_averageVel += delta;
    if (delta.magSquared() > halfDeltaUnitSq)
      movingSlowly = false;
    if (particle.x3c_24_impactPending) {
      x4c_impactCount += 1;
      x54_impactVel = std::max(particle.x38_impactFrameVel, x54_impactVel);
    }
  }
  if (!x4_particles.empty())
    x58_averageVel = x58_averageVel * (1.f / (dt * x4_particles.size()));
  x54_impactVel /= dt;
  if (!x68_28_noOverTimer) {
    x50_overTimer -= dt;
    if (x50_overTimer <= 0.f)
      x68_25_over = true;
  }
  if (movingSlowly && x68_24_prevMovingSlowly)
    x68_25_over = true;
  x68_24_prevMovingSlowly = movingSlowly;
}

void CRagDoll::ClearForces() {
  for (auto& particle : x4_particles)
    particle.x20_velocity = zeus::skZero3f;
}

void CRagDoll::SatisfyConstraints(CStateManager& mgr) {
  for (auto& length : x14_lengthConstraints)
    length.Update();
  for (auto& joint : x24_jointConstraints)
    joint.Update();
  for (auto& plane : x34_planeConstraints)
    plane.Update();
  if (SatisfyWorldConstraints(mgr, 1))
    SatisfyWorldConstraints(mgr, 2);
}

bool CRagDoll::SatisfyWorldConstraints(CStateManager& mgr, int pass) {
  zeus::CAABox aabb;
  for (const auto& particle : x4_particles) {
    if (pass == 1 || particle.x3c_24_impactPending) {
      aabb.accumulateBounds(particle.x14_prevPos - particle.x10_radius);
      aabb.accumulateBounds(particle.x14_prevPos + particle.x10_radius);
      aabb.accumulateBounds(particle.x4_curPos - particle.x10_radius);
      aabb.accumulateBounds(particle.x4_curPos + particle.x10_radius);
    }
  }

  CAreaCollisionCache ccache(aabb);
  CGameCollision::BuildAreaCollisionCache(mgr, ccache);
  bool needs2ndPass = false;

  TUniqueId bestId = kInvalidUniqueId;
  CMaterialList include;
  if (x68_29_noAiCollision)
    include = CMaterialList(EMaterialTypes::Solid);
  else
    include = CMaterialList(EMaterialTypes::Solid, EMaterialTypes::AIBlock);

  CMaterialList exclude;
  if (x68_29_noAiCollision)
    exclude = CMaterialList(EMaterialTypes::Character, EMaterialTypes::Player, EMaterialTypes::AIBlock,
                            EMaterialTypes::Occluder);
  else
    exclude = CMaterialList(EMaterialTypes::Character, EMaterialTypes::Player);

  rstl::reserved_vector<TUniqueId, 1024> nearList;
  CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(include, exclude);
  mgr.BuildNearList(nearList, aabb, filter, nullptr);

  for (auto& particle : x4_particles) {
    if (pass == 1 || particle.x3c_24_impactPending) {
      zeus::CVector3f delta = particle.x4_curPos - particle.x14_prevPos;
      float deltaMag = delta.magnitude();
      if (deltaMag > 0.0001f) {
        delta = delta * (1.f / deltaMag);
        double d = deltaMag;
        CCollidableSphere sphere(zeus::CSphere(particle.x14_prevPos, particle.x10_radius), include);
        CCollisionInfo info;
        CGameCollision::DetectCollision_Cached_Moving(mgr, ccache, sphere, {}, filter, nearList, delta, bestId, info,
                                                      d);
        if (info.IsValid()) {
          needs2ndPass = true;
          switch (pass) {
          case 1: {
            particle.x3c_24_impactPending = true;
            float dot = delta.dot(info.GetNormalLeft());
            particle.x2c_impactResponseDelta = -0.125f * dot * deltaMag * info.GetNormalLeft();
            particle.x38_impactFrameVel = -dot * deltaMag;
            particle.x4_curPos += (0.0001f - (deltaMag - float(d)) * dot) * info.GetNormalLeft();
            break;
          }
          case 2:
            particle.x4_curPos = float(d - 0.0001) * delta + particle.x14_prevPos;
            break;
          default:
            break;
          }
        }
      } else if (!x68_27_continueSmallMovements) {
        particle.x4_curPos = particle.x14_prevPos;
      }
    }
  }

  return needs2ndPass;
}

void CRagDoll::SatisfyWorldConstraintsOnConstruction(CStateManager& mgr) {
  for (auto& particle : x4_particles)
    particle.x3c_24_impactPending = true;
  SatisfyWorldConstraints(mgr, 2);
  for (auto& particle : x4_particles)
    particle.x14_prevPos = particle.x4_curPos;
}

void CRagDoll::Verlet(float dt) {
  for (auto& particle : x4_particles) {
    zeus::CVector3f oldPos = particle.x4_curPos;
    particle.x4_curPos += (particle.x4_curPos - particle.x14_prevPos) * (particle.x3c_24_impactPending ? 0.9f : 1.f);
    particle.x4_curPos += particle.x20_velocity * (dt * dt);
    particle.x4_curPos += particle.x2c_impactResponseDelta;
    particle.x14_prevPos = oldPos;
    zeus::CVector3f deltaPos = particle.x4_curPos - particle.x14_prevPos;
    if (deltaPos.magSquared() > 4.f)
      particle.x4_curPos = deltaPos.normalized() * 2.f + particle.x14_prevPos;
    particle.x3c_24_impactPending = false;
    particle.x2c_impactResponseDelta = zeus::skZero3f;
  }
}

void CRagDoll::PreRender(const zeus::CVector3f& v, CModelData& mData) {
  // Empty
}

void CRagDoll::Update(CStateManager& mgr, float dt, float waterTop) {
  if (!x68_25_over || x68_27_continueSmallMovements) {
    AccumulateForces(dt, waterTop);
    Verlet(dt);
    SatisfyConstraints(mgr);
    ClearForces();
    CheckStatic(dt);
  }
}

void CRagDoll::Prime(CStateManager& mgr, const zeus::CTransform& xf, CModelData& mData) {
  zeus::CVector3f scale = mData.GetScale();
  CAnimData* aData = mData.GetAnimationData();
  aData->BuildPose();
  for (auto& particle : x4_particles) {
    if (particle.x0_id.IsValid()) {
      particle.x4_curPos = xf * (aData->GetPose().GetOffset(particle.x0_id) * scale);
    }
  }
  SatisfyWorldConstraints(mgr, 2);
  for (auto& particle : x4_particles) {
    particle.x3c_24_impactPending = false;
  }
  x68_26_primed = true;
}

} // namespace urde
