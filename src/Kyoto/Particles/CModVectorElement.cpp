#include "Kyoto/Particles/CModVectorElement.hpp"

#include "Kyoto/CRandom16.hpp"

CMVEConstant::CMVEConstant(CRealElement* a, CRealElement* b, CRealElement* c)
: x4_x(a), x8_y(b), xc_z(c) {}

CMVEConstant::~CMVEConstant() {
  delete x4_x;
  delete x8_y;
  delete xc_z;
}

bool CMVEConstant::GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const {
  float x, y, z;
  x4_x->GetValue(frame, x);
  x8_y->GetValue(frame, y);
  xc_z->GetValue(frame, z);
  pVel = CVector3f(x, y, z);
  return false;
}

CMVEFastConstant::CMVEFastConstant(float a, float b, float c) : x4_val(a, b, c) {}

CMVEFastConstant::~CMVEFastConstant() {}

bool CMVEFastConstant::GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const {
  pVel = x4_val;
  return false;
}

CMVEGravity::CMVEGravity(CVectorElement* a) : x4_a(a) {}

CMVEGravity::~CMVEGravity() { delete x4_a; }

bool CMVEGravity::GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const {
  CVector3f grav(0.f, 0.f, 0.f);
  x4_a->GetValue(frame, grav);
  pVel += grav;
  return false;
}

CMVEWind::CMVEWind(CVectorElement* velocity, CRealElement* factor)
: x4_velocity(velocity), x8_factor(factor) {}

CMVEWind::~CMVEWind() {
  delete x4_velocity;
  delete x8_factor;
}

bool CMVEWind::GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const {
  CVector3f wVel(0.f, 0.f, 0.f);
  x4_velocity->GetValue(frame, wVel);
  CVector3f diff = wVel - pVel;
  float factor;
  x8_factor->GetValue(frame, factor);
  diff *= factor;
  pVel += diff;
  return false;
}

CMVEExplode::CMVEExplode(CRealElement* a, CRealElement* b) : x4_a(a), x8_b(b) {}

CMVEExplode::~CMVEExplode() {
  delete x4_a;
  delete x8_b;
}

bool CMVEExplode::GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const {
  if (frame == 0) {
    float x, y, z;
    do {
      x = CRandom16::GetRandomNumber()->Float() - 0.5f;
      y = CRandom16::GetRandomNumber()->Float() - 0.5f;
      z = CRandom16::GetRandomNumber()->Float() - 0.5f;
    } while (x * x + y * y + z * z > 1.f);
    float a;
    x4_a->GetValue(frame, a);
    CVector3f vec(x, y, z);
    vec.Normalize();
    vec *= a;
    pVel = vec;
  } else {
    float b;
    x8_b->GetValue(frame, b);
    pVel *= b;
  }
  return false;
}

CMVETimeChain::CMVETimeChain(CModVectorElement* a, CModVectorElement* b, CIntElement* c)
: x4_a(a), x8_b(b), xc_swFrame(c) {}

CMVETimeChain::~CMVETimeChain() {
  delete x4_a;
  delete x8_b;
  delete xc_swFrame;
}

bool CMVETimeChain::GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const {
  int v;
  xc_swFrame->GetValue(frame, v);
  if (frame < v) {
    return x4_a->GetValue(frame, pVel, pPos);
  } else {
    return x8_b->GetValue(frame - v, pVel, pPos);
  }
}

CMVEPulse::CMVEPulse(CIntElement* a, CIntElement* b, CModVectorElement* c, CModVectorElement* d)
: x4_aDuration(a), x8_bDuration(b), xc_aVal(c), x10_bVal(d) {}

CMVEPulse::~CMVEPulse() {
  delete x4_aDuration;
  delete x8_bDuration;
  delete xc_aVal;
  delete x10_bVal;
}

bool CMVEPulse::GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const {
  int a, b;
  x4_aDuration->GetValue(frame, a);
  x8_bDuration->GetValue(frame, b);
  int cv = a + b + 1;
  if (cv < 0) {
    cv = 1;
  }

  if (b >= 1) {
    if (frame % cv > a) {
      x10_bVal->GetValue(frame, pVel, pPos);
    } else {
      xc_aVal->GetValue(frame, pVel, pPos);
    }
  } else {
    xc_aVal->GetValue(frame, pVel, pPos);
  }
  return false;
}

CMVEImplosion::CMVEImplosion(CVectorElement* a, CRealElement* b, CRealElement* c, CRealElement* d,
                             bool e)
: x4_implPoint(a), x8_magScale(b), xc_maxMag(c), x10_minMag(d), x14_enableMinMag(e) {}

CMVEImplosion::~CMVEImplosion() {
  delete x4_implPoint;
  delete x8_magScale;
  delete xc_maxMag;
  delete x10_minMag;
}

bool CMVEImplosion::GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const {
  CVector3f av(0.f, 0.f, 0.f);
  x4_implPoint->GetValue(frame, av);

  CVector3f dv = av - pPos;
  float dvm = dv.Magnitude();

  float c;
  xc_maxMag->GetValue(frame, c);
  if (dvm > c) {
    return false;
  }

  float d;
  x10_minMag->GetValue(frame, d);
  if (x14_enableMinMag && dvm < d) {
    return true;
  }

  if (dvm == 0.f) {
    return false;
  }

  CVector3f dvs = dv / dvm;
  float b;
  x8_magScale->GetValue(frame, b);
  pVel += b * dvs;
  return false;
}

CMVELinearImplosion::CMVELinearImplosion(CVectorElement* a, CRealElement* b, CRealElement* c,
                                         CRealElement* d, bool e)
: x4_implPoint(a), x8_magScale(b), xc_maxMag(c), x10_minMag(d), x14_enableMinMag(e) {}

CMVELinearImplosion::~CMVELinearImplosion() {
  delete x4_implPoint;
  delete x8_magScale;
  delete xc_maxMag;
  delete x10_minMag;
}

bool CMVELinearImplosion::GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const {
  CVector3f av(0.f, 0.f, 0.f);
  x4_implPoint->GetValue(frame, av);

  CVector3f dv = av - pPos;
  float dvm = dv.Magnitude();

  float c;
  xc_maxMag->GetValue(frame, c);
  if (dvm > c) {
    return false;
  }

  float d;
  x10_minMag->GetValue(frame, d);
  if (x14_enableMinMag && dvm < d) {
    return true;
  }

  if (dvm == 0.f) {
    return false;
  }

  CVector3f dvs = dv / dvm;
  float b;
  x8_magScale->GetValue(frame, b);
  pVel = b * dvs;
  return false;
}

CMVEExponentialImplosion::CMVEExponentialImplosion(CVectorElement* a, CRealElement* b,
                                                   CRealElement* c, CRealElement* d, bool e)
: x4_implPoint(a), x8_magScale(b), xc_maxMag(c), x10_minMag(d), x14_enableMinMag(e) {}

CMVEExponentialImplosion::~CMVEExponentialImplosion() {
  delete x4_implPoint;
  delete x8_magScale;
  delete xc_maxMag;
  delete x10_minMag;
}

bool CMVEExponentialImplosion::GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const {
  CVector3f av(0.f, 0.f, 0.f);
  x4_implPoint->GetValue(frame, av);

  CVector3f dv = av - pPos;
  float dvm = dv.Magnitude();

  float c;
  xc_maxMag->GetValue(frame, c);
  if (dvm > c) {
    return false;
  }

  float d;
  x10_minMag->GetValue(frame, d);
  if (x14_enableMinMag && dvm < d) {
    return true;
  }

  if (dvm == 0.f) {
    return false;
  }

  CVector3f dvs = dv / dvm;
  float b;
  x8_magScale->GetValue(frame, b);
  pVel += dvm * (b * dvs);
  return false;
}

CMVESwirl::CMVESwirl(CVectorElement* a, CVectorElement* b, CRealElement* c, CRealElement* d)
: x4_helixPoint(a), x8_curveBinormal(b), xc_filterGain(c), x10_tangentialVelocity(d) {}

CMVESwirl::~CMVESwirl() {
  delete x4_helixPoint;
  delete x8_curveBinormal;
  delete xc_filterGain;
  delete x10_tangentialVelocity;
}

bool CMVESwirl::GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const {
  CVector3f a(0.f, 0.f, 0.f);
  CVector3f b(0.f, 0.f, 0.f);
  x4_helixPoint->GetValue(frame, a);
  x8_curveBinormal->GetValue(frame, b);

  CVector3f posToOrigin = a - pPos;
  CVector3f posToHelix = posToOrigin - CVector3f::Dot(posToOrigin, b) * b;

  float c = 0.f;
  float d = 0.f;
  xc_filterGain->GetValue(frame, c);
  x10_tangentialVelocity->GetValue(frame, d);

#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
  const CVector3f tangent = CVector3f::Cross(b, posToHelix) * d;
  pVel = (b * CVector3f::Dot(b, pVel) + tangent) * c + (1.f - c) * pVel;
#else
  pVel = (b * CVector3f::Dot(b, pVel) + d * CVector3f::Cross(b, posToHelix)) * c +
         (1.f - c) * pVel;
#endif
  return false;
}

CMVEBounce::CMVEBounce(CVectorElement* planePoint, CVectorElement* planeNormal,
                       CRealElement* friction, CRealElement* restitution, bool e)
: x4_planePoint(planePoint)
, x8_planeNormal(planeNormal)
, xc_friction(friction)
, x10_restitution(restitution)
, x14_planePrecomputed(false)
, x15_dieOnPenetrate(e)
, x18_planeValidatedNormal(0.f, 0.f, 0.f)
, x24_planeD(0.f) {
  if (planePoint && planeNormal && planePoint->IsFastConstant() && planeNormal->IsFastConstant()) {
    // Precompute Hesse normal form of plane (for penetration testing)
    // https://en.wikipedia.org/wiki/Hesse_normal_form
    x14_planePrecomputed = true;
    planeNormal->GetValue(0, x18_planeValidatedNormal);

    if (x18_planeValidatedNormal.MagSquared() > 0.f) {
      x18_planeValidatedNormal.Normalize();
    }
    CVector3f a(0.f, 0.f, 0.f);
    planePoint->GetValue(0, a);
    x24_planeD = CVector3f::Dot(x18_planeValidatedNormal, a);
  }
}

CMVEBounce::~CMVEBounce() {
  delete x4_planePoint;
  delete x8_planeNormal;
  delete xc_friction;
  delete x10_restitution;
}

bool CMVEBounce::GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const {
  if (!x14_planePrecomputed) {
    // Compute Hesse normal form of plane (for penetration testing)
    x8_planeNormal->GetValue(frame, x18_planeValidatedNormal);
    x18_planeValidatedNormal.Normalize();

    CVector3f a(0.f, 0.f, 0.f);
    x4_planePoint->GetValue(frame, a);

    x24_planeD = CVector3f::Dot(x18_planeValidatedNormal, a);
  }

  float dot = CVector3f::Dot(x18_planeValidatedNormal, pPos);
  if (dot - x24_planeD > 0.f) {
    return false;
  } else if (x15_dieOnPenetrate) {
    return true;
  }

  // Deflection event
  if (CVector3f::Dot(x18_planeValidatedNormal, pVel) < 0.f) {
    float pd = CVector3f::Dot(pPos - pVel, x18_planeValidatedNormal) - x24_planeD;
    float pn = CVector3f::Dot(pVel, x18_planeValidatedNormal);
    pPos = pPos + pVel * (-pd / pn - 1.f);

    float d = 0.f;
    x10_restitution->GetValue(frame, d);
    pVel -= d * pVel;

    float c = 0.f;
    xc_friction->GetValue(frame, c);
    pVel -= (c + 1.f) * CVector3f::Dot(x18_planeValidatedNormal, pVel) * x18_planeValidatedNormal;
  }
  return false;
}

CMVESetPosition::CMVESetPosition(CVectorElement* a) : x4_a(a) {}

CMVESetPosition::~CMVESetPosition() { delete x4_a; }

bool CMVESetPosition::GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const {
  x4_a->GetValue(frame, pPos);
  return false;
}
