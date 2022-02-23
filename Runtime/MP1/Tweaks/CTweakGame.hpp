#pragma once

#include "Runtime/Tweaks/ITweakGame.hpp"

namespace hecl {
class CVar;
} // namespace hecl

namespace metaforce {
class CInputStream;
namespace MP1 {

#define DEFINE_CVAR_GLOBAL(name) extern CVar* tw_##name;

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

struct CTweakGame final : Tweaks::ITweakGame {
  std::string x4_worldPrefix;
  std::string x14_defaultRoom;
  float x24_fov{};
  bool x28_unknown1{};
  bool x29_unknown2{};
  bool x2a_unknown3{};
  bool x2b_splashScreensDisabled{};
  float x2c_unknown5{};
  float x30_pressStartDelay{};
  float x34_wavecapIntensityNormal{};
  float x38_wavecapIntensityPoison{};
  float x3c_wavecapIntensityLava{};
  float x40_rippleIntensityNormal{};
  float x44_rippleIntensityPoison{};
  float x48_rippleIntensityLava{};
  float x4c_fluidEnvBumpScale{};
  float x50_waterFogDistanceBase{};
  float x54_waterFogDistanceRange{};
  float x58_gravityWaterFogDistanceBase{};
  float x5c_gravityWaterFogDistanceRange{};
  float x60_hardmodeDamageMult{};
  float x64_hardmodeWeaponMult{};

  std::string_view GetWorldPrefix() const override { return x4_worldPrefix; }
  std::string_view GetDefaultRoom() const { return x14_defaultRoom; }
  bool GetSplashScreensDisabled() const override { return x2b_splashScreensDisabled; }
  float GetFirstPersonFOV() const override { return x24_fov; }
  float GetPressStartDelay() const override { return x30_pressStartDelay; }
  float GetWavecapIntensityNormal() const override { return x34_wavecapIntensityNormal; }
  float GetWavecapIntensityPoison() const override { return x38_wavecapIntensityPoison; }
  float GetWavecapIntensityLava() const override { return x3c_wavecapIntensityLava; }
  float GetRippleIntensityNormal() const override { return x40_rippleIntensityNormal; }
  float GetRippleIntensityPoison() const override { return x44_rippleIntensityPoison; }
  float GetRippleIntensityLava() const override { return x48_rippleIntensityLava; }
  float GetFluidEnvBumpScale() const override { return x4c_fluidEnvBumpScale; }
  float GetWaterFogDistanceBase() const override { return x50_waterFogDistanceBase; }
  float GetWaterFogDistanceRange() const override { return x54_waterFogDistanceRange; }
  float GetGravityWaterFogDistanceBase() const override { return x58_gravityWaterFogDistanceBase; }
  float GetGravityWaterFogDistanceRange() const override { return x5c_gravityWaterFogDistanceRange; }
  float GetHardModeDamageMultiplier() const override { return x60_hardmodeDamageMult; }
  float GetHardModeWeaponMultiplier() const override { return x64_hardmodeWeaponMult; }
  CTweakGame() = default;
  CTweakGame(CInputStream& in);

  void initCVars(CVarManager* mgr) override;

private:
  void _tweakListener(CVar* cv);
};
} // namespace MP1
} // namespace metaforce
