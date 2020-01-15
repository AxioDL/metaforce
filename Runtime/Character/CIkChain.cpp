#include "Runtime/Character/CIkChain.hpp"

#include "Runtime/Character/CAnimData.hpp"

namespace urde {

void CIkChain::Update(float dt) {
  if (x44_24_activated)
    x40_time = zeus::min(x40_time + dt, 1.f);
  else
    x40_time = zeus::max(0.f, x40_time - dt);
}

void CIkChain::Deactivate() { x44_24_activated = false; }

void CIkChain::Activate(const CAnimData& animData, const CSegId& segId, const zeus::CTransform& xf) {
  x0_bone = segId;
  auto info = animData.GetPoseBuilder().CharLayoutInfo();
  x1_p1 = info->GetRootNode()->GetBoneMap()[x0_bone].x0_parentId;
  if (x1_p1 != 2) {
    x2_p2 = info->GetRootNode()->GetBoneMap()[x1_p1].x0_parentId;
    x4_p2p1Dir = info->GetFromParentUnrotated(x1_p1);
    x1c_p2p1Length = x4_p2p1Dir.magnitude();
    x4_p2p1Dir = x4_p2p1Dir / x1c_p2p1Length;
    x10_p1BoneDir = info->GetFromParentUnrotated(x0_bone);
    x20_p1BoneLength = x10_p1BoneDir.magnitude();
    x10_p1BoneDir = x10_p1BoneDir / x20_p1BoneLength;
    x34_holdPos = xf.origin;
    x24_holdRot = zeus::CQuaternion(xf.basis);
    x44_24_activated = true;
  }
}

void CIkChain::PreRender(CAnimData& animData, const zeus::CTransform& xf, const zeus::CVector3f& scale) {
  if (x40_time > 0.f) {
    zeus::CTransform p2Xf = animData.GetLocatorTransform(x2_p2, nullptr);
    zeus::CVector3f localDelta = xf.transposeRotate(x34_holdPos - xf.origin);
    localDelta /= scale;
    localDelta = p2Xf.transposeRotate(localDelta - p2Xf.origin);
    zeus::CQuaternion p2Rot = animData.PoseBuilder().GetTreeMap()[x2_p2].x4_rotation;
    zeus::CQuaternion p1Rot = animData.PoseBuilder().GetTreeMap()[x1_p1].x4_rotation;
    zeus::CQuaternion boneRot = animData.PoseBuilder().GetTreeMap()[x0_bone].x4_rotation;
    zeus::CQuaternion newP2Rot = p2Rot;
    zeus::CQuaternion newP1Rot = p1Rot;
    Solve(newP2Rot, newP1Rot, localDelta);
    zeus::CQuaternion newBoneRot =
      (zeus::CQuaternion((xf * p2Xf).basis) * p2Rot.inverse() * newP2Rot * newP1Rot).inverse() * x24_holdRot;
    if (x40_time < 1.f) {
      newP2Rot = zeus::CQuaternion::slerpShort(p2Rot, newP2Rot, x40_time);
      newP1Rot = zeus::CQuaternion::slerpShort(p1Rot, newP1Rot, x40_time);
      newBoneRot = zeus::CQuaternion::slerpShort(boneRot, newBoneRot, x40_time);
    }
    animData.PoseBuilder().GetTreeMap()[x2_p2].x4_rotation = newP2Rot;
    animData.PoseBuilder().GetTreeMap()[x1_p1].x4_rotation = newP1Rot;
    animData.PoseBuilder().GetTreeMap()[x0_bone].x4_rotation = newBoneRot;
    animData.MarkPoseDirty();
  }
}

void CIkChain::Solve(zeus::CQuaternion& q1, zeus::CQuaternion& q2, const zeus::CVector3f& vec) {
  const float mag = vec.magnitude();
  const float magSq = mag * mag;
  const float twoMag = (2.0f * mag);
  float f29 = std::acos(zeus::clamp(-1.f, (((x20_p1BoneLength * magSq) + x20_p1BoneLength) -
    (x1c_p2p1Length * x1c_p2p1Length)) / (twoMag * x20_p1BoneLength), 1.f));
  float f30 = std::acos(zeus::clamp(-1.f, ((x1c_p2p1Length * (magSq - (x20_p1BoneLength * x20_p1BoneLength))) +
    x1c_p2p1Length) / (twoMag * x1c_p2p1Length), 1.f));

  zeus::CVector3f vecA = q2.transform(x10_p1BoneDir);
  zeus::CVector3f crossVecA = x4_p2p1Dir.cross(vecA);
  float crossAMag = crossVecA.magnitude();
  crossVecA *= zeus::CVector3f(1.f / crossVecA.magnitude());
  float angle = std::asin(zeus::min(crossAMag, 1.f));
  if (x4_p2p1Dir.dot(vecA) < 0.f)
    angle = M_PIF - angle;
  q2 = zeus::CQuaternion::fromAxisAngle(crossVecA, (f30 + f29) - angle) * q2;
  zeus::CVector3f v1 = q1.transform((x1c_p2p1Length * x4_p2p1Dir) + (x20_p1BoneLength * q2.transform(x10_p1BoneDir)));
  zeus::CVector3f v2 = q1.transform(vec);
  zeus::CVector3f crossVecB = v1.normalized().cross((1.f / mag) * v2);
  angle = std::asin(zeus::min(crossVecB.magnitude(), 1.f));
  if (v1.dot((1.f / mag) * v2) < 0.f)
    angle = M_PIF - angle;

  q1 = zeus::CQuaternion::fromAxisAngle(crossVecB * (1.f / crossVecB.magnitude()), angle) * q1;
}
} // namespace urde
