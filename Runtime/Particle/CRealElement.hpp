#pragma once

#include "IElement.hpp"

/* Documentation at: http://www.metroid2002.com/retromodding/wiki/Particle_Script#Real_Elements */

namespace urde {

class CREKeyframeEmitter : public CRealElement {
  u32 x4_percent;
  u32 x8_unk1;
  bool xc_loop;
  bool xd_unk2;
  u32 x10_loopEnd;
  u32 x14_loopStart;
  std::vector<float> x18_keys;

public:
  CREKeyframeEmitter(CInputStream& in);
  bool GetValue(int frame, float& valOut) const override;
};

class CRELifetimeTween : public CRealElement {
  std::unique_ptr<CRealElement> x4_a;
  std::unique_ptr<CRealElement> x8_b;

public:
  CRELifetimeTween(std::unique_ptr<CRealElement>&& a, std::unique_ptr<CRealElement>&& b)
  : x4_a(std::move(a)), x8_b(std::move(b)) {}
  bool GetValue(int frame, float& valOut) const override;
};

class CREConstant : public CRealElement {
  float x4_val;

public:
  CREConstant(float val) : x4_val(val) {}
  bool GetValue(int frame, float& valOut) const override;
  bool IsConstant() const override { return true; }
};

class CRETimeChain : public CRealElement {
  std::unique_ptr<CRealElement> x4_a;
  std::unique_ptr<CRealElement> x8_b;
  std::unique_ptr<CIntElement> xc_swFrame;

public:
  CRETimeChain(std::unique_ptr<CRealElement>&& a, std::unique_ptr<CRealElement>&& b, std::unique_ptr<CIntElement>&& c)
  : x4_a(std::move(a)), x8_b(std::move(b)), xc_swFrame(std::move(c)) {}
  bool GetValue(int frame, float& valOut) const override;
};

class CREAdd : public CRealElement {
  std::unique_ptr<CRealElement> x4_a;
  std::unique_ptr<CRealElement> x8_b;

public:
  CREAdd(std::unique_ptr<CRealElement>&& a, std::unique_ptr<CRealElement>&& b)
  : x4_a(std::move(a)), x8_b(std::move(b)) {}
  bool GetValue(int frame, float& valOut) const override;
};

class CREClamp : public CRealElement {
  std::unique_ptr<CRealElement> x4_min;
  std::unique_ptr<CRealElement> x8_max;
  std::unique_ptr<CRealElement> xc_val;

public:
  CREClamp(std::unique_ptr<CRealElement>&& a, std::unique_ptr<CRealElement>&& b, std::unique_ptr<CRealElement>&& c)
  : x4_min(std::move(a)), x8_max(std::move(b)), xc_val(std::move(c)) {}
  bool GetValue(int frame, float& valOut) const override;
};

class CREInitialRandom : public CRealElement {
  std::unique_ptr<CRealElement> x4_min;
  std::unique_ptr<CRealElement> x8_max;

public:
  CREInitialRandom(std::unique_ptr<CRealElement>&& a, std::unique_ptr<CRealElement>&& b)
  : x4_min(std::move(a)), x8_max(std::move(b)) {}
  bool GetValue(int frame, float& valOut) const override;
  bool IsConstant() const override { return true; }
};

class CRERandom : public CRealElement {
  std::unique_ptr<CRealElement> x4_min;
  std::unique_ptr<CRealElement> x8_max;

public:
  CRERandom(std::unique_ptr<CRealElement>&& a, std::unique_ptr<CRealElement>&& b)
  : x4_min(std::move(a)), x8_max(std::move(b)) {}
  bool GetValue(int frame, float& valOut) const override;
};

class CREDotProduct : public CRealElement {
  std::unique_ptr<CVectorElement> x4_a;
  std::unique_ptr<CVectorElement> x8_b;

public:
  CREDotProduct(std::unique_ptr<CVectorElement>&& a, std::unique_ptr<CVectorElement>&& b)
  : x4_a(std::move(a)), x8_b(std::move(b)) {}
  bool GetValue(int frame, float& valOut) const override;
};

class CREMultiply : public CRealElement {
  std::unique_ptr<CRealElement> x4_a;
  std::unique_ptr<CRealElement> x8_b;

public:
  CREMultiply(std::unique_ptr<CRealElement>&& a, std::unique_ptr<CRealElement>&& b)
  : x4_a(std::move(a)), x8_b(std::move(b)) {}
  bool GetValue(int frame, float& valOut) const override;
};

class CREPulse : public CRealElement {
  std::unique_ptr<CIntElement> x4_aDuration;
  std::unique_ptr<CIntElement> x8_bDuration;
  std::unique_ptr<CRealElement> xc_valA;
  std::unique_ptr<CRealElement> x10_valB;

public:
  CREPulse(std::unique_ptr<CIntElement>&& a, std::unique_ptr<CIntElement>&& b, std::unique_ptr<CRealElement>&& c,
           std::unique_ptr<CRealElement>&& d)
  : x4_aDuration(std::move(a)), x8_bDuration(std::move(b)), xc_valA(std::move(c)), x10_valB(std::move(d)) {}
  bool GetValue(int frame, float& valOut) const override;
};

class CRETimeScale : public CRealElement {
  std::unique_ptr<CRealElement> x4_a;

public:
  CRETimeScale(std::unique_ptr<CRealElement>&& a) : x4_a(std::move(a)) {}
  bool GetValue(int frame, float& valOut) const override;
};

class CRELifetimePercent : public CRealElement {
  std::unique_ptr<CRealElement> x4_percentVal;

public:
  CRELifetimePercent(std::unique_ptr<CRealElement>&& a) : x4_percentVal(std::move(a)) {}
  bool GetValue(int frame, float& valOut) const override;
};

class CRESineWave : public CRealElement {
  std::unique_ptr<CRealElement> x4_frequency;
  std::unique_ptr<CRealElement> x8_amplitude;
  std::unique_ptr<CRealElement> xc_phase;

public:
  CRESineWave(std::unique_ptr<CRealElement>&& a, std::unique_ptr<CRealElement>&& b, std::unique_ptr<CRealElement>&& c)
  : x4_frequency(std::move(a)), x8_amplitude(std::move(b)), xc_phase(std::move(c)) {}
  bool GetValue(int frame, float& valOut) const override;
};

class CREInitialSwitch : public CRealElement {
  std::unique_ptr<CRealElement> x4_a;
  std::unique_ptr<CRealElement> x8_b;

public:
  CREInitialSwitch(std::unique_ptr<CRealElement>&& a, std::unique_ptr<CRealElement>&& b)
  : x4_a(std::move(a)), x8_b(std::move(b)) {}
  bool GetValue(int frame, float& valOut) const override;
};

class CRECompareLessThan : public CRealElement {
  std::unique_ptr<CRealElement> x4_a;
  std::unique_ptr<CRealElement> x8_b;
  std::unique_ptr<CRealElement> xc_c;
  std::unique_ptr<CRealElement> x10_d;

public:
  CRECompareLessThan(std::unique_ptr<CRealElement>&& a, std::unique_ptr<CRealElement>&& b,
                     std::unique_ptr<CRealElement>&& c, std::unique_ptr<CRealElement>&& d)
  : x4_a(std::move(a)), x8_b(std::move(b)), xc_c(std::move(c)), x10_d(std::move(d)) {}
  bool GetValue(int frame, float& valOut) const override;
};

class CRECompareEquals : public CRealElement {
  std::unique_ptr<CRealElement> x4_a;
  std::unique_ptr<CRealElement> x8_b;
  std::unique_ptr<CRealElement> xc_c;
  std::unique_ptr<CRealElement> x10_d;

public:
  CRECompareEquals(std::unique_ptr<CRealElement>&& a, std::unique_ptr<CRealElement>&& b,
                   std::unique_ptr<CRealElement>&& c, std::unique_ptr<CRealElement>&& d)
  : x4_a(std::move(a)), x8_b(std::move(b)), xc_c(std::move(c)), x10_d(std::move(d)) {}
  bool GetValue(int frame, float& valOut) const override;
};

class CREParticleAccessParam1 : public CRealElement {
public:
  bool GetValue(int frame, float& valOut) const override;
};

class CREParticleAccessParam2 : public CRealElement {
public:
  bool GetValue(int frame, float& valOut) const override;
};

class CREParticleAccessParam3 : public CRealElement {
public:
  bool GetValue(int frame, float& valOut) const override;
};

class CREParticleAccessParam4 : public CRealElement {
public:
  bool GetValue(int frame, float& valOut) const override;
};

class CREParticleAccessParam5 : public CRealElement {
public:
  bool GetValue(int frame, float& valOut) const override;
};

class CREParticleAccessParam6 : public CRealElement {
public:
  bool GetValue(int frame, float& valOut) const override;
};

class CREParticleAccessParam7 : public CRealElement {
public:
  bool GetValue(int frame, float& valOut) const override;
};

class CREParticleAccessParam8 : public CRealElement {
public:
  bool GetValue(int frame, float& valOut) const override;
};

class CREParticleSizeOrLineLength : public CRealElement {
public:
  bool GetValue(int frame, float& valOut) const override;
};

class CREParticleRotationOrLineWidth : public CRealElement {
public:
  bool GetValue(int frame, float& valOut) const override;
};

class CRESubtract : public CRealElement {
  std::unique_ptr<CRealElement> x4_a;
  std::unique_ptr<CRealElement> x8_b;

public:
  CRESubtract(std::unique_ptr<CRealElement>&& a, std::unique_ptr<CRealElement>&& b)
  : x4_a(std::move(a)), x8_b(std::move(b)) {}
  bool GetValue(int frame, float& valOut) const override;
};

class CREVectorMagnitude : public CRealElement {
  std::unique_ptr<CVectorElement> x4_a;

public:
  CREVectorMagnitude(std::unique_ptr<CVectorElement>&& a) : x4_a(std::move(a)) {}
  bool GetValue(int frame, float& valOut) const override;
};

class CREVectorXToReal : public CRealElement {
  std::unique_ptr<CVectorElement> x4_a;

public:
  CREVectorXToReal(std::unique_ptr<CVectorElement>&& a) : x4_a(std::move(a)) {}
  bool GetValue(int frame, float& valOut) const override;
};

class CREVectorYToReal : public CRealElement {
  std::unique_ptr<CVectorElement> x4_a;

public:
  CREVectorYToReal(std::unique_ptr<CVectorElement>&& a) : x4_a(std::move(a)) {}
  bool GetValue(int frame, float& valOut) const override;
};

class CREVectorZToReal : public CRealElement {
  std::unique_ptr<CVectorElement> x4_a;

public:
  CREVectorZToReal(std::unique_ptr<CVectorElement>&& a) : x4_a(std::move(a)) {}
  bool GetValue(int frame, float& valOut) const override;
};

class CRECEXT : public CRealElement {
  std::unique_ptr<CIntElement> x4_a;

public:
  CRECEXT(std::unique_ptr<CIntElement>&& a) : x4_a(std::move(a)) {}
  bool GetValue(int frame, float& valOut) const override;
};

class CREIntTimesReal : public CRealElement {
  std::unique_ptr<CIntElement> x4_a;
  std::unique_ptr<CRealElement> x8_b;

public:
  CREIntTimesReal(std::unique_ptr<CIntElement>&& a, std::unique_ptr<CRealElement>&& b)
  : x4_a(std::move(a)), x8_b(std::move(b)) {}
  bool GetValue(int frame, float& valOut) const override;
};

class CREConstantRange : public CRealElement {
  std::unique_ptr<CRealElement> x4_val;
  std::unique_ptr<CRealElement> x8_min;
  std::unique_ptr<CRealElement> xc_max;
  std::unique_ptr<CRealElement> x10_inRange;
  std::unique_ptr<CRealElement> x14_outOfRange;

public:
  CREConstantRange(std::unique_ptr<CRealElement>&& a, std::unique_ptr<CRealElement>&& b,
                   std::unique_ptr<CRealElement>&& c, std::unique_ptr<CRealElement>&& d,
                   std::unique_ptr<CRealElement>&& e)
  : x4_val(std::move(a))
  , x8_min(std::move(b))
  , xc_max(std::move(c))
  , x10_inRange(std::move(d))
  , x14_outOfRange(std::move(e)) {}

  bool GetValue(int frame, float& valOut) const override;
};

class CREGetComponentRed : public CRealElement {
  std::unique_ptr<CColorElement> x4_a;

public:
  CREGetComponentRed(std::unique_ptr<CColorElement>&& a) : x4_a(std::move(a)) {}

  bool GetValue(int frame, float& valOut) const override;
};

class CREGetComponentGreen : public CRealElement {
  std::unique_ptr<CColorElement> x4_a;

public:
  CREGetComponentGreen(std::unique_ptr<CColorElement>&& a) : x4_a(std::move(a)) {}

  bool GetValue(int frame, float& valOut) const override;
};

class CREGetComponentBlue : public CRealElement {
  std::unique_ptr<CColorElement> x4_a;

public:
  CREGetComponentBlue(std::unique_ptr<CColorElement>&& a) : x4_a(std::move(a)) {}

  bool GetValue(int frame, float& valOut) const override;
};

class CREGetComponentAlpha : public CRealElement {
  std::unique_ptr<CColorElement> x4_a;

public:
  CREGetComponentAlpha(std::unique_ptr<CColorElement>&& a) : x4_a(std::move(a)) {}

  bool GetValue(int frame, float& valOut) const override;
};
} // namespace urde
