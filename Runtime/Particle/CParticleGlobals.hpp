#pragma once

#include "zeus/CVector3f.hpp"
#include "zeus/CVector4f.hpp"
#include "zeus/CMatrix4f.hpp"
#include "zeus/CColor.hpp"
#include "RetroTypes.hpp"
#include <array>

namespace urde {
class CElementGen;
class CParticleGlobals {
public:
  static int g_EmitterTime;
  static float g_EmitterTimeReal;
  static void SetEmitterTime(int frame) {
    g_EmitterTime = frame;
    g_EmitterTimeReal = frame;
  }

  static int g_ParticleLifetime;
  static float g_ParticleLifetimeReal;
  static void SetParticleLifetime(int frame) {
    g_ParticleLifetime = frame;
    g_ParticleLifetimeReal = frame;
  }

  static int g_ParticleLifetimePercentage;
  static float g_ParticleLifetimePercentageReal;
  static float g_ParticleLifetimePercentageRemainder;
  static void UpdateParticleLifetimeTweenValues(int frame) {
    float lt = g_ParticleLifetime != 0.0f ? g_ParticleLifetime : 1.0f;
    g_ParticleLifetimePercentageReal = 100.0f * frame / lt;
    g_ParticleLifetimePercentage = int(g_ParticleLifetimePercentageReal);
    g_ParticleLifetimePercentageRemainder = g_ParticleLifetimePercentageReal - g_ParticleLifetimePercentage;
    g_ParticleLifetimePercentage = zeus::clamp(0, g_ParticleLifetimePercentage, 100);
  }

  static const std::array<float, 8>* g_particleAccessParameters;

  struct SParticleSystem {
    FourCC x0_type;
    CElementGen* x4_system;
  };

  static SParticleSystem* g_currentParticleSystem;
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
