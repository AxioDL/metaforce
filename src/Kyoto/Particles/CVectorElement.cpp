#include "Kyoto/Particles/CVectorElement.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Particles/CParticleGlobals.hpp"
#include "Kyoto/Particles/IElement.hpp"
#include "rstl/math.hpp"

CVEConstant::CVEConstant(CRealElement* x, CRealElement* y, CRealElement* z)
: mX(x), mY(y), mZ(z) {}

CVEConstant::~CVEConstant() {
  delete mX;
  delete mY;
  delete mZ;
}

bool CVEConstant::GetValue(int frame, CVector3f& valOut) const {
  float x, y, z;
  mX->GetValue(frame, x);
  mY->GetValue(frame, y);
  mZ->GetValue(frame, z);
  valOut = CVector3f(x, y, z);
  return false;
}

CVEFastConstant::CVEFastConstant(float x, float y, float z) : mVal(x, y, z) {}

CVEFastConstant::~CVEFastConstant() {}

bool CVEFastConstant::GetValue(int frame, CVector3f& valOut) const {
  valOut = mVal;
  return false;
}

CVECone::CVECone(CVectorElement* direction, CRealElement* magnitude)
: mDirection(direction)
, mMagnitude(magnitude)
, mXVec(CVector3f::Zero())
, mYVec(CVector3f::Zero()) {
  CVector3f av(0.f, 0.f, 0.f);
  mDirection->GetValue(0, av);
  CVector3f avNorm = av.AsNormalized();
  if (avNorm.GetX() > 0.8f) {
    mXVec = CVector3f::Cross(av, CVector3f(0.f, 1.f, 0.f));
  } else {
    mXVec = CVector3f::Cross(av, CVector3f(1.f, 0.f, 0.f));
  }
  mYVec = CVector3f::Cross(avNorm, mXVec);
}

CVECone::~CVECone() {
  delete mDirection;
  delete mMagnitude;
}

bool CVECone::GetValue(int frame, CVector3f& valOut) const {
  float b;
  CVector3f dir(0.f, 0.f, 0.f);

  mMagnitude->GetValue(frame, b);
  mDirection->GetValue(frame, dir);
  b = rstl::min_val(1.f, b);

  float randX = 0.f, randY = 0.f;
  do {
    randX = 2.f * b * (CRandom16::GetRandomNumber()->Float() - 0.5f);
    randY = 2.f * b * (CRandom16::GetRandomNumber()->Float() - 0.5f);
  } while (randX * randX + randY * randY > 1.f);

  valOut = dir + mXVec * randX + mYVec * randY;
  return false;
}

CVEAngleCone::CVEAngleCone(CRealElement* angleXConstant, CRealElement* angleYConstant,
                           CRealElement* angleXRange, CRealElement* angleYRange,
                           CRealElement* magnitude)
: mAngleXConstant(angleXConstant)
, mAngleYConstant(angleYConstant)
, mAngleXRange(angleXRange)
, mAngleYRange(angleYRange)
, mMagnitude(magnitude) {}

CVEAngleCone::~CVEAngleCone() {
  delete mAngleXConstant;
  delete mAngleYConstant;
  delete mAngleXRange;
  delete mAngleYRange;
  delete mMagnitude;
}

bool CVEAngleCone::GetValue(int frame, CVector3f& valOut) const {
  float xc, xr, yc, yr;
  mAngleXConstant->GetValue(frame, xc);
  mAngleYConstant->GetValue(frame, yc);
  mAngleXRange->GetValue(frame, xr);
  mAngleYRange->GetValue(frame, yr);

  xc += (xr * 0.5f - (xr * CRandom16::GetRandomNumber()->Float()));
  xc *= M_PIF / 180.f;

  yc += (yr * 0.5f - (yr * CRandom16::GetRandomNumber()->Float()));
  yc *= M_PIF / 180.f;

  CVector3f vec = CVector3f(-CMath::FastSinR(yc) * CMath::FastCosR(xc), CMath::FastSinR(xc),
                            CMath::FastCosR(xc) * CMath::FastCosR(yc));
  float mag = 0.f;
  mMagnitude->GetValue(frame, mag);

  valOut = vec * mag;
  return false;
}

CVECircle::CVECircle(CVectorElement* circleOffset, CVectorElement* circleNormal,
                     CRealElement* angleConstant, CRealElement* angleLinear, CRealElement* radius)
: mCircleOffset(circleOffset)
, mXVec(CVector3f::Zero())
, mYVec(CVector3f::Zero())
, mAngleConstant(angleConstant)
, mAngleLinear(angleLinear)
, mRadius(radius) {
  CVector3f direction = CVector3f(0.f, 0.f, 0.f);
  circleNormal->GetValue(0, direction);
  CVector3f normal = direction.AsNormalized();

  if (normal.GetX() > 0.8f) {
    mXVec = CVector3f::Cross(normal, CVector3f(0.f, 1.f, 0.f));
  } else {
    mXVec = CVector3f::Cross(normal, CVector3f(1.f, 0.f, 0.f));
  }
  mYVec = CVector3f::Cross(normal, mXVec);

  delete circleNormal;
}

CVECircle::~CVECircle() {
  delete mCircleOffset;
  delete mAngleConstant;
  delete mAngleLinear;
  delete mRadius;
}

bool CVECircle::GetValue(int frame, CVector3f& valOut) const {
  float radius;
  float angleLinear;
  float angleConstant;
  mAngleLinear->GetValue(frame, angleLinear);
  mRadius->GetValue(frame, radius);
  mAngleConstant->GetValue(frame, angleConstant);

  float curAngle = (angleLinear * frame + angleConstant) * (M_PIF / 180.f);

  CVector3f offset = CVector3f(0.f, 0.f, 0.f);
  mCircleOffset->GetValue(frame, offset);

  valOut = offset + (mXVec * radius * cosf(curAngle)) + (mYVec * radius * sinf(curAngle));
  return false;
}

CVETimeChain::CVETimeChain(CVectorElement* a, CVectorElement* b, CIntElement* switchFrame)
: mA(a), mB(b), mSwitchFrame(switchFrame) {}

CVETimeChain::~CVETimeChain() {
  delete mA;
  delete mB;
  delete mSwitchFrame;
}

bool CVETimeChain::GetValue(int frame, CVector3f& valOut) const {
  int switchFrame;
  mSwitchFrame->GetValue(frame, switchFrame);

  if (frame < switchFrame) {
    return mA->GetValue(frame, valOut);
  } else {
    return mB->GetValue(frame - switchFrame, valOut);
  }
}

CVECircleCluster::CVECircleCluster(CVectorElement* circleOffset, CVectorElement* circleNormal,
                                   CIntElement* cycleFrames, CRealElement* randomFactor)
: mCircleOffset(circleOffset)
, mXVec(CVector3f::Zero())
, mYVec(CVector3f::Zero())
, mRadius(0.f)
, mRandomFactor(randomFactor) {
  int _cycleFrames;
  cycleFrames->GetValue(0, _cycleFrames);
  mRadius = (M_PIF / 180.f) * (360.f / _cycleFrames);

  CVector3f normal = CVector3f(0.f, 0.f, 0.f);
#if VERSION >= VERSION_GM8P_00
  CVector3f tmp(0.f, 0.f, 0.f);
#else
  CVector3f tmp;
#endif
  circleNormal->GetValue(0, normal);
  tmp = normal;
  if (normal.CanBeNormalized()) {
    normal = normal.AsNormalized();
  } else {
    normal = CVector3f::Up();
  }

  if (normal.GetX() > 0.8f) {
    mXVec = CVector3f::Cross(tmp, CVector3f(0.f, 1.f, 0.f));
  } else {
    mXVec = CVector3f::Cross(tmp, CVector3f(1.f, 0.f, 0.f));
  }

  mYVec = CVector3f::Cross(normal, mXVec);

  delete cycleFrames;
  delete circleNormal;
}

CVECircleCluster::~CVECircleCluster() {
  delete mCircleOffset;
  delete mRandomFactor;
}

bool CVECircleCluster::GetValue(int frame, CVector3f& valOut) const {
  float curAngle;
  curAngle = mRadius;
  curAngle *= frame;
  CVector3f offset = CVector3f(0.f, 0.f, 0.f);
  mCircleOffset->GetValue(frame, offset);

  CVector3f tv = offset + (mXVec * cosf(curAngle)) + (mYVec * sinf(curAngle));

  float dv;
  mRandomFactor->GetValue(frame, dv);

  float magnitude = dv * tv.Magnitude();
  float x = magnitude * CRandom16::GetRandomNumber()->Float();
  float y = magnitude * CRandom16::GetRandomNumber()->Float();
  float z = magnitude * CRandom16::GetRandomNumber()->Float();
  valOut = CVector3f(x + tv.GetX(), y + tv.GetY(), z + tv.GetZ());

  return false;
}

CVEAdd::CVEAdd(CVectorElement* a, CVectorElement* b) : mA(a), mB(b) {}
CVEAdd::~CVEAdd() {
  delete mA;
  delete mB;
}

bool CVEAdd::GetValue(int frame, CVector3f& valOut) const {
  CVector3f a = CVector3f::Zero();
  CVector3f b = CVector3f::Zero();

  mA->GetValue(frame, a);
  mB->GetValue(frame, b);

  valOut = a + b;
  return false;
}

CVEMultiply::CVEMultiply(CVectorElement* a, CVectorElement* b) : mA(a), mB(b) {}
CVEMultiply::~CVEMultiply() {
  delete mA;
  delete mB;
}

bool CVEMultiply::GetValue(int frame, CVector3f& valOut) const {
  CVector3f a = CVector3f::Zero();
  CVector3f b = CVector3f::Zero();

  mA->GetValue(frame, a);
  mB->GetValue(frame, b);

  valOut = CVector3f::ByElementMultiply(a, b);
  return false;
}

CVEPulse::CVEPulse(CIntElement* durationA, CIntElement* durationB, CVectorElement* a,
                   CVectorElement* b)
: mDurationA(durationA), mDurationB(durationB), mA(a), mB(b) {}

CVEPulse::~CVEPulse() {
  delete mDurationA;
  delete mDurationB;
  delete mA;
  delete mB;
}

bool CVEPulse::GetValue(int frame, CVector3f& valOut) const {
  int a;
  int b;
  mDurationA->GetValue(frame, a);
  mDurationB->GetValue(frame, b);
  int cv = a + b + 1;

  if (cv < 0) {
    cv = 1;
  }

  if (b >= 1) {
    if (frame % cv > a) {
      mB->GetValue(frame, valOut);
    } else {
      mA->GetValue(frame, valOut);
    }
  } else {
    mA->GetValue(frame, valOut);
  }
  return false;
}

CVEKeyframeEmitter::CVEKeyframeEmitter(CInputStream& in)
: mPercent(in.Get< uint >())
, mUnk1(in.Get< uint >())
, mLoop(in.Get< bool >())
, mUnk2(in.Get< bool >())
, mLoopEnd(in.Get< uint >())
, mLoopStart(in.Get< uint >())
, mKeys(in) {}

CVEKeyframeEmitter::~CVEKeyframeEmitter() {}

bool CVEKeyframeEmitter::GetValue(int frame, CVector3f& valOut) const {
  if (!mPercent) {
    int emitterTime = CParticleGlobals::GetEmitterTime();
    if (mLoop) {
      if (emitterTime >= mLoopEnd) {
        emitterTime -= mLoopStart;
        emitterTime %= mLoopEnd - mLoopStart;
        emitterTime += mLoopStart;
      }
      valOut = mKeys[emitterTime];
    } else {
      emitterTime = rstl::min_val(emitterTime, mLoopEnd - 1);
      valOut = mKeys[emitterTime];
    }
    return false;
  }

  if (CParticleGlobals::GetParticleLifetimePercentage() == 100) {
    valOut = mKeys[CParticleGlobals::GetParticleLifetimePercentage()];
  } else {
    float remainder = CParticleGlobals::GetParticleLifetimePercentageRemainder();
    valOut = (1.f - remainder) * mKeys[CParticleGlobals::GetParticleLifetimePercentage()] +
             remainder * mKeys[CParticleGlobals::GetParticleLifetimePercentage() + 1];
  }

  return false;
}

CVERealToVector::CVERealToVector(CRealElement* value) : mValue(value) {}
CVERealToVector::~CVERealToVector() { delete mValue; }

bool CVERealToVector::GetValue(int frame, CVector3f& valOut) const {
  float val = 0.f;
  mValue->GetValue(frame, val);
  valOut = CVector3f(val, val, val);

  return false;
}

bool CVEParticleLocation::GetValue(int frame, CVector3f& valOut) const {
  valOut = CParticleGlobals::GetCurrentParticle()->mPos;
  return false;
}

bool CVEParticlePreviousLocation::GetValue(int frame, CVector3f& valOut) const {
  valOut = CParticleGlobals::GetCurrentParticle()->mPrevPos;
  return false;
}

bool CVEParticleVelocity::GetValue(int frame, CVector3f& valOut) const {
  valOut = CParticleGlobals::GetCurrentParticle()->mVel;
  return false;
}

bool CVEParticleSystemOrientationFront::GetValue(int frame, CVector3f& valOut) const {
  valOut = CParticleGlobals::GetCurrentParticleSystem()->mSystem->GetOrientation().GetForward();
  return false;
}

bool CVEParticleSystemOrientationUp::GetValue(int frame, CVector3f& valOut) const {
  valOut = CParticleGlobals::GetCurrentParticleSystem()->mSystem->GetOrientation().GetUp();
  return false;
}

bool CVEParticleSystemOrientationRight::GetValue(int frame, CVector3f& valOut) const {
  valOut = CParticleGlobals::GetCurrentParticleSystem()->mSystem->GetOrientation().GetRight();
  return false;
}

bool CVEParticleSystemTranslation::GetValue(int frame, CVector3f& valOut) const {
  valOut = CParticleGlobals::GetCurrentParticleSystem()->mSystem->GetTranslation();
  return false;
}

CVESubtract::CVESubtract(CVectorElement* a, CVectorElement* b) : mA(a), mB(b) {}
CVESubtract::~CVESubtract() {
  delete mA;
  delete mB;
}

bool CVESubtract::GetValue(int frame, CVector3f& valOut) const {
  CVector3f a = CVector3f::Zero();
  CVector3f b = CVector3f::Zero();

  mA->GetValue(frame, a);
  mB->GetValue(frame, b);

  valOut = a - b;
  return false;
}

CVEColorToVector::CVEColorToVector(CColorElement* value) : mValue(value) {}
CVEColorToVector::~CVEColorToVector() { delete mValue; }

bool CVEColorToVector::GetValue(int frame, CVector3f& valOut) const {
  CColor val = CColor::Black();
  mValue->GetValue(frame, val);
  valOut.SetX(val.GetRed());
  valOut.SetY(val.GetGreen());
  valOut.SetZ(val.GetBlue());

  return false;
}
