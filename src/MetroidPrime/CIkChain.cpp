#include "MetroidPrime/CIkChain.hpp"

#include "MetroidPrime/CAnimData.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"

void CIkChain::Solve(CQuaternion& q1, CQuaternion& q2, const CVector3f& pos) {
  const float mag = pos.Magnitude();
  float secondCos =
      (mP1BoneLength * mP1BoneLength + mag * mag - mP2p1Length * mP2p1Length) /
      (2.f * mag * mP1BoneLength);
  secondCos = CMath::Clamp(-1.f, secondCos, 1.f);
  float secondAngle = acosf(secondCos);
  float firstCos =
      (mP2p1Length * mP2p1Length + (mag * mag - mP1BoneLength * mP1BoneLength)) /
      (2.f * mag * mP2p1Length);
  firstCos = CMath::Clamp(-1.f, firstCos, 1.f);
  float firstAngle = acosf(firstCos);

  CVector3f boneDir = q2.Transform(mP1BoneDir);
  CVector3f axis = CVector3f::Cross(mP2p1Dir, boneDir);
  float axisMag = axis.Magnitude();
  axis *= 1.f / axisMag;
  axisMag = CMath::Min(axisMag, 1.f);
  float angle = static_cast< float >(asin(axisMag));
  if (CVector3f::Dot(mP2p1Dir, boneDir) < 0.f) {
    angle = M_PIF - angle;
  }
  CQuaternion rotation =
      CQuaternion::AxisAngle(CUnitVector3f(axis, CUnitVector3f::kN_No),
                             CRelAngle::FromRadians(firstAngle + secondAngle - angle));
  q2 = rotation * q2;

  CVector3f current =
      q1.Transform(mP2p1Length * mP2p1Dir + mP1BoneLength * q2.Transform(mP1BoneDir));
  CVector3f target = (1.f / mag) * q1.Transform(pos);
  axis = CVector3f::Cross(current.AsNormalized(), target);
  axisMag = axis.Magnitude();
  axis *= 1.f / axisMag;
  axisMag = CMath::Min(axisMag, 1.f);
  angle = static_cast< float >(asin(axisMag));
  if (CVector3f::Dot(current, target) < 0.f) {
    angle = M_PIF - angle;
  }
  CQuaternion targetRotation = CQuaternion::AxisAngle(CUnitVector3f(axis, CUnitVector3f::kN_No),
                                                      CRelAngle::FromRadians(angle));
  q1 = targetRotation * q1;
}

void CIkChain::Activate(const CAnimData& data, CSegId bone, const CTransform4f& xf) {
  mBone = bone;
  const CHierarchyPoseBuilder& builder = data.GetPoseBuilder();
  mP1 = builder.CharLayoutInfo()->GetOriginalParent(mBone);
  if (mP1 != CSegId(2)) {
    mP2 = builder.CharLayoutInfo()->GetOriginalParent(mP1);
    mP2p1Dir = builder.CharLayoutInfo()->GetFromParentUnrotated(mP1);
    mP2p1Length = mP2p1Dir.Magnitude();
    mP2p1Dir *= 1.f / mP2p1Length;
    mP1BoneDir = builder.CharLayoutInfo()->GetFromParentUnrotated(mBone);
    mP1BoneLength = mP1BoneDir.Magnitude();
    mP1BoneDir *= 1.f / mP1BoneLength;
    mHoldPos = xf.GetTranslation();
    mHoldRot = CQuaternion::FromMatrix(xf);
    mActivated = true;
  }
}

void CIkChain::PreRender(CAnimData& data, const CTransform4f& xf, const CVector3f& scale) {
  if (mTime > 0.f) {
    CTransform4f p2Xf = data.GetLocatorTransform(mP2, nullptr);
    CVector3f localDelta = xf.TransposeMultiply(mHoldPos);
    localDelta[kDX] /= scale[kDX];
    localDelta[kDY] /= scale[kDY];
    localDelta[kDZ] /= scale[kDZ];
    localDelta = p2Xf.TransposeMultiply(localDelta);
    CHierarchyPoseBuilder& builder = data.PoseBuilder();
    CQuaternion p2Rot = builder.GetSegRotation(mP2);
    CQuaternion p1Rot = builder.GetSegRotation(mP1);
    CQuaternion boneRot = builder.GetSegRotation(mBone);
    CQuaternion baseRot = CQuaternion::FromMatrix(xf * p2Xf) * p2Rot.BuildInverted();
    CQuaternion newP2Rot = p2Rot;
    CQuaternion newP1Rot = p1Rot;
    Solve(newP2Rot, newP1Rot, localDelta);
    CQuaternion rotation = baseRot * newP2Rot * newP1Rot;
    CQuaternion newBoneRot = rotation.BuildInverted() * mHoldRot;
    if (mTime < 1.f) {
      newP2Rot = CQuaternion::SlerpLocal(p2Rot, newP2Rot, mTime);
      newP1Rot = CQuaternion::SlerpLocal(p1Rot, newP1Rot, mTime);
      newBoneRot = CQuaternion::SlerpLocal(boneRot, newBoneRot, mTime);
    }
    builder.Insert(mP2, newP2Rot);
    builder.Insert(mP1, newP1Rot);
    builder.Insert(mBone, newBoneRot);
    data.SetPoseBuilderValid(false);
  }
}

void CIkChain::Deactivate() { mActivated = false; }

void CIkChain::Update(float dt) {
  if (mActivated) {
    mTime = CMath::Min(mTime + dt, 1.f);
  } else {
    mTime = CMath::Max(0.f, mTime - dt);
  }
}
