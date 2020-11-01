#include "DataSpec/DNAMP1/Tweaks/CTweakGame.hpp"
#include "Runtime/Camera/CCameraManager.hpp"

#include <hecl/CVar.hpp>
#include <hecl/CVarManager.hpp>

#define DEFINE_CVAR_GLOBAL(name)                                                                                       \
  constexpr std::string_view sk##name = std::string_view("tweak.game." #name);                                         \
  hecl::CVar* tw_##name = nullptr;

#define CREATE_CVAR(name, help, value, flags)                                                                          \
  tw_##name = mgr->findOrMakeCVar(sk##name, help, value, flags);                                                       \
  if (tw_##name->wasDeserialized()) {                                                                                  \
    tw_##name->toValue(value);                                                                                         \
  }                                                                                                                    \
  tw_##name->addListener([this](hecl::CVar* cv) { _tweakListener(cv); });

#define CREATE_CVAR_BITFIELD(name, help, value, flags)                                                                 \
  {                                                                                                                    \
    bool tmp = value;                                                                                                  \
    CREATE_CVAR(name, help, tmp, flags)                                                                                \
  }

#define UPDATE_CVAR(name, cv, value)                                                                                   \
  if ((cv) == tw_##name) {                                                                                             \
    (cv)->toValue(value);                                                                                              \
    return;                                                                                                            \
  }

#define UPDATE_CVAR_BITFIELD(name, cv, value)                                                                          \
  {                                                                                                                    \
    bool tmp = value;                                                                                                  \
    UPDATE_CVAR(name, cv, tmp)                                                                                         \
    (value) = tmp;                                                                                                     \
  }

namespace DataSpec::DNAMP1 {
namespace {
constexpr hecl::CVar::EFlags skDefaultFlags = hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Archive;
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
} // anonymous namespace

void CTweakGame::_tweakListener(hecl::CVar* cv) {
  UPDATE_CVAR(WorldPrefix, cv, x4_worldPrefix);
  UPDATE_CVAR(FieldOfView, cv, x24_fov);
  UPDATE_CVAR(SplashScreensDisabled, cv, x2b_splashScreensDisabled);
  UPDATE_CVAR(PressStartDelay, cv, x30_pressStartDelay);
  UPDATE_CVAR(WavecapIntensityNormal, cv, x34_wavecapIntensityNormal);
  UPDATE_CVAR(WavecapIntensityPoison, cv, x38_wavecapIntensityPoison);
  UPDATE_CVAR(WavecapIntensityLava, cv, x3c_wavecapIntensityLava);
  UPDATE_CVAR(RippleIntensityNormal, cv, x40_rippleIntensityNormal);
  UPDATE_CVAR(RippleIntensityPoison, cv, x44_rippleIntensityPoison);
  UPDATE_CVAR(RippleIntensityLava, cv, x48_rippleIntensityLava);
  UPDATE_CVAR(FluidEnvBumpScale, cv, x4c_fluidEnvBumpScale);
  UPDATE_CVAR(WaterFogDistanceBase, cv, x50_waterFogDistanceBase);
  UPDATE_CVAR(WaterFogDistanceRange, cv, x54_waterFogDistanceRange);
  UPDATE_CVAR(GravityWaterFogDistanceBase, cv, x58_gravityWaterFogDistanceBase);
  UPDATE_CVAR(GravityWaterFogDistanceRange, cv, x5c_gravityWaterFogDistanceRange);
  UPDATE_CVAR(HardModeDamageMult, cv, x60_hardmodeDamageMult);
  UPDATE_CVAR(HardModeWeaponMult, cv, x64_hardmodeWeaponMult);
}

void CTweakGame::initCVars(hecl::CVarManager* mgr) {
  CREATE_CVAR(WorldPrefix, "", x4_worldPrefix, skDefaultFlags);
  CREATE_CVAR(FieldOfView, "", x24_fov, skDefaultFlags);
  CREATE_CVAR(SplashScreensDisabled, "", x2b_splashScreensDisabled,
              hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Archive);
  CREATE_CVAR(PressStartDelay, "", x30_pressStartDelay, , skDefaultFlags);
  CREATE_CVAR(WavecapIntensityNormal, "", x34_wavecapIntensityNormal, skDefaultFlags);
  CREATE_CVAR(WavecapIntensityPoison, "", x38_wavecapIntensityPoison, skDefaultFlags);
  CREATE_CVAR(WavecapIntensityLava, "", x3c_wavecapIntensityLava, skDefaultFlags);
  CREATE_CVAR(RippleIntensityNormal, "", x40_rippleIntensityNormal, skDefaultFlags);
  CREATE_CVAR(RippleIntensityPoison, "", x44_rippleIntensityPoison, skDefaultFlags);
  CREATE_CVAR(RippleIntensityLava, "", x48_rippleIntensityLava, skDefaultFlags);
  CREATE_CVAR(FluidEnvBumpScale, "", x4c_fluidEnvBumpScale, skDefaultFlags);
  CREATE_CVAR(WaterFogDistanceBase, "", x50_waterFogDistanceBase, skDefaultFlags);
  CREATE_CVAR(WaterFogDistanceRange, "", x54_waterFogDistanceRange, skDefaultFlags);
  CREATE_CVAR(GravityWaterFogDistanceBase, "", x58_gravityWaterFogDistanceBase, skDefaultFlags);
  CREATE_CVAR(GravityWaterFogDistanceRange, "", x5c_gravityWaterFogDistanceRange, skDefaultFlags);
  CREATE_CVAR(HardModeDamageMult, "", x60_hardmodeDamageMult, skDefaultFlags);
  CREATE_CVAR(HardModeWeaponMult, "", x64_hardmodeWeaponMult, skDefaultFlags);
}
} // namespace DataSpec::DNAMP1
