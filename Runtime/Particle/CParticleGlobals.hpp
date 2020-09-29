#pragma once

#include <array>
#include <vector>

#include "Runtime/RetroTypes.hpp"

#include "zeus/CColor.hpp"
#include "zeus/CMatrix4f.hpp"
#include "zeus/CVector3f.hpp"
#include "zeus/CVector4f.hpp"

namespace urde {
class CElementGen;
class CParticleGlobals {
  CParticleGlobals() = default;
  static std::unique_ptr<CParticleGlobals> g_ParticleGlobals;

public:
  int m_EmitterTime = 0;
  float m_EmitterTimeReal = 0.f;
  void SetEmitterTime(int frame) {
    m_EmitterTime = frame;
    m_EmitterTimeReal = float(frame);
  }

  int m_ParticleLifetime = 0;
  float m_ParticleLifetimeReal = 0.f;
  void SetParticleLifetime(int frame) {
    m_ParticleLifetime = frame;
    m_ParticleLifetimeReal = float(frame);
  }

   int m_ParticleLifetimePercentage = 0;
   float m_ParticleLifetimePercentageReal = 0.f;
   float m_ParticleLifetimePercentageRemainder = 0.f;
   void UpdateParticleLifetimeTweenValues(int frame) {
    const float lt = m_ParticleLifetime != 0 ? float(m_ParticleLifetime) : 1.0f;
    m_ParticleLifetimePercentageReal = 100.0f * float(frame) / lt;
    m_ParticleLifetimePercentage = int(m_ParticleLifetimePercentageReal);
    m_ParticleLifetimePercentageRemainder = m_ParticleLifetimePercentageReal - float(m_ParticleLifetimePercentage);
    m_ParticleLifetimePercentage = zeus::clamp(0, m_ParticleLifetimePercentage, 100);
  }

  const std::array<float, 8>* m_particleAccessParameters = nullptr;

  struct SParticleSystem {
    FourCC x0_type;
    CElementGen* x4_system;
  };

  SParticleSystem* m_currentParticleSystem = nullptr;

  static CParticleGlobals* instance() {
    if (!g_ParticleGlobals)
      g_ParticleGlobals.reset(new CParticleGlobals());

    return g_ParticleGlobals.get();
  }
};

struct SParticleInstanceTex {
  std::array<hsh::float4, 4> pos;
  hsh::float4 color;
  std::array<hsh::float2, 4> uvs;
};
extern std::vector<SParticleInstanceTex> g_instTexData;

struct SParticleInstanceIndTex {
  std::array<hsh::float4, 4> pos;
  hsh::float4 color;
  std::array<hsh::float4, 4> texrTindUVs;
  hsh::float4 sceneUVs;
};
extern std::vector<SParticleInstanceIndTex> g_instIndTexData;

struct SParticleInstanceNoTex {
  std::array<hsh::float4, 4> pos;
  hsh::float4 color;
};
extern std::vector<SParticleInstanceNoTex> g_instNoTexData;

struct SParticleUniforms {
  hsh::float4x4 mvp;
  hsh::float4 moduColor;
};

} // namespace urde
