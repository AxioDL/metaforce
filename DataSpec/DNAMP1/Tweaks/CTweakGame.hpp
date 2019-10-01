#pragma once

#include "DataSpec/DNACommon/Tweaks/ITweakGame.hpp"

namespace hecl {
class CVar;
}

namespace DataSpec::DNAMP1 {

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
  Value<float> x44_rippleIntentityPoison;
  Value<float> x48_rippleIntensityLava;
  Value<float> x4c_fluidEnvBumpScale;
  Value<float> x50_waterFogDistanceBase;
  Value<float> x54_waterFogDistanceRange;
  Value<float> x58_gravityWaterFogDistanceBase;
  Value<float> x5c_gravityWaterFogDistanceRange;
  Value<float> x60_hardmodeDamageMult;
  Value<float> x64_hardmodeWeaponMult;

  std::string_view GetWorldPrefix() const override { return x4_worldPrefix; }
  std::string_view GetDefaultRoom() const { return x14_defaultRoom; }
  bool GetSplashScreensDisabled() const override { return x2b_splashScreensDisabled; }
  float GetFirstPersonFOV() const override { return x24_fov; }
  float GetPressStartDelay() const override { return x30_pressStartDelay; }
  float GetWavecapIntensityNormal() const override { return x34_wavecapIntensityNormal; }
  float GetWavecapIntensityPoison() const override { return x38_wavecapIntensityPoison; }
  float GetWavecapIntensityLava() const override { return x3c_wavecapIntensityLava; }
  float GetRippleIntensityNormal() const override { return x40_rippleIntensityNormal; }
  float GetRippleIntensityPoison() const override { return x44_rippleIntentityPoison; }
  float GetRippleIntensityLava() const override { return x48_rippleIntensityLava; }
  float GetFluidEnvBumpScale() const override { return x4c_fluidEnvBumpScale; }
  float GetWaterFogDistanceBase() const override { return x50_waterFogDistanceBase; }
  float GetWaterFogDistanceRange() const override { return x54_waterFogDistanceRange; }
  float GetGravityWaterFogDistanceBase() const override { return x58_gravityWaterFogDistanceBase; }
  float GetGravityWaterFogDistanceRange() const override { return x5c_gravityWaterFogDistanceRange; }
  float GetHardModeDamageMultiplier() const override { return x60_hardmodeDamageMult; }
  float GetHardModeWeaponMultiplier() const override { return x64_hardmodeWeaponMult; }
  CTweakGame() = default;
  CTweakGame(athena::io::IStreamReader& in) {
    this->read(in);
#ifdef NDEBUG
    x2b_splashScreensDisabled = false;
#endif
  }

  void initCVars(hecl::CVarManager* mgr) override;
private:
  void _tweakGameListener(hecl::CVar* cv);
};
} // namespace DataSpec::DNAMP1
