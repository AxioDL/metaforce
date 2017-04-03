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

    virtual zeus::CColor GetHudMessageFill() const=0;
    virtual zeus::CColor GetHudMessageOutline() const=0;
    virtual zeus::CColor GetHudFrameColor() const=0;
    virtual zeus::CColor GetEnergyBarFilledLowEnergy() const=0;
    virtual zeus::CColor GetEnergyBarShadowLowEnergy() const=0;
    virtual zeus::CColor GetEnergyBarEmptyLowEnergy() const=0;
    virtual zeus::CColor GetEnergyWarningFont() const=0;
    virtual zeus::CColor GetTickDecoColor() const=0;
    virtual zeus::CColor GetEnergyWarningOutline() const=0;
    virtual zeus::CColor GetEnergyBarFlashColor() const=0;
    virtual zeus::CColor GetXRayEnergyDecoColor() const=0;
    virtual zeus::CColor GetHudCounterFill() const=0;
    virtual zeus::CColor GetHudCounterOutline() const=0;
    virtual zeus::CColor GetThermalDecoColor() const=0;
    virtual zeus::CColor GetThermalOutlinesColor() const=0;
    virtual zeus::CColor GetThermalLockColor() const=0;
    virtual VisorEnergyInitColors GetVisorEnergyInitColors(int idx) const=0;
    virtual VisorEnergyBarColors GetVisorEnergyBarColors(int idx) const=0;
};
}

#endif // __DNACOMMON_ITWEAKGUICOLORS_HPP__
