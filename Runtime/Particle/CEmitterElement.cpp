#include "CEmitterElement.hpp"
#include "CRandom16.hpp"

/* Documentation at: http://www.metroid2002.com/retromodding/wiki/Particle_Script#Emitter_Elements */

namespace urde {

bool CEESimpleEmitter::GetValue(int frame, zeus::CVector3f& pPos, zeus::CVector3f& pVel) const {
  x4_loc->GetValue(frame, pPos);

  if (x8_vec)
    x8_vec->GetValue(frame, pVel);
  else
    pVel = zeus::CVector3f();

  return false;
}

bool CVESphere::GetValue(int frame, zeus::CVector3f& pPos, zeus::CVector3f& pVel) const {
  zeus::CVector3f a;
  x4_sphereOrigin->GetValue(frame, a);
  float b;
  x8_sphereRadius->GetValue(frame, b);
  CRandom16* rand = CRandom16::GetRandomNumber();
  int rand1 = rand->Range(-100, 100);
  int rand2 = rand->Range(-100, 100);
  int rand3 = rand->Range(-100, 100);

  zeus::CVector3f normVec1 =
      zeus::CVector3f(0.0099999998f * float(rand3), 0.0099999998f * float(rand2), 0.0099999998f * float(rand1));
  if (normVec1.canBeNormalized())
    normVec1.normalize();

  pPos = b * normVec1 + a;

  zeus::CVector3f normVec2 = (pPos - a);
  if (normVec2.canBeNormalized())
    normVec2.normalize();

  float c;
  xc_velocityMag->GetValue(frame, c);
  pVel = c * normVec2;

  return false;
}

bool CVEAngleSphere::GetValue(int frame, zeus::CVector3f& pPos, zeus::CVector3f& pVel) const {
  zeus::CVector3f a;
  x4_sphereOrigin->GetValue(frame, a);

  float b, d, e, f, g;
  x8_sphereRadius->GetValue(frame, b);
  x10_angleXBias->GetValue(frame, d);
  x14_angleYBias->GetValue(frame, e);
  x18_angleXRange->GetValue(frame, f);
  x1c_angleYRange->GetValue(frame, g);
  CRandom16* rand = CRandom16::GetRandomNumber();
  d = zeus::degToRad(d + ((0.5f * (f * rand->Float())) - f));
  e = zeus::degToRad(e + ((0.5f * (g * rand->Float())) - g));

  float cosD = std::cos(d);
  pPos.x() = a.x() + (b * (-std::sin(e) * cosD));
  pPos.y() = a.y() + (b * std::sin(d));
  pPos.z() = a.z() + (b * (cosD * cosD));
  zeus::CVector3f normVec = (pPos - a).normalized();

  float c;
  xc_velocityMag->GetValue(frame, c);
  pVel = c * normVec;
  return false;
}

} // namespace urde
