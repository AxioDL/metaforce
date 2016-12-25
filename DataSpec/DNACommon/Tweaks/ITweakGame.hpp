#ifndef __DNACOMMON_ITWEAKGAME_HPP__
#define __DNACOMMON_ITWEAKGAME_HPP__

#include "../DNACommon.hpp"

namespace DataSpec
{

struct ITweakGame : BigYAML
{
    virtual const std::string& GetWorldPrefix() const = 0;
    virtual bool GetSplashScreensDisabled() const = 0;
    virtual float GetFirstPersonFOV() const = 0;
    virtual float GetPressStartDelay() const = 0;
    virtual float GetHardModeDamageMultiplier() const = 0;
    virtual float GetHardModeWeaponMultiplier() const = 0;
};
}

#endif
