#pragma once

#include <array>
#include <vector>

#include "Runtime/RetroTypes.hpp"

#include <zeus/CColor.hpp>
#include <zeus/CMatrix4f.hpp>
#include <zeus/CVector3f.hpp>
#include <zeus/CVector4f.hpp>

namespace urde {
class CElementGen;
class CParticleGlobals {
  CParticleGlobals()=default;
  static std::unique_ptr<CParticleGlobals> g_ParticleGlobals;
public:
  int m_EmitterTime = 0;
  float m_EmitterTimeReal = 0.f;
  void SetEmitterTime(int frame) {
    m_EmitterTime = frame;
    m_EmitterTimeReal = frame;
  }

  int m_ParticleLifetime = 0;
  float m_ParticleLifetimeReal = 0.f;
  void SetParticleLifetime(int frame) {
    m_ParticleLifetime = frame;
    m_ParticleLifetimeReal = frame;
  }

   int m_ParticleLifetimePercentage = 0;
   float m_ParticleLifetimePercentageReal = 0.f;
   float m_ParticleLifetimePercentageRemainder = 0.f;
   void UpdateParticleLifetimeTweenValues(int frame) {
    float lt = m_ParticleLifetime != 0.0f ? m_ParticleLifetime : 1.0f;
    m_ParticleLifetimePercentageReal = 100.0f * frame / lt;
    m_ParticleLifetimePercentage = int(m_ParticleLifetimePercentageReal);
    m_ParticleLifetimePercentageRemainder = m_ParticleLifetimePercentageReal - m_ParticleLifetimePercentage;
    m_ParticleLifetimePercentage = zeus::clamp(0, m_ParticleLifetimePercentage, 100);
  }

  const std::array<float, 8>* m_particleAccessParameters = nullptr;

  struct SParticleSystem {
    FourCC x0_type;
    CElementGen* x4_system;
  };

  SParticleSystem* m_currentParticleSystem;

  static CParticleGlobals* instance() {
    if (!g_ParticleGlobals)
      g_ParticleGlobals.reset(new CParticleGlobals());

    return g_ParticleGlobals.get();
  }
};

struct SParticleInstanceTex {
  zeus::CVector4f pos[4];
  zeus::CColor color;
  zeus::CVector2f uvs[4];
};
extern std::vector<SParticleInstanceTex> g_instTexData;

struct SParticleInstanceIndTex {
  zeus::CVector4f pos[4];
  zeus::CColor color;
  zeus::CVector4f texrTindUVs[4];
  zeus::CVector4f sceneUVs;
};
extern std::vector<SParticleInstanceIndTex> g_instIndTexData;

struct SParticleInstanceNoTex {
  zeus::CVector4f pos[4];
  zeus::CColor color;
};
extern std::vector<SParticleInstanceNoTex> g_instNoTexData;

struct SParticleUniforms {
  zeus::CMatrix4f mvp;
  zeus::CColor moduColor;
};

} // namespace urde
