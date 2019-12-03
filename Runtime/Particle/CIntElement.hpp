#pragma once

#include <memory>
#include <vector>

#include "Runtime/Particle/IElement.hpp"

/* Documentation at: https://wiki.axiodl.com/w/Particle_Script#Int_Elements */

namespace urde {

class CIEKeyframeEmitter : public CIntElement {
  u32 x4_percent;
  u32 x8_unk1;
  bool xc_loop;
  bool xd_unk2;
  u32 x10_loopEnd;
  u32 x14_loopStart;
  std::vector<int> x18_keys;

public:
  CIEKeyframeEmitter(CInputStream& in);
  bool GetValue(int frame, int& valOut) const override;
  int GetMaxValue() const override;
};

class CIEDeath : public CIntElement {
  std::unique_ptr<CIntElement> x4_a;
  std::unique_ptr<CIntElement> x8_b;

public:
  CIEDeath(std::unique_ptr<CIntElement>&& a, std::unique_ptr<CIntElement>&& b)
  : x4_a(std::move(a)), x8_b(std::move(b)) {}
  bool GetValue(int frame, int& valOut) const override;
  int GetMaxValue() const override;
};

class CIEClamp : public CIntElement {
  std::unique_ptr<CIntElement> x4_min;
  std::unique_ptr<CIntElement> x8_max;
  std::unique_ptr<CIntElement> xc_val;

public:
  CIEClamp(std::unique_ptr<CIntElement>&& a, std::unique_ptr<CIntElement>&& b, std::unique_ptr<CIntElement>&& c)
  : x4_min(std::move(a)), x8_max(std::move(b)), xc_val(std::move(c)) {}
  bool GetValue(int frame, int& valOut) const override;
  int GetMaxValue() const override;
};

class CIETimeChain : public CIntElement {
  std::unique_ptr<CIntElement> x4_a;
  std::unique_ptr<CIntElement> x8_b;
  std::unique_ptr<CIntElement> xc_swFrame;

public:
  CIETimeChain(std::unique_ptr<CIntElement>&& a, std::unique_ptr<CIntElement>&& b, std::unique_ptr<CIntElement>&& c)
  : x4_a(std::move(a)), x8_b(std::move(b)), xc_swFrame(std::move(c)) {}
  bool GetValue(int frame, int& valOut) const override;
  int GetMaxValue() const override;
};

class CIEAdd : public CIntElement {
  std::unique_ptr<CIntElement> x4_a;
  std::unique_ptr<CIntElement> x8_b;

public:
  CIEAdd(std::unique_ptr<CIntElement>&& a, std::unique_ptr<CIntElement>&& b) : x4_a(std::move(a)), x8_b(std::move(b)) {}
  bool GetValue(int frame, int& valOut) const override;
  int GetMaxValue() const override;
};

class CIEConstant : public CIntElement {
  int x4_val;

public:
  CIEConstant(int val) : x4_val(val) {}
  bool GetValue(int frame, int& valOut) const override;
  int GetMaxValue() const override;
};

class CIEImpulse : public CIntElement {
  std::unique_ptr<CIntElement> x4_a;

public:
  CIEImpulse(std::unique_ptr<CIntElement>&& a) : x4_a(std::move(a)) {}
  bool GetValue(int frame, int& valOut) const override;
  int GetMaxValue() const override;
};

class CIELifetimePercent : public CIntElement {
  std::unique_ptr<CIntElement> x4_percentVal;

public:
  CIELifetimePercent(std::unique_ptr<CIntElement>&& a) : x4_percentVal(std::move(a)) {}
  bool GetValue(int frame, int& valOut) const override;
  int GetMaxValue() const override;
};

class CIEInitialRandom : public CIntElement {
  std::unique_ptr<CIntElement> x4_a;
  std::unique_ptr<CIntElement> x8_b;

public:
  CIEInitialRandom(std::unique_ptr<CIntElement>&& a, std::unique_ptr<CIntElement>&& b)
  : x4_a(std::move(a)), x8_b(std::move(b)) {}
  bool GetValue(int frame, int& valOut) const override;
  int GetMaxValue() const override;
};

class CIEPulse : public CIntElement {
  std::unique_ptr<CIntElement> x4_aDuration;
  std::unique_ptr<CIntElement> x8_bDuration;
  std::unique_ptr<CIntElement> xc_aVal;
  std::unique_ptr<CIntElement> x10_bVal;

public:
  CIEPulse(std::unique_ptr<CIntElement>&& a, std::unique_ptr<CIntElement>&& b, std::unique_ptr<CIntElement>&& c,
           std::unique_ptr<CIntElement>&& d)
  : x4_aDuration(std::move(a)), x8_bDuration(std::move(b)), xc_aVal(std::move(c)), x10_bVal(std::move(d)) {}
  bool GetValue(int frame, int& valOut) const override;
  int GetMaxValue() const override;
};

class CIEMultiply : public CIntElement {
  std::unique_ptr<CIntElement> x4_a;
  std::unique_ptr<CIntElement> x8_b;

public:
  CIEMultiply(std::unique_ptr<CIntElement>&& a, std::unique_ptr<CIntElement>&& b)
  : x4_a(std::move(a)), x8_b(std::move(b)) {}
  bool GetValue(int frame, int& valOut) const override;
  int GetMaxValue() const override;
};

class CIESampleAndHold : public CIntElement {
  std::unique_ptr<CIntElement> x4_sampleSource;
  int x8_nextSampleFrame = 0;
  std::unique_ptr<CIntElement> xc_waitFramesMin;
  std::unique_ptr<CIntElement> x10_waitFramesMax;
  int x14_holdVal;

public:
  CIESampleAndHold(std::unique_ptr<CIntElement>&& a, std::unique_ptr<CIntElement>&& b, std::unique_ptr<CIntElement>&& c)
  : x4_sampleSource(std::move(a)), xc_waitFramesMin(std::move(b)), x10_waitFramesMax(std::move(c)) {}
  bool GetValue(int frame, int& valOut) const override;
  int GetMaxValue() const override;
};

class CIERandom : public CIntElement {
  std::unique_ptr<CIntElement> x4_min;
  std::unique_ptr<CIntElement> x8_max;

public:
  CIERandom(std::unique_ptr<CIntElement>&& a, std::unique_ptr<CIntElement>&& b)
  : x4_min(std::move(a)), x8_max(std::move(b)) {}
  bool GetValue(int frame, int& valOut) const override;
  int GetMaxValue() const override;
};

class CIETimeScale : public CIntElement {
  std::unique_ptr<CRealElement> x4_a;

public:
  CIETimeScale(std::unique_ptr<CRealElement>&& a) : x4_a(std::move(a)) {}
  bool GetValue(int frame, int& valOut) const override;
  int GetMaxValue() const override;
};

class CIEGetCumulativeParticleCount : public CIntElement {
public:
  bool GetValue(int frame, int& valOut) const override;
  int GetMaxValue() const override;
};

class CIEGetActiveParticleCount : public CIntElement {
public:
  bool GetValue(int frame, int& valOut) const override;
  int GetMaxValue() const override;
};

class CIEGetEmitterTime : public CIntElement {
public:
  bool GetValue(int frame, int& valOut) const override;
  int GetMaxValue() const override;
};

class CIEModulo : public CIntElement {
  std::unique_ptr<CIntElement> x4_a;
  std::unique_ptr<CIntElement> x8_b;

public:
  CIEModulo(std::unique_ptr<CIntElement>&& a, std::unique_ptr<CIntElement>&& b)
  : x4_a(std::move(a)), x8_b(std::move(b)) {}
  bool GetValue(int frame, int& valOut) const override;
  int GetMaxValue() const override;
};

class CIESubtract : public CIntElement {
  std::unique_ptr<CIntElement> x4_a;
  std::unique_ptr<CIntElement> x8_b;

public:
  CIESubtract(std::unique_ptr<CIntElement>&& a, std::unique_ptr<CIntElement>&& b)
  : x4_a(std::move(a)), x8_b(std::move(b)) {}
  bool GetValue(int frame, int& valOut) const override;
  int GetMaxValue() const override;
};

class CIERealToInt final : public CIntElement {
  std::unique_ptr<CRealElement> x4_a;
  std::unique_ptr<CRealElement> x8_b;

public:
  explicit CIERealToInt(std::unique_ptr<CRealElement>&& a, std::unique_ptr<CRealElement>&& b)
  : x4_a{std::move(a)}, x8_b{std::move(b)} {}

  bool GetValue(int frame, int& valOut) const override;
  int GetMaxValue() const override;
};

} // namespace urde
