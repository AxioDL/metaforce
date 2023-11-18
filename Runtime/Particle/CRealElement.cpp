#include "Runtime/Particle/CRealElement.hpp"

#include "Runtime/CRandom16.hpp"
#include "Runtime/Graphics/CTexture.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/Particle/CParticleGlobals.hpp"

#include <zeus/Math.hpp>

/* Documentation at: https://wiki.axiodl.com/w/Particle_Script#Real_Elements */

namespace metaforce {

CREKeyframeEmitter::CREKeyframeEmitter(CInputStream& in) {
  x4_percent = in.ReadLong();
  x8_unk1 = in.ReadLong();
  xc_loop = in.ReadBool();
  xd_unk2 = in.ReadBool();
  x10_loopEnd = in.ReadLong();
  x14_loopStart = in.ReadLong();

  u32 count = in.ReadLong();
  x18_keys.reserve(count);
  for (u32 i = 0; i < count; ++i)
    x18_keys.push_back(in.ReadFloat());
}

bool CREKeyframeEmitter::GetValue([[maybe_unused]] int frame, float& valOut) const {
  if (!x4_percent) {
    int emitterTime = CParticleGlobals::instance()->m_EmitterTime;
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
    int ltPerc = CParticleGlobals::instance()->m_ParticleLifetimePercentage;
    float ltPercRem = CParticleGlobals::instance()->m_ParticleLifetimePercentageRemainder;
    if (ltPerc == 100)
      valOut = x18_keys[100];
    else
      valOut = ltPercRem * x18_keys[ltPerc + 1] + (1.0f - ltPercRem) * x18_keys[ltPerc];
  }
  return false;
}

bool CRELifetimeTween::GetValue(int frame, float& valOut) const {
  float ltFac = frame / CParticleGlobals::instance()->m_ParticleLifetimeReal;
  float a, b;
  x4_a->GetValue(frame, a);
  x8_b->GetValue(frame, b);
  valOut = b * ltFac + (1.0f - ltFac) * a;
  return false;
}

bool CREConstant::GetValue([[maybe_unused]] int frame, float& valOut) const {
  valOut = x4_val;
  return false;
}

bool CRETimeChain::GetValue(int frame, float& valOut) const {
  int v;
  xc_swFrame->GetValue(frame, v);
  if (frame < v) {
    return x4_a->GetValue(frame, valOut);
  } else {
    return x8_b->GetValue(frame - v, valOut);
  }
}

bool CREAdd::GetValue(int frame, float& valOut) const {
  float a, b;
  x4_a->GetValue(frame, a);
  x8_b->GetValue(frame, b);
  valOut = a + b;
  return false;
}

bool CREClamp::GetValue(int frame, float& valOut) const {
  float a, b;
  x4_min->GetValue(frame, a);
  x8_max->GetValue(frame, b);
  xc_val->GetValue(frame, valOut);
  if (valOut > b)
    valOut = b;
  if (valOut < a)
    valOut = a;
  return false;
}

bool CREInitialRandom::GetValue(int frame, float& valOut) const {
  if (frame == 0) {
    float a, b;
    x4_min->GetValue(frame, a);
    x8_max->GetValue(frame, b);
    float rand = CRandom16::GetRandomNumber()->Float();
    valOut = b * rand + a * (1.0f - rand);
  }
  return false;
}

bool CRERandom::GetValue(int frame, float& valOut) const {
  float min;
  float max;
  x4_min->GetValue(frame, min);
  x8_max->GetValue(frame, max);
  valOut = (max - min) * CRandom16::GetRandomNumber()->Float() + min;
  return false;
}

bool CREDotProduct::GetValue(int frame, float& valOut) const {
  zeus::CVector3f a, b;
  x4_a->GetValue(frame, a);
  x8_b->GetValue(frame, b);
  valOut = a.dot(b);
  return false;
}

bool CREMultiply::GetValue(int frame, float& valOut) const {
  float a, b;
  x4_a->GetValue(frame, a);
  x8_b->GetValue(frame, b);
  valOut = a * b;
  return false;
}

bool CREPulse::GetValue(int frame, float& valOut) const {
  int a, b;
  x4_aDuration->GetValue(frame, a);
  x8_bDuration->GetValue(frame, b);
  int cv = a + b + 1;
  if (cv < 0) {
    cv = 1;
  }

  // CREPulse is an outlier here, the other
  // IElement classes  use <= instead of <.
  if (b < 1 || frame % cv < a) {
    xc_valA->GetValue(frame, valOut);
  } else {
    x10_valB->GetValue(frame, valOut);
  }
  return false;
}

bool CRETimeScale::GetValue(int frame, float& valOut) const {
  float a;
  x4_a->GetValue(frame, a);
  valOut = float(frame) * a;
  return false;
}

bool CRELifetimePercent::GetValue(int frame, float& valOut) const {
  float a;
  x4_percentVal->GetValue(frame, a);
  a = std::max(0.0f, a);
  valOut = (a / 100.0f) * CParticleGlobals::instance()->m_ParticleLifetimeReal;
  return false;
}

bool CRESineWave::GetValue(int frame, float& valOut) const {
  float a, b, c;
  x4_frequency->GetValue(frame, a);
  x8_amplitude->GetValue(frame, b);
  xc_phase->GetValue(frame, c);
  valOut = std::sin(zeus::degToRad(frame * a + c)) * b;
  return false;
}

bool CREInitialSwitch::GetValue(int frame, float& valOut) const {
  if (frame == 0) {
    x4_a->GetValue(0, valOut);
  } else {
    x8_b->GetValue(frame - 1, valOut);
  }
  return false;
}

bool CRECompareLessThan::GetValue(int frame, float& valOut) const {
  float a, b;
  x4_a->GetValue(frame, a);
  x8_b->GetValue(frame, b);
  if (a < b)
    xc_c->GetValue(frame, valOut);
  else
    x10_d->GetValue(frame, valOut);
  return false;
}

bool CRECompareEquals::GetValue(int frame, float& valOut) const {
  float a, b;
  x4_a->GetValue(frame, a);
  x8_b->GetValue(frame, b);
  if (zeus::close_enough(a, b))
    xc_c->GetValue(frame, valOut);
  else
    x10_d->GetValue(frame, valOut);
  return false;
}

bool CREParticleAccessParam1::GetValue(int /*frame*/, float& valOut) const {
  valOut = (*CParticleGlobals::instance()->m_particleAccessParameters)[0];
  return false;
}

bool CREParticleAccessParam2::GetValue(int /*frame*/, float& valOut) const {
  valOut = (*CParticleGlobals::instance()->m_particleAccessParameters)[1];
  return false;
}

bool CREParticleAccessParam3::GetValue(int /*frame*/, float& valOut) const {
  valOut = (*CParticleGlobals::instance()->m_particleAccessParameters)[2];
  return false;
}

bool CREParticleAccessParam4::GetValue(int /*frame*/, float& valOut) const {
  valOut = (*CParticleGlobals::instance()->m_particleAccessParameters)[3];
  return false;
}

bool CREParticleAccessParam5::GetValue(int /*frame*/, float& valOut) const {
  valOut = (*CParticleGlobals::instance()->m_particleAccessParameters)[4];
  return false;
}

bool CREParticleAccessParam6::GetValue(int /*frame*/, float& valOut) const {
  valOut = (*CParticleGlobals::instance()->m_particleAccessParameters)[5];
  return false;
}

bool CREParticleAccessParam7::GetValue(int /*frame*/, float& valOut) const {
  valOut = (*CParticleGlobals::instance()->m_particleAccessParameters)[6];
  return false;
}

bool CREParticleAccessParam8::GetValue(int /*frame*/, float& valOut) const {
  valOut = (*CParticleGlobals::instance()->m_particleAccessParameters)[7];
  return false;
}

bool CREParticleSizeOrLineLength::GetValue(int /*frame*/, float& valOut) const {
  valOut = CElementGen::g_currentParticle->x2c_lineLengthOrSize;
  return false;
}

bool CREParticleRotationOrLineWidth::GetValue(int /*frame*/, float& valOut) const {
  valOut = CElementGen::g_currentParticle->x30_lineWidthOrRota;
  return false;
}

bool CRESubtract::GetValue(int frame, float& valOut) const {
  float a, b;
  x4_a->GetValue(frame, a);
  x8_b->GetValue(frame, b);
  valOut = a - b;
  return false;
}

bool CREVectorMagnitude::GetValue(int frame, float& valOut) const {
  zeus::CVector3f a;
  x4_a->GetValue(frame, a);
  valOut = a.magnitude();
  return false;
}

bool CREVectorXToReal::GetValue(int frame, float& valOut) const {
  zeus::CVector3f a;
  x4_a->GetValue(frame, a);
  valOut = a[0];
  return false;
}

bool CREVectorYToReal::GetValue(int frame, float& valOut) const {
  zeus::CVector3f a;
  x4_a->GetValue(frame, a);
  valOut = a[1];
  return false;
}

bool CREVectorZToReal::GetValue(int frame, float& valOut) const {
  zeus::CVector3f a;
  x4_a->GetValue(frame, a);
  valOut = a[2];
  return false;
}

bool CREExternalVar::GetValue(int frame, float& valOut) const {
  int a;
  x4_a->GetValue(frame, a);
  int cv = std::max(0, a);
  valOut = CParticleGlobals::instance()->m_currentParticleSystem->x4_system->GetExternalVar(cv & 0xf);
  return false;
}

bool CREIntTimesReal::GetValue(int frame, float& valOut) const {
  int a;
  x4_a->GetValue(frame, a);
  float b;
  x8_b->GetValue(frame, b);
  valOut = float(a) * b;
  return false;
}

bool CREConstantRange::GetValue(int frame, float& valOut) const {
  float val, min, max;
  x4_val->GetValue(frame, val);
  x8_min->GetValue(frame, min);
  xc_max->GetValue(frame, max);

  if (val > min && val < max)
    x10_inRange->GetValue(frame, valOut);
  else
    x14_outOfRange->GetValue(frame, valOut);

  return false;
}

bool CREGetComponentRed::GetValue(int frame, float& valOut) const {
  zeus::CColor a = zeus::skBlack;
  x4_a->GetValue(frame, a);
  valOut = a.r();
  return false;
}

bool CREGetComponentGreen::GetValue(int frame, float& valOut) const {
  zeus::CColor a = zeus::skBlack;
  x4_a->GetValue(frame, a);
  valOut = a.g();
  return false;
}

bool CREGetComponentBlue::GetValue(int frame, float& valOut) const {
  zeus::CColor a = zeus::skBlack;
  x4_a->GetValue(frame, a);
  valOut = a.b();
  return false;
}

bool CREGetComponentAlpha::GetValue(int frame, float& valOut) const {
  zeus::CColor a = zeus::skBlack;
  x4_a->GetValue(frame, a);
  valOut = a.a();
  return false;
}

} // namespace metaforce
