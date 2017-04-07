#ifndef __DNACOMMON_ITWEAKGUICOLORS_HPP__
#define __DNACOMMON_ITWEAKGUICOLORS_HPP__

#include "../DNACommon.hpp"

namespace DataSpec
{
struct ITweakGuiColors : BigYAML
{
    struct VisorEnergyInitColors
    {
        zeus::CColor tankFilled;
        zeus::CColor tankEmpty;
        zeus::CColor digitsFont;
        zeus::CColor digitsOutline;
    };

    struct VisorEnergyBarColors
    {
        zeus::CColor filled;
        zeus::CColor empty;
        zeus::CColor shadow;
    };

    virtual zeus::CColor GetRadarStuffColor() const=0;
    virtual zeus::CColor GetRadarPlayerPaintColor() const=0;
    virtual zeus::CColor GetRadarEnemyPaintColor() const=0;
    virtual zeus::CColor GetHudMessageFill() const=0;
    virtual zeus::CColor GetHudMessageOutline() const=0;
    virtual zeus::CColor GetHudFrameColor() const=0;
    virtual zeus::CColor GetMissileIconColorActive() const=0;
    virtual zeus::CColor GetEnergyBarFilledLowEnergy() const=0;
    virtual zeus::CColor GetEnergyBarShadowLowEnergy() const=0;
    virtual zeus::CColor GetEnergyBarEmptyLowEnergy() const=0;
    virtual zeus::CColor GetEnergyWarningFont() const=0;
    virtual zeus::CColor GetThreatWarningFont() const=0;
    virtual zeus::CColor GetMissileWarningFont() const=0;
    virtual zeus::CColor GetThreatBarFilled() const=0;
    virtual zeus::CColor GetThreatBarShadow() const=0;
    virtual zeus::CColor GetThreatBarEmpty() const=0;
    virtual zeus::CColor GetMissileBarFilled() const=0;
    virtual zeus::CColor GetMissileBarShadow() const=0;
    virtual zeus::CColor GetMissileBarEmpty() const=0;
    virtual zeus::CColor GetThreatIconColor() const=0;
    virtual zeus::CColor GetTickDecoColor() const=0;
    virtual zeus::CColor GetHelmetLightColor() const=0;
    virtual zeus::CColor GetThreatIconSafeColor() const=0;
    virtual zeus::CColor GetMissileIconColorInactive() const=0;
    virtual zeus::CColor GetMissileIconColorChargedCanAlt() const=0;
    virtual zeus::CColor GetMissileIconColorChargedNoAlt() const=0;
    virtual zeus::CColor GetMissileIconColorDepleteAlt() const=0;
    virtual zeus::CColor GetEnergyWarningOutline() const=0;
    virtual zeus::CColor GetThreatWarningOutline() const=0;
    virtual zeus::CColor GetMissileWarningOutline() const=0;
    virtual zeus::CColor GetEnergyBarFlashColor() const=0;
    virtual zeus::CColor GetXRayEnergyDecoColor() const=0;
    virtual zeus::CColor GetPowerBombDigitAvailableFont() const=0;
    virtual zeus::CColor GetPowerBombDigitAvailableOutline() const=0;
    virtual zeus::CColor GetBallBombFilledColor() const=0;
    virtual zeus::CColor GetBallBombEmptyColor() const=0;
    virtual zeus::CColor GetPowerBombIconAvailableColor() const=0;
    virtual zeus::CColor GetBallBombEnergyColor() const=0;
    virtual zeus::CColor GetBallBombDecoColor() const=0;
    virtual zeus::CColor GetPowerBombDigitDelpetedFont() const=0;
    virtual zeus::CColor GetPowerBombDigitDelpetedOutline() const=0;
    virtual zeus::CColor GetPowerBombIconDepletedColor() const=0;
    virtual zeus::CColor GetThreatIconWarningColor() const=0;
    virtual zeus::CColor GetHudCounterFill() const=0;
    virtual zeus::CColor GetHudCounterOutline() const=0;
    virtual zeus::CColor GetThreatDigitsFont() const=0;
    virtual zeus::CColor GetThreatDigitsOutline() const=0;
    virtual zeus::CColor GetMissileDigitsFont() const=0;
    virtual zeus::CColor GetMissileDigitsOutline() const=0;
    virtual zeus::CColor GetThermalDecoColor() const=0;
    virtual zeus::CColor GetThermalOutlinesColor() const=0;
    virtual zeus::CColor GetThermalLockColor() const=0;
    virtual VisorEnergyInitColors GetVisorEnergyInitColors(int idx) const=0;
    virtual VisorEnergyBarColors GetVisorEnergyBarColors(int idx) const=0;
};
}

#endif // __DNACOMMON_ITWEAKGUICOLORS_HPP__
