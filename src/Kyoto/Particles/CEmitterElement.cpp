#include "Kyoto/Particles/CEmitterElement.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CVector3f.hpp"

CEESimpleEmitter::CEESimpleEmitter(CVectorElement* pos, CVectorElement* vel)
: mPos(pos), mVel(vel) {}

CEESimpleEmitter::~CEESimpleEmitter() {
  delete mPos;
  delete mVel;
}

bool CEESimpleEmitter::GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const {
  mPos->GetValue(frame, pPos);
  if (mVel != NULL) {
    mVel->GetValue(frame, pVel);
  } else {
    pVel = CVector3f::Zero();
  }

  return false;
}

CVESphere::CVESphere(CVectorElement* origin, CRealElement* radius, CRealElement* velocityMag)
: mSphereOrigin(origin), mSphereRadius(radius), mVelocityMag(velocityMag) {}

CVESphere::~CVESphere() {
  delete mSphereOrigin;
  delete mSphereRadius;
  delete mVelocityMag;
}
bool CVESphere::GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const {
  CVector3f origin = CVector3f::Zero();
  float radius;
  float mag;
  mSphereOrigin->GetValue(frame, origin);
  mSphereRadius->GetValue(frame, radius);

  CVector3f normVec1 = CVector3f(CRandom16::GetRandomNumber()->Range(-100, 100),
                                 CRandom16::GetRandomNumber()->Range(-100, 100),
                                 CRandom16::GetRandomNumber()->Range(-100, 100));
  normVec1 = (normVec1.CanBeNormalized() ? (0.01f * normVec1).AsNormalized() : (0.01f * normVec1));
  pPos = origin + (normVec1 * radius);

  CVector3f offset = (pPos - origin);
  CVector3f direction = offset.CanBeNormalized() ? offset.AsNormalized() : offset;
  mVelocityMag->GetValue(frame, mag);
  pVel = mag * direction;

  return false;
}

CVEAngleSphere::CVEAngleSphere(CVectorElement* origin, CRealElement* radius,
                               CRealElement* velocityMag, CRealElement* angleXBias,
                               CRealElement* angleYBias, CRealElement* angleXRange,
                               CRealElement* angleYRange)
: mSphereOrigin(origin)
, mSphereRadius(radius)
, mVelocityMag(velocityMag)
, mAngleXBias(angleXBias)
, mAngleYBias(angleYBias)
, mAngleXRange(angleXRange)
, mAngleYRange(angleYRange) {}

CVEAngleSphere::~CVEAngleSphere() {
  delete mSphereOrigin;
  delete mSphereRadius;
  delete mVelocityMag;
  delete mAngleXBias;
  delete mAngleYBias;
  delete mAngleXRange;
  delete mAngleYRange;
}

bool CVEAngleSphere::GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const {
  CVector3f origin = CVector3f::Zero();
  float radius;
  float mag;
  float xBias;
  float xRange;
  float yBias;
  float yRange;
  mSphereOrigin->GetValue(frame, origin);
  mSphereRadius->GetValue(frame, radius);

  mAngleXBias->GetValue(frame, xBias);

  mAngleYBias->GetValue(frame, yBias);

  mAngleXRange->GetValue(frame, xRange);

  mAngleYRange->GetValue(frame, yRange);

  xBias += ((0.5f * xRange) - (CRandom16::GetRandomNumber()->Float() * xRange));
  xBias *= (M_PIF / 180.f);

  yBias += ((0.5f * yRange) - (CRandom16::GetRandomNumber()->Float() * yRange));
  yBias *= (M_PIF / 180.f);

  CVector3f vec(-CMath::FastSinR(yBias) * CMath::FastCosR(xBias), CMath::FastSinR(xBias),
                CMath::FastCosR(xBias) * CMath::FastCosR(yBias));
  pPos = origin + (radius * vec);
  CVector3f dir = (pPos - origin).AsNormalized();
  mVelocityMag->GetValue(frame, mag);
  pVel = mag * dir;

  return false;
}
