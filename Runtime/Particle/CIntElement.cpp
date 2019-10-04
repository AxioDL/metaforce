#include "Runtime/Particle/CIntElement.hpp"

#include "Runtime/CRandom16.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/Particle/CParticleGlobals.hpp"

/* Documentation at: http://www.metroid2002.com/retromodding/wiki/Particle_Script#Int_Elements */

namespace urde {

CIEKeyframeEmitter::CIEKeyframeEmitter(CInputStream& in) {
  x4_percent = in.readUint32Big();
  x8_unk1 = in.readUint32Big();
  xc_loop = in.readBool();
  xd_unk2 = in.readBool();
  x10_loopEnd = in.readUint32Big();
  x14_loopStart = in.readUint32Big();

  u32 count = in.readUint32Big();
  x18_keys.reserve(count);
  for (u32 i = 0; i < count; ++i)
    x18_keys.push_back(in.readInt32Big());
}

bool CIEKeyframeEmitter::GetValue(int frame, int& valOut) const {
  if (!x4_percent) {
    int emitterTime = CParticleGlobals::g_EmitterTime;
    int calcKey = emitterTime;
    if (xc_loop) {
      if (emitterTime >= x10_loopEnd) {
        int v1 = emitterTime - x14_loopStart;
        int v2 = x10_loopEnd - x14_loopStart;
        calcKey = v1 % v2;
        calcKey += x14_loopStart;
      }
    } else {
      int v1 = x10_loopEnd - 1;
      if (v1 < emitterTime)
        calcKey = v1;
    }
    valOut = x18_keys[calcKey];
  } else {
    int ltPerc = CParticleGlobals::g_ParticleLifetimePercentage;
    float ltPercRem = CParticleGlobals::g_ParticleLifetimePercentageRemainder;
    if (ltPerc == 100)
      valOut = x18_keys[100];
    else
      valOut = ltPercRem * x18_keys[ltPerc + 1] + (1.0f - ltPercRem) * x18_keys[ltPerc];
  }
  return false;
}

int CIEKeyframeEmitter::GetMaxValue() const {
  int maxVal = INT_MIN;
  for (int k : x18_keys)
    if (k > maxVal)
      maxVal = k;
  return maxVal;
}

bool CIEDeath::GetValue(int frame, int& valOut) const {
  x4_a->GetValue(frame, valOut);
  int b;
  x8_b->GetValue(frame, b);
  /* Not 100% sure about this, originally some kinda branchless comparison */
  return frame > b;
}

int CIEDeath::GetMaxValue() const { return x4_a->GetMaxValue(); }

bool CIEClamp::GetValue(int frame, int& valOut) const {
  int a, b;
  x4_min->GetValue(frame, a);
  x8_max->GetValue(frame, b);
  xc_val->GetValue(frame, valOut);
  if (valOut > b)
    valOut = b;
  if (valOut < a)
    valOut = a;
  return false;
}

int CIEClamp::GetMaxValue() const {
  const int a = x4_min->GetMaxValue();
  const int b = x8_max->GetMaxValue();
  int valOut = xc_val->GetMaxValue();

  if (valOut > b) {
    valOut = b;
  }
  if (valOut < a) {
    valOut = a;
  }

  return valOut;
}

bool CIETimeChain::GetValue(int frame, int& valOut) const {
  int v;
  xc_swFrame->GetValue(frame, v);
  if (frame >= v)
    return x8_b->GetValue(frame, valOut);
  else
    return x4_a->GetValue(frame, valOut);
}

int CIETimeChain::GetMaxValue() const { return std::max(x8_b->GetMaxValue(), x4_a->GetMaxValue()); }

bool CIEAdd::GetValue(int frame, int& valOut) const {
  int a, b;
  x4_a->GetValue(frame, a);
  x8_b->GetValue(frame, b);
  valOut = a + b;
  return false;
}

int CIEAdd::GetMaxValue() const {
  const int a = x4_a->GetMaxValue();
  const int b = x8_b->GetMaxValue();
  return a + b;
}

bool CIEConstant::GetValue(int frame, int& valOut) const {
  valOut = x4_val;
  return false;
}

int CIEConstant::GetMaxValue() const { return x4_val; }

bool CIEImpulse::GetValue(int frame, int& valOut) const {
  if (frame == 0)
    x4_a->GetValue(frame, valOut);
  else
    valOut = 0;
  return false;
}

int CIEImpulse::GetMaxValue() const { return x4_a->GetMaxValue(); }

bool CIELifetimePercent::GetValue(int frame, int& valOut) const {
  int a;
  x4_percentVal->GetValue(frame, a);
  a = std::max(0, a);
  valOut = (a / 100.0f) * CParticleGlobals::g_ParticleLifetimeReal + 0.5f;
  return false;
}

int CIELifetimePercent::GetMaxValue() const {
  const int a = std::max(0, x4_percentVal->GetMaxValue());

  // Assume 10000 frames max (not ideal estimate)
  return int((float(a) / 100.0f) * 10000 + 0.5f);
}

bool CIEInitialRandom::GetValue(int frame, int& valOut) const {
  if (frame == 0) {
    int a, b;
    x4_a->GetValue(frame, a);
    x8_b->GetValue(frame, b);
    valOut = CRandom16::GetRandomNumber()->Range(a, b);
  }
  return false;
}

int CIEInitialRandom::GetMaxValue() const { return x8_b->GetMaxValue(); }

bool CIEPulse::GetValue(int frame, int& valOut) const {
  int a, b;
  x4_aDuration->GetValue(frame, a);
  x8_bDuration->GetValue(frame, b);
  int cv = std::max(1, a + b + 1);

  if (b >= 1) {
    int cv2 = frame % cv;
    if (cv2 >= a)
      x10_bVal->GetValue(frame, valOut);
    else
      xc_aVal->GetValue(frame, valOut);
  } else
    xc_aVal->GetValue(frame, valOut);

  return false;
}

int CIEPulse::GetMaxValue() const { return std::max(xc_aVal->GetMaxValue(), x10_bVal->GetMaxValue()); }

bool CIEMultiply::GetValue(int frame, int& valOut) const {
  int a, b;
  x4_a->GetValue(frame, a);
  x8_b->GetValue(frame, b);
  valOut = a * b;
  return false;
}

int CIEMultiply::GetMaxValue() const { return x4_a->GetMaxValue() * x8_b->GetMaxValue(); }

bool CIESampleAndHold::GetValue(int frame, int& valOut) const {
  if (x8_nextSampleFrame < frame) {
    int b, c;
    xc_waitFramesMin->GetValue(frame, b);
    x10_waitFramesMax->GetValue(frame, c);
    /* const-correctness, who needs it? */
    const_cast<CIESampleAndHold*>(this)->x8_nextSampleFrame = CRandom16::GetRandomNumber()->Range(b, c) + frame;
    x4_sampleSource->GetValue(frame, valOut);
    const_cast<CIESampleAndHold*>(this)->x14_holdVal = valOut;
  } else
    valOut = x14_holdVal;
  return false;
}

int CIESampleAndHold::GetMaxValue() const { return x4_sampleSource->GetMaxValue(); }

bool CIERandom::GetValue(int frame, int& valOut) const {
  int a, b;
  x4_min->GetValue(frame, a);
  x8_max->GetValue(frame, b);
  if (a > 0)
    valOut = CRandom16::GetRandomNumber()->Range(a, b);
  else
    valOut = CRandom16::GetRandomNumber()->Next();
  return false;
}

int CIERandom::GetMaxValue() const {
  if (x4_min->GetMaxValue() > 0)
    return x8_max->GetMaxValue();
  else
    return 65535;
}

bool CIETimeScale::GetValue(int frame, int& valOut) const {
  float a;
  x4_a->GetValue(frame, a);
  valOut = float(frame) * a;
  return false;
}

int CIETimeScale::GetMaxValue() const { return 10000; /* Assume 10000 frames max (not ideal estimate) */ }

bool CIEGetCumulativeParticleCount::GetValue(int frame, int& valOut) const {
  valOut = CParticleGlobals::g_currentParticleSystem->x4_system->GetCumulativeParticleCount();
  return false;
}

int CIEGetCumulativeParticleCount::GetMaxValue() const { return 256; }

bool CIEGetActiveParticleCount::GetValue(int frame, int& valOut) const {
  valOut = CParticleGlobals::g_currentParticleSystem->x4_system->GetParticleCount();
  return false;
}

int CIEGetActiveParticleCount::GetMaxValue() const { return 256; }

bool CIEGetEmitterTime::GetValue(int frame, int& valOut) const {
  valOut = CParticleGlobals::g_currentParticleSystem->x4_system->GetEmitterTime();
  return false;
}

int CIEGetEmitterTime::GetMaxValue() const { return 10000; /* Assume 10000 frames max (not ideal estimate) */ }

bool CIEModulo::GetValue(int frame, int& valOut) const {
  int a, b;
  x4_a->GetValue(frame, a);
  x8_b->GetValue(frame, b);
  if (b != 0)
    valOut = a % b;
  else
    valOut = a;
  return false;
}

int CIEModulo::GetMaxValue() const {
  const int a = x4_a->GetMaxValue();
  const int b = x8_b->GetMaxValue();

  if (b != 0) {
    return b - 1;
  }

  return a;
}

bool CIESubtract::GetValue(int frame, int& valOut) const {
  int a, b;
  x4_a->GetValue(frame, a);
  x8_b->GetValue(frame, b);
  valOut = a - b;
  return false;
}

int CIESubtract::GetMaxValue() const {
  const int a = x4_a->GetMaxValue();
  const int b = x8_b->GetMaxValue();
  return a - b;
}

} // namespace urde
