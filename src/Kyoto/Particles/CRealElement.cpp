#include "Kyoto/Particles/CRealElement.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CParticleGlobals.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

#include "rstl/math.hpp"

CREConstant::CREConstant(float val) : mVal(val) {}

CREConstant::~CREConstant() {}

bool CREConstant::GetValue(int frame, float& valOut) const {
  valOut = mVal;
  return false;
}

CRESineWave::CRESineWave(CRealElement* a, CRealElement* b, CRealElement* c)
: mFrequency(b), mAmplitude(c), mPhase(a) {}

CRESineWave::~CRESineWave() {
  delete mFrequency;
  delete mAmplitude;
  delete mPhase;
}

bool CRESineWave::GetValue(int frame, float& valOut) const {
  float amp, freq, phase;
  mAmplitude->GetValue(frame, amp);
  mFrequency->GetValue(frame, freq);
  mPhase->GetValue(frame, phase);
  valOut = sine(CRelAngle::FromDegrees(frame * freq + phase)) * amp;
  return false;
}

CRETimeScale::CRETimeScale(CRealElement* a) : mA(a) {}

CRETimeScale::~CRETimeScale() { delete mA; }

bool CRETimeScale::GetValue(int frame, float& valOut) const {
  float a;
  mA->GetValue(frame, a);
  valOut = static_cast< float >(frame) * a;
  return false;
}

CREAdd::CREAdd(CRealElement* a, CRealElement* b) : mA(a), mB(b) {}

CREAdd::~CREAdd() {
  delete mA;
  delete mB;
}

bool CREAdd::GetValue(int frame, float& valOut) const {
#if NONMATCHING
  float a = 0.f, b = 0.f;
#else
  float a, b;
#endif
  mA->GetValue(frame, a);
  mB->GetValue(frame, b);
  valOut = a + b;
  return false;
}

CREMultiply::CREMultiply(CRealElement* a, CRealElement* b) : mA(a), mB(b) {}

CREMultiply::~CREMultiply() {
  delete mA;
  delete mB;
}

bool CREMultiply::GetValue(int frame, float& valOut) const {
#if NONMATCHING
  float a = 0.f, b = 0.f;
#else
  float a, b;
#endif
  mA->GetValue(frame, a);
  mB->GetValue(frame, b);
  valOut = a * b;
  return false;
}

CREDotProduct::CREDotProduct(CVectorElement* a, CVectorElement* b) : mA(a), mB(b) {}

CREDotProduct::~CREDotProduct() {
  delete mA;
  delete mB;
}

bool CREDotProduct::GetValue(int frame, float& valOut) const {
  CVector3f a = CVector3f::Zero();
  CVector3f b = CVector3f::Zero();
  mA->GetValue(frame, a);
  mB->GetValue(frame, b);
  valOut = CVector3f::Dot(a, b);
  return false;
}

CRERandom::CRERandom(CRealElement* min, CRealElement* max) : mMin(min), mMax(max) {}

CRERandom::~CRERandom() {
  delete mMin;
  delete mMax;
}

bool CRERandom::GetValue(int frame, float& valOut) const {
  float min, max;
  mMin->GetValue(frame, min);
  mMax->GetValue(frame, max);
  valOut = (max - min) * CRandom16::GetRandomNumber()->Float() + min;
  return false;
}

CREInitialRandom::CREInitialRandom(CRealElement* min, CRealElement* max)
: mMin(min), mMax(max) {}

CREInitialRandom::~CREInitialRandom() {
  delete mMin;
  delete mMax;
}

bool CREInitialRandom::GetValue(int frame, float& valOut) const {
  if (frame == 0) {
    float min, max;
    mMin->GetValue(frame, min);
    mMax->GetValue(frame, max);
    valOut = (max - min) * CRandom16::GetRandomNumber()->Float() + min;
  }
  return false;
}

CRETimeChain::CRETimeChain(CRealElement* a, CRealElement* b, CIntElement* c)
: mA(a), mB(b), mSwFrame(c) {}

CRETimeChain::~CRETimeChain() {
  delete mA;
  delete mB;
  delete mSwFrame;
}

bool CRETimeChain::GetValue(int frame, float& valOut) const {
  int v;
  mSwFrame->GetValue(frame, v);
  if (frame < v) {
    return mA->GetValue(frame, valOut);
  } else {
    return mB->GetValue(frame - v, valOut);
  }
}

CREClamp::CREClamp(CRealElement* a, CRealElement* b, CRealElement* c)
: mMin(a), mMax(b), mVal(c) {}

CREClamp::~CREClamp() {
  delete mMin;
  delete mMax;
  delete mVal;
}

bool CREClamp::GetValue(int frame, float& valOut) const {
  float a, b;
  mMin->GetValue(frame, a);
  mMax->GetValue(frame, b);
  mVal->GetValue(frame, valOut);
  if (valOut > b) {
    valOut = b;
  }
  if (valOut < a) {
    valOut = a;
  }
  return false;
}

CREPulse::CREPulse(CIntElement* a, CIntElement* b, CRealElement* c, CRealElement* d)
: mADuration(a), mBDuration(b), mValA(c), mValB(d) {}

CREPulse::~CREPulse() {
  delete mADuration;
  delete mBDuration;
  delete mValA;
  delete mValB;
}

bool CREPulse::GetValue(int frame, float& valOut) const {
  int a, b;
  mADuration->GetValue(frame, a);
  mBDuration->GetValue(frame, b);
  int cv = a + b + 1;
  if (cv < 0) {
    cv = 1;
  }

  if (b >= 1) {
    // CREPulse is an outlier here, the other
    // IElement classes use > instead of >=.
    if (frame % cv >= a) {
      mValB->GetValue(frame, valOut);
    } else {
      mValA->GetValue(frame, valOut);
    }
  } else {
    mValA->GetValue(frame, valOut);
  }
  return false;
}

CRELifetimePercent::CRELifetimePercent(CRealElement* a) : mPercentVal(a) {}

CRELifetimePercent::~CRELifetimePercent() { delete mPercentVal; }

bool CRELifetimePercent::GetValue(int frame, float& valOut) const {
  float a = 0.f;
  mPercentVal->GetValue(frame, a);
  if (a < 0.f) {
    a = 0.f;
  }
  valOut = (a / 100.f) * CParticleGlobals::GetParticleLifetimeReal();
  return false;
}

CRELifetimeTween::CRELifetimeTween(CRealElement* a, CRealElement* b) : mA(a), mB(b) {}

CRELifetimeTween::~CRELifetimeTween() {
  delete mA;
  delete mB;
}

// fake but using it to test
static inline float Lerp(float a, float b, float c) { return b * c + a * (1.f - c); }

bool CRELifetimeTween::GetValue(int frame, float& valOut) const {
  float ltFac = frame / CParticleGlobals::GetParticleLifetimeReal();
  float a, b;
  mA->GetValue(frame, a);
  mB->GetValue(frame, b);
  valOut = Lerp(a, b, ltFac);
  return false;
}

CREKeyframeEmitter::CREKeyframeEmitter(CInputStream& in)
: mPercent(in.ReadLong())
, mUnk1(in.ReadLong())
, mLoop(in.ReadBool())
, mUnk2(in.ReadBool())
, mLoopEnd(in.ReadLong())
, mLoopStart(in.ReadLong())
, mKeys(in) {}

CREKeyframeEmitter::~CREKeyframeEmitter() {}

bool CREKeyframeEmitter::GetValue(int frame, float& valOut) const {
  if (mPercent == 0) {
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
    valOut = (1.f - CParticleGlobals::GetParticleLifetimePercentageRemainder()) *
                 mKeys[CParticleGlobals::GetParticleLifetimePercentage()] +
             CParticleGlobals::GetParticleLifetimePercentageRemainder() *
                 mKeys[CParticleGlobals::GetParticleLifetimePercentage() + 1];
  }
  return false;
}

bool CREParticleAccessParameter1::GetValue(int, float& valOut) const {
  valOut = CParticleGlobals::GetParticleAccessParameters()[0];
  return false;
}

bool CREParticleAccessParameter2::GetValue(int, float& valOut) const {
  valOut = CParticleGlobals::GetParticleAccessParameters()[1];
  return false;
}

bool CREParticleAccessParameter3::GetValue(int, float& valOut) const {
  valOut = CParticleGlobals::GetParticleAccessParameters()[2];
  return false;
}

bool CREParticleAccessParameter4::GetValue(int, float& valOut) const {
  valOut = CParticleGlobals::GetParticleAccessParameters()[3];
  return false;
}

bool CREParticleAccessParameter5::GetValue(int, float& valOut) const {
  valOut = CParticleGlobals::GetParticleAccessParameters()[4];
  return false;
}

bool CREParticleAccessParameter6::GetValue(int, float& valOut) const {
  valOut = CParticleGlobals::GetParticleAccessParameters()[5];
  return false;
}

bool CREParticleAccessParameter7::GetValue(int, float& valOut) const {
  valOut = CParticleGlobals::GetParticleAccessParameters()[6];
  return false;
}

bool CREParticleAccessParameter8::GetValue(int, float& valOut) const {
  valOut = CParticleGlobals::GetParticleAccessParameters()[7];
  return false;
}

bool CREParticleSizeOrLineLength::GetValue(int, float& valOut) const {
  valOut = CParticleGlobals::GetCurrentParticle()->mLineLengthOrSize;
  return false;
}

bool CREParticleRotationOrLineWidth::GetValue(int, float& valOut) const {
  valOut = CParticleGlobals::GetCurrentParticle()->mLineWidthOrRota;
  return false;
}

CREVectorXToReal::CREVectorXToReal(CVectorElement* a) : mA(a) {}

CREVectorXToReal::~CREVectorXToReal() { delete mA; }

bool CREVectorXToReal::GetValue(int frame, float& valOut) const {
  CVector3f a = CVector3f::Zero();
  mA->GetValue(frame, a);
  valOut = a[0];
  return false;
}

CREVectorYToReal::CREVectorYToReal(CVectorElement* a) : mA(a) {}

CREVectorYToReal::~CREVectorYToReal() { delete mA; }

bool CREVectorYToReal::GetValue(int frame, float& valOut) const {
  CVector3f a = CVector3f::Zero();
  mA->GetValue(frame, a);
  valOut = a[1];
  return false;
}

CREVectorZToReal::CREVectorZToReal(CVectorElement* a) : mA(a) {}

CREVectorZToReal::~CREVectorZToReal() { delete mA; }

bool CREVectorZToReal::GetValue(int frame, float& valOut) const {
  CVector3f a = CVector3f::Zero();
  mA->GetValue(frame, a);
  valOut = a[2];
  return false;
}

CREVectorMagnitude::CREVectorMagnitude(CVectorElement* a) : mA(a) {}

CREVectorMagnitude::~CREVectorMagnitude() { delete mA; }

bool CREVectorMagnitude::GetValue(int frame, float& valOut) const {
  CVector3f a = CVector3f::Zero();
  mA->GetValue(frame, a);
  valOut = a.Magnitude();
  return false;
}

CREInitialSwitch::CREInitialSwitch(CRealElement* a, CRealElement* b) : mA(a), mB(b) {}

CREInitialSwitch::~CREInitialSwitch() {
  delete mA;
  delete mB;
}

bool CREInitialSwitch::GetValue(int frame, float& valOut) const {
  if (frame == 0) {
    mA->GetValue(0, valOut);
  } else {
    mB->GetValue(frame - 1, valOut);
  }
  return false;
}

CRECompareLessThan::CRECompareLessThan(CRealElement* a, CRealElement* b, CRealElement* c,
                                       CRealElement* d)
: mA(a), mB(b), mC(c), mD(d) {}

CRECompareLessThan::~CRECompareLessThan() {
  delete mA;
  delete mB;
  delete mC;
  delete mD;
}

bool CRECompareLessThan::GetValue(int frame, float& valOut) const {
  float a, b;
  mA->GetValue(frame, a);
  mB->GetValue(frame, b);
  if (a < b) {
    mC->GetValue(frame, valOut);
  } else {
    mD->GetValue(frame, valOut);
  }
  return false;
}

CRECompareEqual::CRECompareEqual(CRealElement* a, CRealElement* b, CRealElement* c, CRealElement* d)
: mA(a), mB(b), mC(c), mD(d) {}

CRECompareEqual::~CRECompareEqual() {
  delete mA;
  delete mB;
  delete mC;
  delete mD;
}

bool CRECompareEqual::GetValue(int frame, float& valOut) const {
  float a, b;
  mA->GetValue(frame, a);
  mB->GetValue(frame, b);
  if (close_enough(a, b)) {
    mC->GetValue(frame, valOut);
  } else {
    mD->GetValue(frame, valOut);
  }
  return false;
}

CREConstantRange::CREConstantRange(CRealElement* a, CRealElement* b, CRealElement* c,
                                   CRealElement* d, CRealElement* e)
: mVal(a), mMin(b), mMax(c), mInRange(d), mOutOfRange(e) {}

CREConstantRange::~CREConstantRange() {
  delete mVal;
  delete mMin;
  delete mMax;
  delete mInRange;
  delete mOutOfRange;
}

bool CREConstantRange::GetValue(int frame, float& valOut) const {
  float val, min, max;
  mVal->GetValue(frame, val);
  mMin->GetValue(frame, min);
  mMax->GetValue(frame, max);
  if (val > min && val < max) {
    mInRange->GetValue(frame, valOut);
  } else {
    mOutOfRange->GetValue(frame, valOut);
  }
  return false;
}

CREExternalVar::CREExternalVar(CIntElement* a) : mA(a) {}

CREExternalVar::~CREExternalVar() { delete mA; }

bool CREExternalVar::GetValue(int frame, float& valOut) const {
  int a = 0;
  mA->GetValue(frame, a);
  a = rstl::max_val(0, a);
  a %= 16;
  valOut = CParticleGlobals::GetCurrentParticleSystem()->mSystem->GetExternalVar(a);
  return false;
}

CRESubtract::CRESubtract(CRealElement* a, CRealElement* b) : mA(a), mB(b) {}

CRESubtract::~CRESubtract() {
  delete mA;
  delete mB;
}

bool CRESubtract::GetValue(int frame, float& valOut) const {
  float a = 0.f, b = 0.f;
  mA->GetValue(frame, a);
  mB->GetValue(frame, b);
  valOut = a - b;
  return false;
}

CREGetComponentRed::CREGetComponentRed(CColorElement* a) : mA(a) {}

CREGetComponentRed::~CREGetComponentRed() { delete mA; }

bool CREGetComponentRed::GetValue(int frame, float& valOut) const {
  CColor color = CColor::Black();
  mA->GetValue(frame, color);
  valOut = color.GetRed();
  return false;
}

CREGetComponentGreen::CREGetComponentGreen(CColorElement* a) : mA(a) {}

CREGetComponentGreen::~CREGetComponentGreen() { delete mA; }

bool CREGetComponentGreen::GetValue(int frame, float& valOut) const {
  CColor color = CColor::Black();
  mA->GetValue(frame, color);
  valOut = color.GetGreen();
  return false;
}

CREGetComponentBlue::CREGetComponentBlue(CColorElement* a) : mA(a) {}

CREGetComponentBlue::~CREGetComponentBlue() { delete mA; }

bool CREGetComponentBlue::GetValue(int frame, float& valOut) const {
  CColor color = CColor::Black();
  mA->GetValue(frame, color);
  valOut = color.GetBlue();
  return false;
}

CREGetComponentAlpha::CREGetComponentAlpha(CColorElement* a) : mA(a) {}

CREGetComponentAlpha::~CREGetComponentAlpha() { delete mA; }

bool CREGetComponentAlpha::GetValue(int frame, float& valOut) const {
  CColor color = CColor::Black();
  mA->GetValue(frame, color);
  valOut = color.GetAlpha();
  return false;
}

CREIntTimesReal::CREIntTimesReal(CIntElement* a, CRealElement* b) : mA(a), mB(b) {}

CREIntTimesReal::~CREIntTimesReal() {
  delete mA;
  delete mB;
}

bool CREIntTimesReal::GetValue(int frame, float& valOut) const {
  int a = 0;
  float b = 1.f;
  mB->GetValue(frame, b);
  mA->GetValue(frame, a);
  valOut = b * static_cast< float >(a);
  return false;
}
