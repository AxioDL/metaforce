#pragma once

#include "Runtime/Tweaks/ITweak.hpp"

namespace metaforce::Tweaks {

struct ITweakGame : ITweak {
  virtual std::string_view GetWorldPrefix() const = 0;
  virtual bool GetSplashScreensDisabled() const = 0;
  virtual float GetFirstPersonFOV() const = 0;
  virtual float GetPressStartDelay() const = 0;
  virtual float GetWavecapIntensityNormal() const = 0;
  virtual float GetWavecapIntensityPoison() const = 0;
  virtual float GetWavecapIntensityLava() const = 0;
  virtual float GetRippleIntensityNormal() const = 0;
  virtual float GetRippleIntensityPoison() const = 0;
  virtual float GetRippleIntensityLava() const = 0;
  virtual float GetFluidEnvBumpScale() const = 0;
  virtual float GetWaterFogDistanceBase() const = 0;
  virtual float GetWaterFogDistanceRange() const = 0;
  virtual float GetGravityWaterFogDistanceBase() const = 0;
  virtual float GetGravityWaterFogDistanceRange() const = 0;
  virtual float GetHardModeDamageMultiplier() const = 0;
  virtual float GetHardModeWeaponMultiplier() const = 0;
};
} // namespace metaforce
