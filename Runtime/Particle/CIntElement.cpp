#include "Runtime/Particle/CIntElement.hpp"

#include <algorithm>

#include "Runtime/CRandom16.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/Particle/CParticleGlobals.hpp"

/* Documentation at: https://wiki.axiodl.com/w/Particle_Script#Int_Elements */

namespace metaforce {

CIEKeyframeEmitter::CIEKeyframeEmitter(CInputStream& in) {
  x4_percent = in.ReadLong();
  x8_unk1 = in.ReadLong();
  xc_loop = in.ReadBool();
  xd_unk2 = in.ReadBool();
  x10_loopEnd = in.ReadLong();
  x14_loopStart = in.ReadLong();

  u32 count = in.ReadLong();
  x18_keys.reserve(count);
  for (u32 i = 0; i < count; ++i)
    x18_keys.push_back(in.ReadInt32());
}

bool CIEKeyframeEmitter::GetValue([[maybe_unused]] int frame, int& valOut) const {
  if (x4_percent == 0) {
    int emitterTime = CParticleGlobals::instance()->m_EmitterTime;
    if (xc_loop) {
      if (emitterTime >= x10_loopEnd) {
        emitterTime -= x14_loopStart;
        emitterTime = emitterTime % (x10_loopEnd - x14_loopStart);
        emitterTime += x14_loopStart;
      }
      valOut = x18_keys[emitterTime];
    } else {
      emitterTime = std::min<int>(emitterTime, x10_loopEnd - 1);
      valOut = x18_keys[emitterTime];
    }
    return false;
  } else {
    int ltPerc = CParticleGlobals::instance()->m_ParticleLifetimePercentage;
    if (ltPerc == 100) {
      valOut = x18_keys[ltPerc];
    } else {
      float ltPercRem = CParticleGlobals::instance()->m_ParticleLifetimePercentageRemainder;
      float lerp = (1.0f - ltPercRem) * x18_keys[ltPerc] + ltPercRem * x18_keys[ltPerc + 1];
      valOut = static_cast<int>(lerp);
    }
    return false;
  }
}

int CIEKeyframeEmitter::GetMaxValue() const { return *std::max_element(x18_keys.cbegin(), x18_keys.cend()); }

bool CIEDeath::GetValue(int frame, int& valOut) const {
  int b;
  x4_a->GetValue(frame, valOut);
  x8_b->GetValue(frame, b);
  return frame >= b;
}

int CIEDeath::GetMaxValue() const { return x4_a->GetMaxValue(); }

bool CIEClamp::GetValue(int frame, int& valOut) const {
  int a, b;
  x4_min->GetValue(frame, a);
  x8_max->GetValue(frame, b);
  xc_val->GetValue(frame, valOut);
  if (valOut > b) {
    valOut = b;
  }
  if (valOut < a) {
    valOut = a;
  }
  return false;
}

int CIEClamp::GetMaxValue() const {
  const int a = x4_min->GetMaxValue();
  const int b = x8_max->GetMaxValue();
  const int valOut = xc_val->GetMaxValue();

  return std::clamp(valOut, a, b);
}

bool CIETimeChain::GetValue(int frame, int& valOut) const {
  int v;
  xc_swFrame->GetValue(frame, v);
  if (frame < v) {
    return x4_a->GetValue(frame, valOut);
  } else {
    return x8_b->GetValue(frame - v, valOut);
  }
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

bool CIEConstant::GetValue([[maybe_unused]] int frame, int& valOut) const {
  valOut = x4_val;
  return false;
}

int CIEConstant::GetMaxValue() const { return x4_val; }

bool CIEImpulse::GetValue(int frame, int& valOut) const {
  if (frame == 0) {
    x4_a->GetValue(frame, valOut);
  } else {
    valOut = 0;
  }
  return false;
}

int CIEImpulse::GetMaxValue() const { return x4_a->GetMaxValue(); }

bool CIELifetimePercent::GetValue(int frame, int& valOut) const {
  int a = 0;
  x4_percentVal->GetValue(frame, a);
  if (a < 0) {
    a = 0;
  }
  valOut = (a / 100.0f) * CParticleGlobals::instance()->m_ParticleLifetimeReal + 0.5f;
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
  int cv = a + b + 1;
  if (cv < 0) {
    cv = 1;
  }

  if (b >= 1) {
    if (frame % cv > a) {
      x10_bVal->GetValue(frame, valOut);
    } else {
      xc_aVal->GetValue(frame, valOut);
    }
  } else {
    xc_aVal->GetValue(frame, valOut);
  }
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
  bool ret;
  if (x8_nextSampleFrame < frame) {
    int b, c;
    xc_waitFramesMin->GetValue(frame, b);
    x10_waitFramesMax->GetValue(frame, c);
    x8_nextSampleFrame = CRandom16::GetRandomNumber()->Range(b, c) + frame;
    ret = x4_sampleSource->GetValue(frame, valOut);
    x14_holdVal = valOut;
  } else {
    valOut = x14_holdVal;
    ret = false;
  }
  return ret;
}

int CIESampleAndHold::GetMaxValue() const { return x4_sampleSource->GetMaxValue(); }

bool CIERandom::GetValue(int frame, int& valOut) const {
  int a, b;
  x4_min->GetValue(frame, a);
  x8_max->GetValue(frame, b);
  if (a > 0) {
    valOut = CRandom16::GetRandomNumber()->Range(a, b);
  } else {
    valOut = CRandom16::GetRandomNumber()->Next();
  }
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
  valOut = static_cast< float >(frame) * a;
  return false;
}

int CIETimeScale::GetMaxValue() const { return 10000; /* Assume 10000 frames max (not ideal estimate) */ }

bool CIEGetCumulativeParticleCount::GetValue([[maybe_unused]] int frame, int& valOut) const {
  valOut = CParticleGlobals::instance()->m_currentParticleSystem->x4_system->GetCumulativeParticleCount();
  return false;
}

int CIEGetCumulativeParticleCount::GetMaxValue() const { return 256; }

bool CIEGetActiveParticleCount::GetValue([[maybe_unused]] int frame, int& valOut) const {
  valOut = CParticleGlobals::instance()->m_currentParticleSystem->x4_system->GetParticleCount();
  return false;
}

int CIEGetActiveParticleCount::GetMaxValue() const { return 256; }

bool CIEGetEmitterTime::GetValue([[maybe_unused]] int frame, int& valOut) const {
  valOut = CParticleGlobals::instance()->m_currentParticleSystem->x4_system->GetEmitterTime();
  return false;
}

int CIEGetEmitterTime::GetMaxValue() const { return 10000; /* Assume 10000 frames max (not ideal estimate) */ }

bool CIEModulo::GetValue(int frame, int& valOut) const {
  int a, b;
  x4_a->GetValue(frame, a);
  x8_b->GetValue(frame, b);
  if (b != 0) {
    valOut = a % b;
  } else {
    valOut = a;
  }
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

bool CIERealToInt::GetValue(int frame, int& valOut) const {
  float a = 0.0f;
  float b = 1.0f;

  x8_b->GetValue(frame, b);
  x4_a->GetValue(frame, a);

  valOut = static_cast<int>(a * b);
  return false;
}

int CIERealToInt::GetMaxValue() const {
  // TODO: Implement
  return 1;
}

} // namespace metaforce
