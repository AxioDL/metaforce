#ifndef _CINTELEMENT
#define _CINTELEMENT

#include "types.h"

#include "Kyoto/Particles/IElement.hpp"

#include "rstl/vector.hpp"

class CInputStream;

class CIEConstant : public CIntElement {
  int mVal;

public:
  CIEConstant(int val);
  ~CIEConstant() override;
  bool GetValue(int frame, int& valOut) const override;
};

class CIEDeath : public CIntElement {
  CIntElement* mA;
  CIntElement* mB;

public:
  CIEDeath(CIntElement* a, CIntElement* b);
  ~CIEDeath() override;
  bool GetValue(int frame, int& valOut) const override;
};

class CIEAdd : public CIntElement {
  CIntElement* mA;
  CIntElement* mB;

public:
  CIEAdd(CIntElement* a, CIntElement* b);
  ~CIEAdd() override;
  bool GetValue(int frame, int& valOut) const override;
};

class CIEMultiply : public CIntElement {
  CIntElement* mA;
  CIntElement* mB;

public:
  CIEMultiply(CIntElement* a, CIntElement* b);
  ~CIEMultiply() override;
  bool GetValue(int frame, int& valOut) const override;
};

class CIEModulo : public CIntElement {
  CIntElement* mA;
  CIntElement* mB;

public:
  CIEModulo(CIntElement* a, CIntElement* b);
  ~CIEModulo() override;
  bool GetValue(int frame, int& valOut) const override;
};

class CIERandom : public CIntElement {
  CIntElement* mA;
  CIntElement* mB;

public:
  CIERandom(CIntElement* a, CIntElement* b);
  ~CIERandom() override;
  bool GetValue(int frame, int& valOut) const override;
};

class CIESampleAndHold : public CIntElement {
  CIntElement* mSampleSource;
  mutable int mNextSampleFrame;
  CIntElement* mWaitFramesMin;
  CIntElement* mWaitFramesMax;
  mutable int mHoldVal;

public:
  CIESampleAndHold(CIntElement* a, CIntElement* b, CIntElement* c);
  ~CIESampleAndHold() override;
  bool GetValue(int frame, int& valOut) const override;
};

class CIEImpulse : public CIntElement {
  CIntElement* mA;

public:
  CIEImpulse(CIntElement* a);
  ~CIEImpulse() override;
  bool GetValue(int frame, int& valOut) const override;
};

class CIETimescale : public CIntElement {
  CRealElement* mA;

public:
  CIETimescale(CRealElement* a);
  ~CIETimescale();
  bool GetValue(int frame, int& valOut) const override;
};

class CIEInitialRandom : public CIntElement {
  CIntElement* mA;
  CIntElement* mB;

public:
  CIEInitialRandom(CIntElement* a, CIntElement* b);
  ~CIEInitialRandom() override;
  bool GetValue(int frame, int& valOut) const override;
};

class CIETimeChain : public CIntElement {
  CIntElement* mA;
  CIntElement* mB;
  CIntElement* mSwFrame;

public:
  CIETimeChain(CIntElement* a, CIntElement* b, CIntElement* c);
  ~CIETimeChain() override;
  bool GetValue(int frame, int& valOut) const override;
};

class CIEClamp : public CIntElement {
  CIntElement* mMin;
  CIntElement* mMax;
  CIntElement* mVal;

public:
  CIEClamp(CIntElement* a, CIntElement* b, CIntElement* c);
  ~CIEClamp() override;
  bool GetValue(int frame, int& valOut) const override;
};

class CIEPulse : public CIntElement {
  CIntElement* mADuration;
  CIntElement* mBDuration;
  CIntElement* mAVal;
  CIntElement* mBVal;

public:
  CIEPulse(CIntElement* a, CIntElement* b, CIntElement* c, CIntElement* d);
  ~CIEPulse() override;
  bool GetValue(int frame, int& valOut) const override;
};

class CIELifetimePercent : public CIntElement {
  CIntElement* mPercentVal;

public:
  CIELifetimePercent(CIntElement* a);
  ~CIELifetimePercent() override;
  bool GetValue(int frame, int& valOut) const override;
};

class CIEKeyframeEmitter : public CIntElement {
  int mPercent;
  int mUnk1;
  bool mLoop;
  bool mUnk2;
  int mLoopEnd;
  int mLoopStart;
  rstl::vector< int > mKeys;

public:
  CIEKeyframeEmitter(CInputStream& in);
  ~CIEKeyframeEmitter() override;
  bool GetValue(int frame, int& valOut) const override;
};

class CIESubtract : public CIntElement {
  CIntElement* mA;
  CIntElement* mB;

public:
  CIESubtract(CIntElement* a, CIntElement* b);
  ~CIESubtract() override;
  bool GetValue(int frame, int& valOut) const override;
};

class CIERealToInt : public CIntElement {
  CRealElement* mA;
  CRealElement* mB;

public:
  CIERealToInt(CRealElement* a, CRealElement* b);
  ~CIERealToInt() override;
  bool GetValue(int frame, int& valOut) const override;
};

class CIEGetCumulativeParticleCount : public CIntElement {
public:
  ~CIEGetCumulativeParticleCount() override {}
  bool GetValue(int frame, int& valOut) const override;
};

class CIEGetActiveParticleCount : public CIntElement {
public:
  ~CIEGetActiveParticleCount() override {}
  bool GetValue(int frame, int& valOut) const override;
};

class CIEGetEmitterTime : public CIntElement {
public:
  ~CIEGetEmitterTime() override;
  bool GetValue(int frame, int& valOut) const override;
};

#endif // _CINTELEMENT
