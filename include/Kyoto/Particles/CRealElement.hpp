#ifndef _CREALELEMENT
#define _CREALELEMENT

#include "types.h"

#include "Kyoto/Particles/IElement.hpp"

#include "rstl/vector.hpp"

class CREConstant : public CRealElement {
  float mVal;

public:
  CREConstant(float val);
  ~CREConstant() override;
  bool GetValue(int frame, float& valOut) const override;
  bool IsConstant() const override { return true; }
};

class CRESineWave : public CRealElement {
  CRealElement* mFrequency;
  CRealElement* mAmplitude;
  CRealElement* mPhase;

public:
  CRESineWave(CRealElement* a, CRealElement* b, CRealElement* c);
  ~CRESineWave() override;
  bool GetValue(int frame, float& valOut) const override;
};

class CRETimeScale : public CRealElement {
  CRealElement* mA;

public:
  CRETimeScale(CRealElement* a);
  ~CRETimeScale() override;
  bool GetValue(int frame, float& valOut) const override;
};

class CREAdd : public CRealElement {
  CRealElement* mA;
  CRealElement* mB;

public:
  CREAdd(CRealElement* a, CRealElement* b);
  ~CREAdd() override;
  bool GetValue(int frame, float& valOut) const override;
};

class CREMultiply : public CRealElement {
  CRealElement* mA;
  CRealElement* mB;

public:
  CREMultiply(CRealElement* a, CRealElement* b);
  ~CREMultiply() override;
  bool GetValue(int frame, float& valOut) const override;
};

class CREDotProduct : public CRealElement {
  CVectorElement* mA;
  CVectorElement* mB;

public:
  CREDotProduct(CVectorElement* a, CVectorElement* b);
  ~CREDotProduct() override;
  bool GetValue(int frame, float& valOut) const override;
};

class CRERandom : public CRealElement {
  CRealElement* mMin;
  CRealElement* mMax;

public:
  CRERandom(CRealElement* min, CRealElement* max);
  ~CRERandom() override;
  bool GetValue(int frame, float& valOut) const override;
};

class CREInitialRandom : public CRealElement {
  CRealElement* mMin;
  CRealElement* mMax;

public:
  CREInitialRandom(CRealElement* min, CRealElement* max);
  ~CREInitialRandom() override;
  bool GetValue(int frame, float& valOut) const override;
  bool IsConstant() const override { return true; }
};

class CRETimeChain : public CRealElement {
  CRealElement* mA;
  CRealElement* mB;
  CIntElement* mSwFrame;

public:
  CRETimeChain(CRealElement* a, CRealElement* b, CIntElement* c);
  ~CRETimeChain() override;
  bool GetValue(int frame, float& valOut) const override;
};

class CREClamp : public CRealElement {
  CRealElement* mMin;
  CRealElement* mMax;
  CRealElement* mVal;

public:
  CREClamp(CRealElement* a, CRealElement* b, CRealElement* c);
  ~CREClamp() override;
  bool GetValue(int frame, float& valOut) const override;
};

class CREPulse : public CRealElement {
  CIntElement* mADuration;
  CIntElement* mBDuration;
  CRealElement* mValA;
  CRealElement* mValB;

public:
  CREPulse(CIntElement* a, CIntElement* b, CRealElement* c, CRealElement* d);
  ~CREPulse() override;
  bool GetValue(int frame, float& valOut) const override;
};

class CRELifetimePercent : public CRealElement {
  CRealElement* mPercentVal;

public:
  CRELifetimePercent(CRealElement* a); // : x4_percentVal(a) {}
  ~CRELifetimePercent() override;
  bool GetValue(int frame, float& valOut) const override;
};

class CRELifetimeTween : public CRealElement {
  CRealElement* mA;
  CRealElement* mB;

public:
  CRELifetimeTween(CRealElement* a, CRealElement* b);
  ~CRELifetimeTween() override;
  bool GetValue(int frame, float& valOut) const override;
};

class CREKeyframeEmitter : public CRealElement {
  int mPercent;
  int mUnk1;
  bool mLoop;
  bool mUnk2;
  int mLoopEnd;
  int mLoopStart;
  rstl::vector< float > mKeys;

public:
  CREKeyframeEmitter(CInputStream& in);
  ~CREKeyframeEmitter() override;
  bool GetValue(int frame, float& valOut) const override;
};

class CREParticleAccessParameter1 : public CRealElement {
public:
  ~CREParticleAccessParameter1() override {}
  bool GetValue(int frame, float& valOut) const override;
};

class CREParticleAccessParameter2 : public CRealElement {
public:
  ~CREParticleAccessParameter2() override {}
  bool GetValue(int frame, float& valOut) const override;
};

class CREParticleAccessParameter3 : public CRealElement {
public:
  ~CREParticleAccessParameter3() override {}
  bool GetValue(int frame, float& valOut) const override;
};

class CREParticleAccessParameter4 : public CRealElement {
public:
  ~CREParticleAccessParameter4() override {}
  bool GetValue(int frame, float& valOut) const override;
};

class CREParticleAccessParameter5 : public CRealElement {
public:
  ~CREParticleAccessParameter5() override {}
  bool GetValue(int frame, float& valOut) const override;
};

class CREParticleAccessParameter6 : public CRealElement {
public:
  ~CREParticleAccessParameter6() override {}
  bool GetValue(int frame, float& valOut) const override;
};

class CREParticleAccessParameter7 : public CRealElement {
public:
  ~CREParticleAccessParameter7() override {}
  bool GetValue(int frame, float& valOut) const override;
};

class CREParticleAccessParameter8 : public CRealElement {
public:
  ~CREParticleAccessParameter8() override {}
  bool GetValue(int frame, float& valOut) const override;
};

class CREParticleSizeOrLineLength : public CRealElement {
public:
  ~CREParticleSizeOrLineLength() override {}
  bool GetValue(int frame, float& valOut) const override;
};

class CREParticleRotationOrLineWidth : public CRealElement {
public:
  ~CREParticleRotationOrLineWidth() override {}
  bool GetValue(int frame, float& valOut) const override;
};

class CREVectorXToReal : public CRealElement {
  CVectorElement* mA;

public:
  CREVectorXToReal(CVectorElement* a);
  ~CREVectorXToReal() override;
  bool GetValue(int frame, float& valOut) const override;
};

class CREVectorYToReal : public CRealElement {
  CVectorElement* mA;

public:
  CREVectorYToReal(CVectorElement* a);
  ~CREVectorYToReal() override;
  bool GetValue(int frame, float& valOut) const override;
};

class CREVectorZToReal : public CRealElement {
  CVectorElement* mA;

public:
  CREVectorZToReal(CVectorElement* a);
  ~CREVectorZToReal() override;
  bool GetValue(int frame, float& valOut) const override;
};

class CREVectorMagnitude : public CRealElement {
  CVectorElement* mA;

public:
  CREVectorMagnitude(CVectorElement* a);
  ~CREVectorMagnitude() override;
  bool GetValue(int frame, float& valOut) const override;
};

class CREInitialSwitch : public CRealElement {
  CRealElement* mA;
  CRealElement* mB;

public:
  CREInitialSwitch(CRealElement* a, CRealElement* b);
  ~CREInitialSwitch() override;
  bool GetValue(int frame, float& valOut) const override;
};

class CRECompareLessThan : public CRealElement {
  CRealElement* mA;
  CRealElement* mB;
  CRealElement* mC;
  CRealElement* mD;

public:
  CRECompareLessThan(CRealElement* a, CRealElement* b, CRealElement* c, CRealElement* d);
  ~CRECompareLessThan() override;
  bool GetValue(int frame, float& valOut) const override;
};

class CRECompareEqual : public CRealElement {
  CRealElement* mA;
  CRealElement* mB;
  CRealElement* mC;
  CRealElement* mD;

public:
  CRECompareEqual(CRealElement* a, CRealElement* b, CRealElement* c, CRealElement* d);
  ~CRECompareEqual() override;
  bool GetValue(int frame, float& valOut) const override;
};

class CREConstantRange : public CRealElement {
  CRealElement* mVal;
  CRealElement* mMin;
  CRealElement* mMax;
  CRealElement* mInRange;
  CRealElement* mOutOfRange;

public:
  CREConstantRange(CRealElement* a, CRealElement* b, CRealElement* c, CRealElement* d,
                   CRealElement* e);
  ~CREConstantRange() override;
  bool GetValue(int frame, float& valOut) const override;
};

class CREExternalVar : public CRealElement {
  CIntElement* mA;

public:
  CREExternalVar(CIntElement* a);
  ~CREExternalVar() override;
  bool GetValue(int frame, float& valOut) const override;
};

class CRESubtract : public CRealElement {
  CRealElement* mA;
  CRealElement* mB;

public:
  CRESubtract(CRealElement* a, CRealElement* b);
  ~CRESubtract() override;
  bool GetValue(int frame, float& valOut) const override;
};

class CREGetComponentRed : public CRealElement {
  CColorElement* mA;

public:
  CREGetComponentRed(CColorElement* a);
  ~CREGetComponentRed() override;
  bool GetValue(int frame, float& valOut) const override;
};

class CREGetComponentGreen : public CRealElement {
  CColorElement* mA;

public:
  CREGetComponentGreen(CColorElement* a);
  ~CREGetComponentGreen() override;
  bool GetValue(int frame, float& valOut) const override;
};

class CREGetComponentBlue : public CRealElement {
  CColorElement* mA;

public:
  CREGetComponentBlue(CColorElement* a);
  ~CREGetComponentBlue() override;
  bool GetValue(int frame, float& valOut) const override;
};

class CREGetComponentAlpha : public CRealElement {
  CColorElement* mA;

public:
  CREGetComponentAlpha(CColorElement* a);
  ~CREGetComponentAlpha() override;
  bool GetValue(int frame, float& valOut) const override;
};

class CREIntTimesReal : public CRealElement {
  CIntElement* mA;
  CRealElement* mB;

public:
  CREIntTimesReal(CIntElement* a, CRealElement* b);
  ~CREIntTimesReal() override;
  bool GetValue(int frame, float& valOut) const override;
};

#endif // _CREALELEMENT
