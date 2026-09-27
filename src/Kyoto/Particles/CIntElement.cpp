#include "Kyoto/Particles/CIntElement.hpp"

#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Particles/CParticleGlobals.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

#include "rstl/math.hpp"

CIEConstant::CIEConstant(int val) : mVal(val) {}

CIEConstant::~CIEConstant() {}

bool CIEConstant::GetValue(int frame, int& valOut) const {
  valOut = mVal;
  return false;
}

CIEDeath::CIEDeath(CIntElement* a, CIntElement* b) : mA(a), mB(b) {}

CIEDeath::~CIEDeath() {
  delete mA;
  delete mB;
}

bool CIEDeath::GetValue(int frame, int& valOut) const {
  int b;
  mA->GetValue(frame, valOut);
  mB->GetValue(frame, b);
  return frame >= b ? TRUE : FALSE;
}

CIEAdd::CIEAdd(CIntElement* a, CIntElement* b) : mA(a), mB(b) {}

CIEAdd::~CIEAdd() {
  delete mA;
  delete mB;
}

bool CIEAdd::GetValue(int frame, int& valOut) const {
  int a, b;
  mA->GetValue(frame, a);
  mB->GetValue(frame, b);
  valOut = a + b;
  return false;
}

CIEMultiply::CIEMultiply(CIntElement* a, CIntElement* b) : mA(a), mB(b) {}

CIEMultiply::~CIEMultiply() {
  delete mA;
  delete mB;
}

bool CIEMultiply::GetValue(int frame, int& valOut) const {
  int a, b;
  mA->GetValue(frame, a);
  mB->GetValue(frame, b);
  valOut = a * b;
  return false;
}

CIEModulo::CIEModulo(CIntElement* a, CIntElement* b) : mA(a), mB(b) {}

CIEModulo::~CIEModulo() {
  delete mA;
  delete mB;
}

bool CIEModulo::GetValue(int frame, int& valOut) const {
  int a, b;
  mA->GetValue(frame, a);
  mB->GetValue(frame, b);
  if (b != 0) {
    valOut = a % b;
  } else {
    valOut = a;
  }
  return false;
}

CIERandom::CIERandom(CIntElement* a, CIntElement* b) : mA(a), mB(b) {}

CIERandom::~CIERandom() {
  delete mA;
  delete mB;
}

bool CIERandom::GetValue(int frame, int& valOut) const {
  int a, b;
  mA->GetValue(frame, a);
  mB->GetValue(frame, b);
  if (a > 0) {
    valOut = CRandom16::GetRandomNumber()->Range(a, b);
  } else {
    valOut = CRandom16::GetRandomNumber()->Next();
  }
  return false;
}

CIESampleAndHold::CIESampleAndHold(CIntElement* a, CIntElement* b, CIntElement* c)
: mSampleSource(a), mNextSampleFrame(0), mWaitFramesMin(b), mWaitFramesMax(c) {}

CIESampleAndHold::~CIESampleAndHold() {
  delete mSampleSource;
  delete mWaitFramesMin;
  delete mWaitFramesMax;
}

bool CIESampleAndHold::GetValue(int frame, int& valOut) const {
  bool ret;
  if (mNextSampleFrame < frame) {
    int b, c;
    mWaitFramesMin->GetValue(frame, b);
    mWaitFramesMax->GetValue(frame, c);
    mNextSampleFrame = CRandom16::GetRandomNumber()->Range(b, c) + frame;
    ret = mSampleSource->GetValue(frame, valOut);
    mHoldVal = valOut;
  } else {
    valOut = mHoldVal;
    ret = false;
  }
  return ret;
}

CIEImpulse::CIEImpulse(CIntElement* a) : mA(a) {}

CIEImpulse::~CIEImpulse() { delete mA; }

bool CIEImpulse::GetValue(int frame, int& valOut) const {
  if (frame == 0) {
    mA->GetValue(frame, valOut);
  } else {
    valOut = 0;
  }
  return false;
}

CIETimescale::CIETimescale(CRealElement* a) : mA(a) {}

CIETimescale::~CIETimescale() { delete mA; }

bool CIETimescale::GetValue(int frame, int& valOut) const {
  float a;
  mA->GetValue(frame, a);
  valOut = static_cast< float >(frame) * a;
  return false;
}

CIEInitialRandom::CIEInitialRandom(CIntElement* a, CIntElement* b) : mA(a), mB(b) {}

CIEInitialRandom::~CIEInitialRandom() {
  delete mA;
  delete mB;
}

bool CIEInitialRandom::GetValue(int frame, int& valOut) const {
  if (frame == 0) {
    int a, b;
    mA->GetValue(frame, a);
    mB->GetValue(frame, b);
    valOut = CRandom16::GetRandomNumber()->Range(a, b);
  }
  return false;
}

CIETimeChain::CIETimeChain(CIntElement* a, CIntElement* b, CIntElement* c)
: mA(a), mB(b), mSwFrame(c) {}

CIETimeChain::~CIETimeChain() {
  delete mA;
  delete mB;
  delete mSwFrame;
}

bool CIETimeChain::GetValue(int frame, int& valOut) const {
  int v;
  mSwFrame->GetValue(frame, v);
  if (frame < v) {
    return mA->GetValue(frame, valOut);
  } else {
    return mB->GetValue(frame - v, valOut);
  }
}

CIEClamp::CIEClamp(CIntElement* a, CIntElement* b, CIntElement* c)
: mMin(a), mMax(b), mVal(c) {}

CIEClamp::~CIEClamp() {
  delete mMin;
  delete mMax;
  delete mVal;
}

bool CIEClamp::GetValue(int frame, int& valOut) const {
  int a, b;
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

CIEPulse::CIEPulse(CIntElement* a, CIntElement* b, CIntElement* c, CIntElement* d)
: mADuration(a), mBDuration(b), mAVal(c), mBVal(d) {}

CIEPulse::~CIEPulse() {
  delete mADuration;
  delete mBDuration;
  delete mAVal;
  delete mBVal;
}

bool CIEPulse::GetValue(int frame, int& valOut) const {
  int a, b;
  mADuration->GetValue(frame, a);
  mBDuration->GetValue(frame, b);
  int cv = a + b + 1;
  if (cv < 0) {
    cv = 1;
  }

  if (b >= 1) {
    if (frame % cv > a) {
      mBVal->GetValue(frame, valOut);
    } else {
      mAVal->GetValue(frame, valOut);
    }
  } else {
    mAVal->GetValue(frame, valOut);
  }
  return false;
}

CIELifetimePercent::CIELifetimePercent(CIntElement* a) : mPercentVal(a) {}

CIELifetimePercent::~CIELifetimePercent() { delete mPercentVal; }

bool CIELifetimePercent::GetValue(int frame, int& valOut) const {
  int a = 0;
  mPercentVal->GetValue(frame, a);
  if (a < 0) {
    a = 0;
  }
  valOut = (a / 100.0f) * CParticleGlobals::GetParticleLifetimeReal() + 0.5f;
  return false;
}

CIEKeyframeEmitter::CIEKeyframeEmitter(CInputStream& in)
: mPercent(in.ReadLong())
, mUnk1(in.ReadLong())
, mLoop(in.ReadBool())
, mUnk2(in.ReadBool())
, mLoopEnd(in.ReadLong())
, mLoopStart(in.ReadLong())
, mKeys(in) {}

CIEKeyframeEmitter::~CIEKeyframeEmitter() {}

bool CIEKeyframeEmitter::GetValue(int frame, int& valOut) const {
  if (mPercent == 0) {
    int emitterTime = CParticleGlobals::GetEmitterTime();
    if (mLoop) {
      if (emitterTime >= mLoopEnd) {
        emitterTime -= mLoopStart;
        emitterTime = emitterTime % (mLoopEnd - mLoopStart);
        emitterTime += mLoopStart;
      }
      valOut = mKeys[emitterTime];
    } else {
      emitterTime = rstl::min_val(emitterTime, mLoopEnd - 1);
      valOut = mKeys[emitterTime];
    }
    return false;
  } else {
    int ltPerc = CParticleGlobals::GetParticleLifetimePercentage();
    if (ltPerc == 100) {
      valOut = mKeys[ltPerc];
    } else {
      float ltPercRem = CParticleGlobals::GetParticleLifetimePercentageRemainder();
      float lerp = (1.0f - ltPercRem) * mKeys[ltPerc] + ltPercRem * mKeys[ltPerc + 1];
      valOut = CCast::ToInt32(lerp);
    }
    return false;
  }
}

CIESubtract::CIESubtract(CIntElement* a, CIntElement* b) : mA(a), mB(b) {}

CIESubtract::~CIESubtract() {
  delete mA;
  delete mB;
}

bool CIESubtract::GetValue(int frame, int& valOut) const {
  int a, b;
  mA->GetValue(frame, a);
  mB->GetValue(frame, b);
  valOut = a - b;
  return false;
}

CIERealToInt::CIERealToInt(CRealElement* a, CRealElement* b) : mA(a), mB(b) {}

CIERealToInt::~CIERealToInt() {
  delete mA;
  delete mB;
}

bool CIERealToInt::GetValue(int frame, int& valOut) const {
  float a = 0.0f;
  float b = 1.0f;
  mB->GetValue(frame, b);
  mA->GetValue(frame, a);
  valOut = CCast::ToInt32(a * b);
  return false;
}

bool CIEGetCumulativeParticleCount::GetValue(int frame, int& valOut) const {
  valOut = CParticleGlobals::GetCurrentParticleSystem()->mSystem->GetCumulativeParticleCount();
  return false;
}

bool CIEGetActiveParticleCount::GetValue(int frame, int& valOut) const {
  valOut = CParticleGlobals::GetCurrentParticleSystem()->mSystem->GetParticleCount();
  return false;
}

bool CIEGetEmitterTime::GetValue(int frame, int& valOut) const {
  valOut = CParticleGlobals::GetCurrentParticleSystem()->mSystem->GetEmitterTime();
  return false;
}

CIEGetEmitterTime::~CIEGetEmitterTime() {}
