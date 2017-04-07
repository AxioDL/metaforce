#ifndef __DNAMP1_CTWEAKGUICOLORS_HPP__
#define __DNAMP1_CTWEAKGUICOLORS_HPP__

#include "../../DNACommon/Tweaks/ITweakGuiColors.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct CTweakGuiColors : public ITweakGuiColors
{
    DECL_YAML
    DNAColor x4_;
    DNAColor x8_radarStuffColor;
    DNAColor xc_radarPlayerPaintColor;
    DNAColor x10_radarEnemyPaintColor;
    DNAColor x14_hudMessageFill;
    DNAColor x18_hudMessageOutline;
    DNAColor x1c_hudFrameColor;
    DNAColor x20_;
    DNAColor x24_;
    DNAColor x28_missileIconColorActive;
    DNAColor x2c_;
    DNAColor x30_;
    DNAColor x34_energyBarFilledLowEnergy;
    DNAColor x38_energyBarShadowLowEnergy;
    DNAColor x3c_energyBarEmptyLowEnergy;
    DNAColor x40_;
    DNAColor x44_;
    DNAColor x48_;
    DNAColor x4c_;
    DNAColor x50_;
    DNAColor x54_;
    DNAColor x58_;
    DNAColor x5c_energyWarningFont;
    DNAColor x60_threatWarningFont;
    DNAColor x64_missileWarningFont;
    DNAColor x68_threatBarFilled;
    DNAColor x6c_threatBarShadow;
    DNAColor x70_threatBarEmpty;
    DNAColor x74_missileBarFilled;
    DNAColor x78_missileBarShadow;
    DNAColor x7c_missileBarEmpty;
    DNAColor x80_threatIconColor;
    DNAColor x84_;
    DNAColor x88_tickDecoColor;
    DNAColor x8c_helmetLightColor;
    DNAColor x90_threatIconSafeColor;
    DNAColor x94_missileIconColorInactive;
    DNAColor x98_missileIconColorChargedCanAlt;
    DNAColor x9c_missileIconColorChargedNoAlt;
    DNAColor xa0_missileIconColorDepleteAlt;
    DNAColor xa4_;
    DNAColor xa8_;
    DNAColor xac_;
    DNAColor xb0_;
    DNAColor xb4_energyWarningOutline;
    DNAColor xb8_threatWarningOutline;
    DNAColor xbc_missileWarningOutline;
    DNAColor xc0_;
    DNAColor xc4_;
    DNAColor xc8_;
    DNAColor xcc_;
    DNAColor xd0_;
    DNAColor xd4_;
    DNAColor xd8_;
    DNAColor xdc_;
    DNAColor xe0_;
    DNAColor xe4_;
    DNAColor xe8_energyBarFlashColor;
    DNAColor xec_;
    DNAColor xf0_;
    DNAColor xf4_;
    DNAColor xf8_;
    DNAColor xfc_;
    DNAColor x100_xrayEnergyDecoColor;
    DNAColor x104_;
    DNAColor x108_;
    DNAColor x10c_;
    DNAColor x110_;
    DNAColor x114_;
    DNAColor x118_;
    DNAColor x11c_;
    DNAColor x120_;
    DNAColor x124_;
    DNAColor x128_;
    DNAColor x12c_;
    DNAColor x130_;
    DNAColor x134_;
    DNAColor x138_;
    DNAColor x13c_powerBombDigitAvailableFont;
    DNAColor x140_powerBombDigitAvailableOutline;
    DNAColor x144_;
    DNAColor x148_ballBombFilled;
    DNAColor x14c_ballBombEmpty;
    DNAColor x150_powerBombIconAvailable;
    DNAColor x154_;
    DNAColor x158_ballEnergyDeco;
    DNAColor x15c_ballBombDeco;
    DNAColor x160_powerBombDigitDepletedFont;
    DNAColor x164_powerBombDigitDepletedOutline;
    DNAColor x168_powerBombIconUnavailable;
    DNAColor x16c_;
    DNAColor x170_;
    DNAColor x174_;
    DNAColor x178_;
    DNAColor x17c_threatIconWarningColor;
    DNAColor x180_hudCounterFill;
    DNAColor x184_hudCounterOutline;
    DNAColor x188_;
    DNAColor x18c_;
    DNAColor x190_;
    DNAColor x194_;
    DNAColor x198_;
    DNAColor x19c_threatDigitsFont;
    DNAColor x1a0_threatDigitsOutline;
    DNAColor x1a4_missileDigitsFont;
    DNAColor x1a8_missileDigitsOutline;
    DNAColor x1ac_thermalDecoColor;
    DNAColor x1b0_thermalOutlinesColor;
    DNAColor x1b4_;
    DNAColor x1b8_thermalLockColor;
    DNAColor x1bc_;
    DNAColor x1c0_;
    struct PerVisorColors : BigYAML
    {
        DECL_YAML
        DNAColor x0_energyBarFilled;
        DNAColor x4_energyBarEmpty;
        DNAColor x8_energyBarShadow;
        DNAColor xc_energyTankFilled;
        DNAColor x10_energyTankEmpty;
        DNAColor x14_energyDigitsFont;
        DNAColor x18_energyDigitsOutline;
    };
    Value<atUint32> x1c4_perVisorCount;
    /* Combat, Scan, XRay, Thermal, Ball */
    Vector<PerVisorColors, DNA_COUNT(x1c4_perVisorCount)> x1c4_perVisorColors;

    CTweakGuiColors() = default;
    CTweakGuiColors(athena::io::IStreamReader& r) { this->read(r); }

    zeus::CColor GetRadarStuffColor() const { return x8_radarStuffColor; }
    zeus::CColor GetRadarPlayerPaintColor() const { return xc_radarPlayerPaintColor; }
    zeus::CColor GetRadarEnemyPaintColor() const { return x10_radarEnemyPaintColor; }
    zeus::CColor GetHudMessageFill() const { return x14_hudMessageFill; }
    zeus::CColor GetHudMessageOutline() const { return x18_hudMessageOutline; }
    zeus::CColor GetHudFrameColor() const { return x1c_hudFrameColor; }
    zeus::CColor GetMissileIconColorActive() const { return x28_missileIconColorActive; }
    zeus::CColor GetEnergyBarFilledLowEnergy() const { return x34_energyBarFilledLowEnergy; }
    zeus::CColor GetEnergyBarShadowLowEnergy() const { return x38_energyBarShadowLowEnergy; }
    zeus::CColor GetEnergyBarEmptyLowEnergy() const { return x3c_energyBarEmptyLowEnergy; }
    zeus::CColor GetEnergyWarningFont() const { return x5c_energyWarningFont; }
    zeus::CColor GetThreatWarningFont() const { return x60_threatWarningFont; }
    zeus::CColor GetMissileWarningFont() const { return x64_missileWarningFont; }
    zeus::CColor GetThreatBarFilled() const { return x68_threatBarFilled; }
    zeus::CColor GetThreatBarShadow() const { return x6c_threatBarShadow; }
    zeus::CColor GetThreatBarEmpty() const { return x70_threatBarEmpty; }
    zeus::CColor GetMissileBarFilled() const { return x74_missileBarFilled; }
    zeus::CColor GetMissileBarShadow() const { return x78_missileBarShadow; }
    zeus::CColor GetMissileBarEmpty() const { return x7c_missileBarEmpty; }
    zeus::CColor GetThreatIconColor() const { return x80_threatIconColor; }
    zeus::CColor GetTickDecoColor() const { return x88_tickDecoColor; }
    zeus::CColor GetHelmetLightColor() const { return x8c_helmetLightColor; }
    zeus::CColor GetThreatIconSafeColor() const { return x90_threatIconSafeColor; }
    zeus::CColor GetMissileIconColorInactive() const { return x94_missileIconColorInactive; }
    zeus::CColor GetMissileIconColorChargedCanAlt() const { return x98_missileIconColorChargedCanAlt; }
    zeus::CColor GetMissileIconColorChargedNoAlt() const { return x9c_missileIconColorChargedNoAlt; }
    zeus::CColor GetMissileIconColorDepleteAlt() const { return xa0_missileIconColorDepleteAlt; }
    zeus::CColor GetEnergyWarningOutline() const { return xb4_energyWarningOutline; }
    zeus::CColor GetThreatWarningOutline() const { return xb8_threatWarningOutline; }
    zeus::CColor GetMissileWarningOutline() const { return xbc_missileWarningOutline; }
    zeus::CColor GetEnergyBarFlashColor() const { return xe8_energyBarFlashColor; }
    zeus::CColor GetXRayEnergyDecoColor() const { return x100_xrayEnergyDecoColor; }
    zeus::CColor GetPowerBombDigitAvailableFont() const { return x13c_powerBombDigitAvailableFont; }
    zeus::CColor GetPowerBombDigitAvailableOutline() const { return x140_powerBombDigitAvailableOutline; }
    zeus::CColor GetBallBombFilledColor() const { return x148_ballBombFilled; }
    zeus::CColor GetBallBombEmptyColor() const { return x14c_ballBombEmpty; }
    zeus::CColor GetPowerBombIconAvailableColor() const { return x150_powerBombIconAvailable; }
    zeus::CColor GetBallBombEnergyColor() const { return x158_ballEnergyDeco; }
    zeus::CColor GetBallBombDecoColor() const { return x15c_ballBombDeco; }
    zeus::CColor GetPowerBombDigitDelpetedFont() const { return x160_powerBombDigitDepletedFont; }
    zeus::CColor GetPowerBombDigitDelpetedOutline() const { return x164_powerBombDigitDepletedOutline; }
    zeus::CColor GetPowerBombIconDepletedColor() const { return x168_powerBombIconUnavailable; }
    zeus::CColor GetThreatIconWarningColor() const { return x17c_threatIconWarningColor; }
    zeus::CColor GetHudCounterFill() const { return x180_hudCounterFill; }
    zeus::CColor GetHudCounterOutline() const { return x184_hudCounterOutline; }
    zeus::CColor GetThreatDigitsFont() const { return x19c_threatDigitsFont; }
    zeus::CColor GetThreatDigitsOutline() const { return x1a0_threatDigitsOutline; }
    zeus::CColor GetMissileDigitsFont() const { return x1a4_missileDigitsFont; }
    zeus::CColor GetMissileDigitsOutline() const { return x1a8_missileDigitsOutline; }
    zeus::CColor GetThermalDecoColor() const { return x1ac_thermalDecoColor; }
    zeus::CColor GetThermalOutlinesColor() const { return x1b0_thermalOutlinesColor; }
    zeus::CColor GetThermalLockColor() const { return x1b8_thermalLockColor; }
    VisorEnergyInitColors GetVisorEnergyInitColors(int idx) const
    {
        const PerVisorColors& colors = x1c4_perVisorColors[idx];
        return {colors.xc_energyTankFilled, colors.x10_energyTankEmpty,
                colors.x14_energyDigitsFont, colors.x18_energyDigitsOutline};
    }
    VisorEnergyBarColors GetVisorEnergyBarColors(int idx) const
    {
        const PerVisorColors& colors = x1c4_perVisorColors[idx];
        return {colors.x0_energyBarFilled, colors.x4_energyBarEmpty, colors.x8_energyBarShadow};
    }
};
}
}

#endif // __DNAMP1_CTWEAKGUICOLORS_HPP__
