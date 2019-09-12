#pragma once

#include "RetroTypes.hpp"
#include "zeus/CColor.hpp"
#include "Character/CActorLights.hpp"

namespace urde {

class CLightParameters {
  friend class CActor;

public:
  enum class EShadowTesselation { Zero };

  enum class EWorldLightingOptions { Zero, NormalWorld, NoShadowCast, DisableWorld };

  enum class ELightRecalculationOptions { LargeFrameCount, EightFrames, FourFrames, OneFrame };

private:
  bool x4_a = false;
  float x8_b = 0.f;
  EShadowTesselation xc_shadowTesselation = EShadowTesselation::Zero;
  float x10_d = 0.f;
  float x14_e = 0.f;
  zeus::CColor x18_noLightsAmbient;
  bool x1c_makeLights = false;
  bool x1d_ambientChannelOverflow = false;
  EWorldLightingOptions x20_worldLightingOptions = EWorldLightingOptions::Zero;
  ELightRecalculationOptions x24_lightRecalcOpts = ELightRecalculationOptions::EightFrames;
  s32 x28_layerIdx = 0;
  zeus::CVector3f x2c_actorPosBias;
  s32 x38_maxDynamicLights = 4;
  s32 x3c_maxAreaLights = 4;

public:
  CLightParameters() = default;
  CLightParameters(bool a, float b, EShadowTesselation shadowTess, float d, float e,
                   const zeus::CColor& noLightsAmbient, bool makeLights, EWorldLightingOptions lightingOpts,
                   ELightRecalculationOptions lightRecalcOpts, const zeus::CVector3f& actorPosBias,
                   s32 maxDynamicLights, s32 maxAreaLights, bool ambChannelOverflow, s32 layerIdx)
  : x4_a(a)
  , x8_b(b)
  , xc_shadowTesselation(shadowTess)
  , x10_d(d)
  , x14_e(e)
  , x18_noLightsAmbient(noLightsAmbient)
  , x1c_makeLights(makeLights)
  , x1d_ambientChannelOverflow(ambChannelOverflow)
  , x20_worldLightingOptions(lightingOpts)
  , x24_lightRecalcOpts(lightRecalcOpts)
  , x28_layerIdx(layerIdx)
  , x2c_actorPosBias(actorPosBias)
  , x38_maxDynamicLights(maxDynamicLights)
  , x3c_maxAreaLights(maxAreaLights) {
    if (x38_maxDynamicLights > 4 || x38_maxDynamicLights == -1)
      x38_maxDynamicLights = 4;
    if (x3c_maxAreaLights > 4 || x3c_maxAreaLights == -1)
      x3c_maxAreaLights = 4;
  }
  static CLightParameters None() { return CLightParameters(); }

  static u32 GetFramesBetweenRecalculation(ELightRecalculationOptions opts) {
    if (opts == ELightRecalculationOptions::LargeFrameCount)
      return 0x3FFFFFFF;
    else if (opts == ELightRecalculationOptions::EightFrames)
      return 8;
    else if (opts == ELightRecalculationOptions::FourFrames)
      return 4;
    else if (opts == ELightRecalculationOptions::OneFrame)
      return 1;
    return 8;
  }

  std::unique_ptr<CActorLights> MakeActorLights() const {
    if (!x1c_makeLights) {
      return nullptr;
    }

    const u32 updateFrames = GetFramesBetweenRecalculation(x24_lightRecalcOpts);
    auto lights = std::make_unique<CActorLights>(updateFrames, x2c_actorPosBias, x38_maxDynamicLights,
                                                 x3c_maxAreaLights, x1d_ambientChannelOverflow, x28_layerIdx == 1,
                                                 x20_worldLightingOptions == EWorldLightingOptions::DisableWorld, 0.1f);
    if (x20_worldLightingOptions == EWorldLightingOptions::NoShadowCast) {
      lights->SetCastShadows(false);
    }
    if (x3c_maxAreaLights == 0) {
      lights->SetAmbientColor(x18_noLightsAmbient);
    }
    return lights;
  }
  const zeus::CColor& GetNoLightsAmbient() const { return x18_noLightsAmbient; }
};

} // namespace urde
