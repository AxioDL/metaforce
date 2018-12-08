#include "Character/CIkChain.hpp"
#include "Character/CAnimData.hpp"

namespace urde {

void CIkChain::Update(float dt) {
  if (x44_24_activated)
    x40_time = zeus::min(x40_time + dt, 1.f);
  else
    x40_time = zeus::max(0.f, x40_time - dt);
}

void CIkChain::Deactivate() { x44_24_activated = false; }

void CIkChain::Activate(const CAnimData& animData, const CSegId& segId, const zeus::CTransform& xf) {
  // const CHierarchyPoseBuilder& posBuilder = animData.GetPoseBuilder();
  x0_ = segId;

  // const TLockedToken<CCharLayoutInfo>& info = posBuilder.CharLayoutInfo();
}

void CIkChain::PreRender(CAnimData& animData, const zeus::CTransform& xf, const zeus::CVector3f& scale) {}

void CIkChain::Solve(zeus::CQuaternion& q1, zeus::CQuaternion& q2, const zeus::CVector3f& vec) {
  const float mag = vec.magnitude();
  const float magSq = mag * mag;
  const float twoMag = (2.0f * mag);
  float f29 = std::acos(zeus::clamp(-1.f, (((x20_ * magSq) + x20_) - (x1c_ * x1c_)) / (twoMag * x20_), 1.f));
  float f30 = std::acos(zeus::clamp(-1.f, ((x1c_ * (magSq - (x20_ * x20_))) + x1c_) / (twoMag * x1c_), 1.f));

  zeus::CVector3f vecA = q2.transform(x10_);
  zeus::CVector3f crossVecA = x4_.cross(vecA);
  float crossAMag = crossVecA.magnitude();
  crossVecA *= zeus::CVector3f(1.f / crossVecA.magnitude());
  float angle = std::asin(zeus::min(crossAMag, 1.f));
  if (x4_.dot(vecA) < 0.f)
    angle = M_PIF - angle;
  q2 = zeus::CQuaternion::fromAxisAngle(crossVecA, (f30 + f29) - angle) * q2;
  zeus::CVector3f v1 = q1.transform((x1c_ * x4_) + (x20_ * q2.transform(x10_)));
  zeus::CVector3f v2 = q1.transform(vec);
  zeus::CVector3f crossVecB = v1.normalized().cross((1.f / mag) * v2);
  angle = std::asin(zeus::min(crossVecB.magnitude(), 1.f));
  if (v1.dot((1.f / mag) * v2) < 0.f)
    angle = M_PIF - angle;

  q1 = zeus::CQuaternion::fromAxisAngle(crossVecB * (1.f / crossVecB.magnitude()), angle) * q1;
}
} // namespace urde
