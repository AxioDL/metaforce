#include "Runtime/MP1/Tweaks/CTweakGame.hpp"
#include "Runtime/Camera/CCameraManager.hpp"

#include "Runtime/Streams/CInputStream.hpp"

#include "ConsoleVariables/CVar.hpp"
#include "ConsoleVariables/CVarManager.hpp"

#define DEFINE_CVAR_GLOBAL(name)                                                                                       \
  constexpr std::string_view sk##name = std::string_view("tweak.game." #name);                                         \
  CVar* tw_##name = nullptr;

#define CREATE_CVAR(name, help, value, flags)                                                                          \
  tw_##name = mgr->findOrMakeCVar(sk##name, help, value, flags);                                                       \
  if (tw_##name->wasDeserialized()) {                                                                                  \
    tw_##name->toValue(value);                                                                                         \
  }                                                                                                                    \
  tw_##name->addListener([this](CVar* cv) { _tweakListener(cv); });

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

namespace metaforce::MP1 {

CTweakGame::CTweakGame(CInputStream& in) {
  x4_worldPrefix = in.Get<std::string>();
  x14_defaultRoom = in.Get<std::string>();
  x24_fov = in.ReadFloat();
  x28_unknown1 = in.ReadBool();
  x29_unknown2 = in.ReadBool();
  x2a_unknown3 = in.ReadBool();
  x2b_splashScreensDisabled = in.ReadBool();
  x2c_unknown5 = in.ReadFloat();
  x30_pressStartDelay = in.ReadFloat();
  x34_wavecapIntensityNormal = in.ReadFloat();
  x38_wavecapIntensityPoison = in.ReadFloat();
  x3c_wavecapIntensityLava = in.ReadFloat();
  x40_rippleIntensityNormal = in.ReadFloat();
  x44_rippleIntensityPoison = in.ReadFloat();
  x48_rippleIntensityLava = in.ReadFloat();
  x4c_fluidEnvBumpScale = in.ReadFloat();
  x50_waterFogDistanceBase = in.ReadFloat();
  x54_waterFogDistanceRange = in.ReadFloat();
  x58_gravityWaterFogDistanceBase = in.ReadFloat();
  x5c_gravityWaterFogDistanceRange = in.ReadFloat();
  x60_hardmodeDamageMult = in.ReadFloat();
  x64_hardmodeWeaponMult = in.ReadFloat();
#ifdef NDEBUG
  x2b_splashScreensDisabled = false;
#endif
}

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

void CTweakGame::_tweakListener(CVar* cv) {
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

void CTweakGame::initCVars(CVarManager* mgr) {
  constexpr CVar::EFlags skDefaultFlags = CVar::EFlags::Game | CVar::EFlags::Archive;
  CREATE_CVAR(WorldPrefix, "", x4_worldPrefix, skDefaultFlags);
  CREATE_CVAR(FieldOfView, "", x24_fov, skDefaultFlags);
  CREATE_CVAR(SplashScreensDisabled, "", x2b_splashScreensDisabled, skDefaultFlags);
  CREATE_CVAR(PressStartDelay, "", x30_pressStartDelay, skDefaultFlags);
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
} // namespace metaforce::MP1
