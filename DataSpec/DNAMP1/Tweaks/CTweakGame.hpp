#ifndef _DNAMP1_CTWEAKGAME_HPP_
#define _DNAMP1_CTWEAKGAME_HPP_

#include "../../DNACommon/Tweaks/ITweakGame.hpp"

namespace DataSpec
{
namespace DNAMP1
{

struct CTweakGame : ITweakGame
{
    DECL_YAML
    String<-1> x4_worldPrefix;
    String<-1> x14_ruinsArea; // ????
    Value<float> x24_fov;
    Value<bool> x28_unknown1;
    Value<bool> x29_unknown2;
    Value<bool> x2a_unknown3;
    Value<bool> x2b_splashScreensDisabled;
    Value<float> x2c_unknown5;
    Value<float> x30_pressStartDelay;
    Value<float> x34_rippleSideLenNormal;
    Value<float> x38_rippleSideLenPoison;
    Value<float> x3c_rippleSideLenLava;
    Value<float> x40_unknown10;
    Value<float> x44_unknown11;
    Value<float> x48_unknown12;
    Value<float> x4c_fluidEnvBumpScale;
    Value<float> x50_unknown14;
    Value<float> x54_unknown15;
    Value<float> x58_unknown16;
    Value<float> x5c_unknown17;
    Value<float> x60_hardmodeDamageMult;
    Value<float> x64_hardmodeWeaponMult;

    const std::string& GetWorldPrefix() const { return x4_worldPrefix; }
    bool GetSplashScreensDisabled() const { return x2b_splashScreensDisabled; }
    float GetFirstPersonFOV() const { return x24_fov; }
    float GetPressStartDelay() const { return x30_pressStartDelay; }
    float GetRippleSideLengthNormal() const { return x34_rippleSideLenNormal; }
    float GetRippleSideLengthPoison() const { return x38_rippleSideLenPoison; }
    float GetRippleSideLengthLava() const { return x3c_rippleSideLenLava; }
    float GetFluidEnvBumpScale() const { return x4c_fluidEnvBumpScale; }
    float GetHardModeDamageMultiplier() const { return x60_hardmodeDamageMult; }
    float GetHardModeWeaponMultiplier() const { return x64_hardmodeWeaponMult; }
    CTweakGame() = default;
    CTweakGame(athena::io::IStreamReader& in)
    {
        this->read(in);
        x2b_splashScreensDisabled = false;
    }
};
}
}

#endif
