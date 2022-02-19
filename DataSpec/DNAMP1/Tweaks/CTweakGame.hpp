#pragma once

#include "DataSpec/DNACommon/Tweaks/ITweakGame.hpp"

namespace hecl {
class CVar;
} // namespace hecl

namespace DataSpec::DNAMP1 {

#define DEFINE_CVAR_GLOBAL(name) extern hecl::CVar* tw_##name;

DEFINE_CVAR_GLOBAL(WorldPrefix);
DEFINE_CVAR_GLOBAL(FieldOfView);
DEFINE_CVAR_GLOBAL(SplashScreensDisabled);
DEFINE_CVAR_GLOBAL(PressStartDelay);
DEFINE_CVAR_GLOBAL(WavecapIntensityNormal);
DEFINE_CVAR_GLOBAL(WavecapIntensityPoison);
DEFINE_CVAR_GLOBAL(WavecapIntensityLava);
DEFINE_CVAR_GLOBAL(RippleIntensityNormal);
DEFINE_CVAR_GLOBAL(RippleIntensityPoison);
DEFINE_CVAR_GLOBAL(RippleIntensityLava);
DEFINE_CVAR_GLOBAL(FluidEnvBumpScale);
DEFINE_CVAR_GLOBAL(WaterFogDistanceBase);
DEFINE_CVAR_GLOBAL(WaterFogDistanceRange);
DEFINE_CVAR_GLOBAL(GravityWaterFogDistanceBase);
DEFINE_CVAR_GLOBAL(GravityWaterFogDistanceRange);
DEFINE_CVAR_GLOBAL(HardModeDamageMult);
DEFINE_CVAR_GLOBAL(HardModeWeaponMult);

#undef DEFINE_CVAR_GLOBAL

struct CTweakGame final : ITweakGame {
  AT_DECL_DNA_YAML
  String<-1> x4_worldPrefix;
  String<-1> x14_defaultRoom;
  Value<float> x24_fov;
  Value<bool> x28_unknown1;
  Value<bool> x29_unknown2;
  Value<bool> x2a_unknown3;
  Value<bool> x2b_splashScreensDisabled;
  Value<float> x2c_unknown5;
  Value<float> x30_pressStartDelay;
  Value<float> x34_wavecapIntensityNormal;
  Value<float> x38_wavecapIntensityPoison;
  Value<float> x3c_wavecapIntensityLava;
  Value<float> x40_rippleIntensityNormal;
  Value<float> x44_rippleIntensityPoison;
  Value<float> x48_rippleIntensityLava;
  Value<float> x4c_fluidEnvBumpScale;
  Value<float> x50_waterFogDistanceBase;
  Value<float> x54_waterFogDistanceRange;
  Value<float> x58_gravityWaterFogDistanceBase;
  Value<float> x5c_gravityWaterFogDistanceRange;
  Value<float> x60_hardmodeDamageMult;
  Value<float> x64_hardmodeWeaponMult;
};
} // namespace DataSpec::DNAMP1
