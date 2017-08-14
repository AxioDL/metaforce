#ifndef __DNACOMMON_ITWEAKGAME_HPP__
#define __DNACOMMON_ITWEAKGAME_HPP__

#include "ITweak.hpp"

namespace DataSpec
{

struct ITweakGame : ITweak
{
    virtual const std::string& GetWorldPrefix() const = 0;
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
    virtual float GetHardModeDamageMultiplier() const = 0;
    virtual float GetHardModeWeaponMultiplier() const = 0;
};
}

#endif
