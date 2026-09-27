#include "MetroidPrime/CRagDoll.hpp"
#include "Collision/CCollidableSphere.hpp"
#include "Collision/CCollisionInfo.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "WorldFormat/CMetroidAreaCollider.hpp"

#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CHierarchyPoseBuilder.hpp"
#include "Kyoto/Math/CMath.hpp"

#include <float.h>

CRagDoll::CRagDoll(float normalGravity, float floatingGravity, float overTime, uint flags)
: mNormalGravity(normalGravity)
, mFloatingGravity(floatingGravity)
, mImpactCount(0)
, mOverTimer(overTime)
, mImpactVel(0.f)
, mAverageVel(CVector3f::Zero())
, mAngTimer(0.f)
, mPrevMovingSlowly(false)
, mOver(false)
, mPrimed(false)
, mContinueSmallMovements((flags & 1) != 0)
, mNoOverTimer((flags & 2) != 0)
, mNoAiCollision((flags & 4) != 0) {}

void CRagDoll::SatisfyWorldConstraintsOnConstruction(CStateManager& mgr) {
  for (int i = 0; i < mParticles.size(); ++i) {
    mParticles[i].mImpactPending = true;
  }
  SatisfyWorldConstraints(mgr, 2);
  for (int i = 0; i < mParticles.size(); ++i) {
    mParticles[i].mPrevPos = mParticles[i].mCurPos;
  }
}

void CRagDoll::Prime(CStateManager& mgr, const CTransform4f& xf, CModelData& mData) {
  const CVector3f& scale = CVector3f(mData.GetScale());
  CAnimData* animData = mData.AnimationData();
  animData->BuildPose();
  for (int i = 0; i < mParticles.size(); ++i) {
    CSegId id = mParticles[i].GetBone();
    if (id != CSegId::Invalid()) {
      mParticles[i].mCurPos =
          xf * CVector3f::ByElementMultiply(scale, animData->GetPose().GetOffset(id));
    }
  }
  SatisfyWorldConstraints(mgr, 2);
  for (int i = 0; i < mParticles.size(); ++i) {
    mParticles[i].mImpactPending = false;
  }
  mPrimed = true;
}

void CRagDoll::Verlet(float dt) {
  for (int i = 0; i < mParticles.size(); ++i) {
    CVector3f& pos = mParticles[i].mCurPos;
    CVector3f& prevPos = mParticles[i].mPrevPos;
    CVector3f oldPos = pos;
    float damping = mParticles[i].mImpactPending ? 0.9f : 1.f;
    pos += damping * (pos - prevPos);
    pos += dt * (dt * mParticles[i].mVelocity);
    pos += mParticles[i].mImpactResponseDelta;
    prevPos = oldPos;
    const CVector3f& delta = pos - prevPos;
    if (delta.MagSquared() > 4.f) {
      pos = prevPos + 2.f * CVector3f(delta).AsNormalized();
    }
    mParticles[i].mImpactPending = false;
    mParticles[i].mImpactResponseDelta = CVector3f::Zero();
  }
}

void CRagDoll::AccumulateForces(float dt, float waterTop) {
  float inverseDt = 1.f / dt;
  mAngTimer += dt;
  if (mAngTimer > 4.f) {
    mAngTimer -= 4.f;
  }
  float targetZ = 0.1f * CMath::FastSinR(1.5707964f * mAngTimer) + (waterTop - 0.2f);
  CVector3f centerOfVolume = CVector3f::Zero();
  float totalVolume = 0.f;
  for (int i = 0; i < mParticles.size(); ++i) {
    CRagDollParticle& particle = mParticles[i];
    float volume = particle.mRadius * particle.mRadius * particle.mRadius;
    totalVolume += volume;
    centerOfVolume += volume * particle.mCurPos;
    float fromTargetZ = particle.mCurPos.GetZ() - targetZ;
    float verticalAcc = mFloatingGravity;
    float termVelCoefficient = 0.f;
    if (CMath::AbsF(fromTargetZ) < 0.5f) {
      termVelCoefficient = 0.5f * fromTargetZ / 0.5f + 0.5f;
      verticalAcc *= -fromTargetZ / 0.5f;
    } else if (fromTargetZ > 0.f) {
      verticalAcc = mNormalGravity;
      termVelCoefficient = 1.f;
    }
    particle.mVelocity[kDZ] += verticalAcc;
    CVector3f velocity = inverseDt * (particle.mCurPos - particle.mPrevPos);
    float speed = velocity.Magnitude();
    if (speed > FLT_EPSILON) {
      CVector3f direction = (1.f / speed) * velocity;
      float drag = 0.75f * (1.2f * termVelCoefficient + 1000.f * (1.f - termVelCoefficient));
      float acceleration = drag * speed * speed / (8000.f * particle.mRadius);
      particle.mVelocity -= acceleration * direction;
    }
  }
  float inverseVolume = 1.f / totalVolume;
  CVector3f averageTorque = CVector3f::Zero();
  centerOfVolume *= inverseVolume;
  for (int i = 0; i < mParticles.size(); ++i) {
    CRagDollParticle& particle = mParticles[i];
    float volume = particle.mRadius * particle.mRadius * particle.mRadius;
    averageTorque += volume * CVector3f::Cross(particle.mCurPos - centerOfVolume,
                                               particle.mCurPos - particle.mPrevPos);
  }
  averageTorque *= inverseDt * inverseVolume;
  if (averageTorque.CanBeNormalized()) {
    for (int i = 0; i < mParticles.size(); ++i) {
      CRagDollParticle& particle = mParticles[i];
      particle.mVelocity -=
          25.f * CVector3f::Cross(averageTorque, particle.mCurPos - centerOfVolume);
    }
  }
}

bool CRagDoll::SatisfyWorldConstraints(CStateManager& mgr, int pass) {
  CAABox bounds = CAABox::MakeMaxInvertedBox();
  for (int i = 0; i < mParticles.size(); ++i) {
    if (pass == 1 || mParticles[i].mImpactPending) {
      float radius = mParticles[i].mRadius;
      CVector3f extent(radius, radius, radius);
      bounds.AccumulateBounds(mParticles[i].mPrevPos - extent);
      bounds.AccumulateBounds(mParticles[i].mPrevPos + extent);
      bounds.AccumulateBounds(mParticles[i].mCurPos - extent);
      bounds.AccumulateBounds(mParticles[i].mCurPos + extent);
    }
  }
  CAreaCollisionCache cache(bounds);
  CGameCollision::BuildAreaCollisionCache(mgr, cache);
  bool needsSecondPass = false;
  TUniqueId bestId = kInvalidUniqueId;
  CMaterialList include =
      mNoAiCollision ? CMaterialList(kMT_Solid) : CMaterialList(kMT_Solid, kMT_AIBlock);
  CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
      include, mNoAiCollision
                   ? CMaterialList(kMT_Character, kMT_Player, kMT_AIBlock, kMT_Occluder)
                   : CMaterialList(kMT_Character, kMT_Player));
  TEntityList nearList;
  mgr.BuildNearList(nearList, bounds, filter, nullptr);
  for (int i = 0; i < mParticles.size(); ++i) {
    if (pass == 1 || mParticles[i].mImpactPending) {
      CVector3f delta = mParticles[i].mCurPos - mParticles[i].mPrevPos;
      float magnitude = delta.Magnitude();
      if (magnitude > 0.0001f) {
        delta *= 1.f / magnitude;
        CSphere sphere(mParticles[i].mPrevPos, mParticles[i].mRadius);
        double distance = magnitude;
        CCollisionInfo info;
        CGameCollision::DetectCollision_Cached_Moving(
            mgr, cache, CCollidableSphere(sphere, include), CTransform4f::Identity(), filter,
            nearList, delta, bestId, info, distance);
        if (info.IsValid()) {
          needsSecondPass = true;
          switch (pass) {
          case 1: {
            mParticles[i].mImpactPending = true;
            float dot = CVector3f::Dot(delta, info.GetNormalLeft());
            mParticles[i].mImpactFrameVel = -dot * magnitude;
            mParticles[i].mImpactResponseDelta =
                magnitude * (-0.125f * dot) * info.GetNormalLeft();
            float penetration = (magnitude - static_cast< float >(distance)) * dot;
            mParticles[i].mCurPos += (0.0001f - penetration) * info.GetNormalLeft();
            break;
          }
          case 2:
            mParticles[i].mCurPos =
                mParticles[i].mPrevPos + static_cast< float >(distance - 0.0001) * delta;
            break;
          }
        }
      } else if (!mContinueSmallMovements) {
        mParticles[i].mCurPos = mParticles[i].mPrevPos;
      }
    }
  }
  return needsSecondPass;
}

void CRagDoll::SatisfyConstraints(CStateManager& mgr) {
  int i;
  for (i = 0; i < mLengthConstraints.size(); ++i) {
    mLengthConstraints[i].Update();
  }
  for (i = 0; i < mJointConstraints.size(); ++i) {
    mJointConstraints[i].Update();
  }
  for (i = 0; i < mPlaneConstraints.size(); ++i) {
    mPlaneConstraints[i].Update();
  }
  if (SatisfyWorldConstraints(mgr, 1)) {
    SatisfyWorldConstraints(mgr, 2);
  }
}

void CRagDoll::ClearForces() {
  for (int i = 0; i < mParticles.size(); ++i) {
    mParticles[i].mVelocity = CVector3f::Zero();
  }
}

void CRagDoll::CheckStatic(float dt) {
  mImpactCount = 0;
  mImpactVel = 0.f;
  float halfDt = 0.5f * dt;
  float threshold = halfDt * halfDt;
  mAverageVel = CVector3f::Zero();
  bool movingSlowly = true;
  for (int i = 0; i < mParticles.size(); ++i) {
    CVector3f delta = mParticles[i].mCurPos - mParticles[i].mPrevPos;
    mAverageVel += delta;
    if (delta.MagSquared() > threshold) {
      movingSlowly = false;
    }
    if (mParticles[i].mImpactPending) {
      ++mImpactCount;
      mImpactVel = CMath::Max(mImpactVel, mParticles[i].mImpactFrameVel);
    }
  }
  if (!mParticles.empty()) {
    mAverageVel *= 1.f / (dt * mParticles.size());
  }
  mImpactVel /= dt;
  if (!mNoOverTimer) {
    mOverTimer -= dt;
    if (mOverTimer <= 0.f) {
      mOver = true;
    }
  }
  if (movingSlowly && mPrevMovingSlowly) {
    mOver = true;
  }
  mPrevMovingSlowly = movingSlowly;
}

void CRagDoll::Update(CStateManager& mgr, float dt, float waterTop) {
  if (!IsOver() || WillContinueSmallMovements()) {
    AccumulateForces(dt, waterTop);
    Verlet(dt);
    SatisfyConstraints(mgr);
    ClearForces();
    CheckStatic(dt);
  }
}

void CRagDoll::CRagDollLengthConstraint::Update() {
  CVector3f& p1 = mP1->Position();
  CVector3f& p2 = mP2->Position();
  const CVector3f delta = p2 - p1;
  float magSquared = delta.MagSquared();
  float lengthSquared = mLength * mLength;
  bool solve = true;
  switch (mIneqType) {
  case 1:
    solve = magSquared < lengthSquared;
    break;
  case 2:
    solve = magSquared > lengthSquared;
    break;
  }
  if (solve) {
    const CVector3f correction = delta * (lengthSquared / (magSquared + lengthSquared) - 0.5f);
    p1 -= correction;
    p2 += correction;
  }
}

void CRagDoll::CRagDollJointConstraint::Update() {
  const CVector3f plane = CVector3f::Cross(mP3->GetPosition() - mP1->GetPosition(),
                                           mP2->GetPosition() - mP1->GetPosition());
  const CVector3f limb = mP5->GetPosition() - mP4->GetPosition();
  const CVector3f cross = CVector3f::Cross(limb, plane);
  if (cross.CanBeNormalized()) {
    const CVector3f normal = CVector3f::Cross(cross, limb).AsNormalized();
    const CVector3f delta = mP6->GetPosition() - mP5->GetPosition();
    float distance = CVector3f::Dot(delta, normal);
    if (distance > 0.f) {
      const CVector3f correction = 0.5f * distance * normal;
      mP6->Position() -= correction;
      mP5->Position() += correction;
    }
  }
}

void CRagDoll::CRagDollPlaneConstraint::Update() {
  const CVector3f normal = (mP2->GetPosition() - mP1->GetPosition()).AsNormalized();
  const CVector3f delta = mP4->GetPosition() - mP3->GetPosition();
  float distance = CVector3f::Dot(normal, delta);
  if (distance < 0.f) {
    const CVector3f correction = 0.5f * distance * normal;
    mP4->Position() -= correction;
    mP5->Position() += correction;
  }
}

void CRagDoll::AddParticle(const CSegId& id, const CVector3f& prevPos, const CVector3f& curPos,
                           float radius) {
  mParticles.push_back(CRagDollParticle(id, curPos, radius, prevPos));
}

void CRagDoll::AddLengthConstraint(int i1, int i2) {
  mLengthConstraints.push_back(CRagDollLengthConstraint(
      &mParticles[i1], &mParticles[i2],
      (mParticles[i1].GetPosition() - mParticles[i2].GetPosition()).Magnitude(), 0));
}

void CRagDoll::AddMinLengthConstraint(int i1, int i2, float length) {
  mLengthConstraints.push_back(
      CRagDollLengthConstraint(&mParticles[i1], &mParticles[i2], length, 1));
}

void CRagDoll::AddMaxLengthConstraint(int i1, int i2, float length) {
  mLengthConstraints.push_back(
      CRagDollLengthConstraint(&mParticles[i1], &mParticles[i2], length, 2));
}

void CRagDoll::AddJointConstraint(int i1, int i2, int i3, int i4, int i5, int i6) {
  mJointConstraints.push_back(CRagDollJointConstraint(&mParticles[i1], &mParticles[i2],
                                                         &mParticles[i3], &mParticles[i4],
                                                         &mParticles[i5], &mParticles[i6]));
}

CQuaternion CRagDoll::BoneAlign(CHierarchyPoseBuilder& pb, const CCharLayoutInfo& charInfo, int i1,
                                int i2, const CQuaternion& q) {
  CVector3f fromParent = charInfo.GetFromParentUnrotated(mParticles[i2].GetBone());
  CVector3f delta = mParticles[i2].mCurPos - mParticles[i1].GetPosition();
  delta = CQuaternion::ScalarVector(q.GetScalar(), -q.GetVector()).Transform(delta);
  CQuaternion rotation = CQuaternion::ShortestRotationArc(fromParent, delta);
  pb.Insert(mParticles[i1].GetBone(), rotation);
  return rotation;
}

CAABox CRagDoll::CalculateRenderBounds() const {
  CVector3f min(FLT_MAX, FLT_MAX, FLT_MAX);
  CVector3f max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
  for (int i = 0; i < mParticles.size(); ++i) {
    for (int j = 0; j < 3; ++j) {
      min[j] = CMath::Min(min[j], mParticles[i].GetPosition()[j] - mParticles[i].GetRadius());
      max[j] = CMath::Max(max[j], mParticles[i].GetPosition()[j] + mParticles[i].GetRadius());
    }
  }
  return CAABox(min, max);
}

void CRagDoll::PreRender(const CVector3f& pos, CModelData& mData) {}
