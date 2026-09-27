#include "Kyoto/Particles/CColorElement.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Particles/CParticleGlobals.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

#include "rstl/math.hpp"

CCEConstant::CCEConstant(CRealElement* r, CRealElement* g, CRealElement* b, CRealElement* a)
: mR(r), mG(g), mB(b), mA(a) {}

CCEConstant::~CCEConstant() {
  delete mR;
  delete mG;
  delete mB;
  delete mA;
}

bool CCEConstant::GetValue(int frame, CColor& colorOut) const {
  float r, g, b, a;
  mR->GetValue(frame, r);
  r = CMath::Clamp(0.f, r, 1.f);
  mG->GetValue(frame, g);
  g = CMath::Clamp(0.f, g, 1.f);
  mB->GetValue(frame, b);
  b = CMath::Clamp(0.f, b, 1.f);
  mA->GetValue(frame, a);
  a = CMath::Clamp(0.f, a, 1.f);
  colorOut.Set(r, g, b, a);
  return false;
}

CCEFastConstant::CCEFastConstant(const float r, const float g, const float b, const float a) {
   float cr = CMath::Clamp(0.f, r, 1.f);
   float cg = CMath::Clamp(0.f, g, 1.f);
   float cb = CMath::Clamp(0.f, b, 1.f);
   float ca = CMath::Clamp(0.f, a, 1.f);
   mVal.Set(cr, cg, cb, ca);
}

CCEFastConstant::~CCEFastConstant() {}

bool CCEFastConstant::GetValue(int frame, CColor& colorOut) const {
  colorOut = mVal;
  return false;
}

CCEFade::CCEFade(CColorElement* a, CColorElement* b, CRealElement* end)
: mA(a), mB(b), mEndFrame(end) {}

CCEFade::~CCEFade() {
  delete mA;
  delete mB;
  delete mEndFrame;
}

bool CCEFade::GetValue(int frame, CColor& colorOut) const {
  float c;
  mEndFrame->GetValue(frame, c);

  float t = static_cast< float >(frame) * (1.f / c);
  if (t >= 1.f) {
    mB->GetValue(frame, colorOut);
  } else {
    CColor colA;
    CColor colB;
    mA->GetValue(frame, colA);
    mB->GetValue(frame, colB);

    float ar, ag, ab, aa;
    float br, bg, bb, ba;
    colA.Get(ar, ag, ab, aa);
    colB.Get(br, bg, bb, ba);

    float nt = 1.f - t;
    colorOut = CColor(ar * nt + br * t, ag * nt + bg * t, ab * nt + bb * t, aa * nt + ba * t);
  }
  return false;
}

CCEFadeEnd::CCEFadeEnd(CColorElement* a, CColorElement* b, CRealElement* start, CRealElement* end)
: mA(a), mB(b), mStartFrame(start), mEndFrame(end) {}

CCEFadeEnd::~CCEFadeEnd() {
  delete mA;
  delete mB;
  delete mStartFrame;
  delete mEndFrame;
}

bool CCEFadeEnd::GetValue(int frame, CColor& colorOut) const {
  float start;
  mStartFrame->GetValue(frame, start);

  float frameF = static_cast< float >(frame);
  if (frameF < start) {
    mA->GetValue(frame, colorOut);
    return false;
  }

  float end;
  mEndFrame->GetValue(frame, end);

  CColor colA;
  CColor colB;
  mA->GetValue(frame, colA);
  mB->GetValue(frame, colB);

  float ar, ag, ab, aa;
  float br, bg, bb, ba;
  colA.Get(ar, ag, ab, aa);
  colB.Get(br, bg, bb, ba);

  float t = (frameF - start) / (end - start);
  float nt = 1.f - t;
  colorOut = CColor(ar * nt + br * t, ag * nt + bg * t, ab * nt + bb * t, aa * nt + ba * t);
  return false;
}

CCETimeChain::CCETimeChain(CColorElement* a, CColorElement* b, CIntElement* c)
: mA(a), mB(b), mSwFrame(c) {}

CCETimeChain::~CCETimeChain() {
  delete mA;
  delete mB;
  delete mSwFrame;
}

bool CCETimeChain::GetValue(int frame, CColor& colorOut) const {
  int v;
  mSwFrame->GetValue(frame, v);
  if (frame < v) {
    return mA->GetValue(frame, colorOut);
  } else {
    return mB->GetValue(frame - v, colorOut);
  }
}

CCEPulse::CCEPulse(CIntElement* a, CIntElement* b, CColorElement* c, CColorElement* d)
: mADuration(a), mBDuration(b), mAVal(c), mBVal(d) {}

CCEPulse::~CCEPulse() {
  delete mADuration;
  delete mBDuration;
  delete mAVal;
  delete mBVal;
}

bool CCEPulse::GetValue(int frame, CColor& colorOut) const {
  int a, b;
  mADuration->GetValue(frame, a);
  mBDuration->GetValue(frame, b);
  int cv = a + b + 1;
  if (cv < 0) {
    cv = 1;
  }

  if (b >= 1) {
    if (frame % cv > a) {
      mBVal->GetValue(frame, colorOut);
    } else {
      mAVal->GetValue(frame, colorOut);
    }
  } else {
    mAVal->GetValue(frame, colorOut);
  }
  return false;
}

CCEKeyframeEmitter::CCEKeyframeEmitter(CInputStream& in)
: mPercent(in.ReadLong())
, mUnk1(in.ReadLong())
, mLoop(in.ReadBool())
, mUnk2(in.ReadBool())
, mLoopEnd(in.ReadLong())
, mLoopStart(in.ReadLong())
, mKeys(in) {
  if (mLoopStart >= mLoopEnd) {
    mLoopStart = 0;
  }
}

CCEKeyframeEmitter::~CCEKeyframeEmitter() {}

bool CCEKeyframeEmitter::GetValue(int frame, CColor& valOut) const {
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
  }

  if (CParticleGlobals::GetParticleLifetimePercentage() == 100) {
    valOut = mKeys[CParticleGlobals::GetParticleLifetimePercentage()];
  } else {
    valOut = CColor::Lerp(mKeys[CParticleGlobals::GetParticleLifetimePercentage()],
                          mKeys[CParticleGlobals::GetParticleLifetimePercentage() + 1],
                          CParticleGlobals::GetParticleLifetimePercentageRemainder());
  }
  return false;
}

bool CCEParticleColor::GetValue(int frame, CColor& colorOut) const {
  colorOut = CParticleGlobals::GetCurrentParticle()->mColor;
  return false;
}
