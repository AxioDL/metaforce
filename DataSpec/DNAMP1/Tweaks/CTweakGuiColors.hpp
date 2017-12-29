#ifndef __DNAMP1_CTWEAKGUICOLORS_HPP__
#define __DNAMP1_CTWEAKGUICOLORS_HPP__

#include "../../DNACommon/Tweaks/ITweakGuiColors.hpp"

namespace DataSpec::DNAMP1
{
struct CTweakGuiColors final : public ITweakGuiColors
{
    DECL_YAML
    DNAColor x4_pauseBlurFilterColor;
    DNAColor x8_radarStuffColor;
    DNAColor xc_radarPlayerPaintColor;
    DNAColor x10_radarEnemyPaintColor;
    DNAColor x14_hudMessageFill;
    DNAColor x18_hudMessageOutline;
    DNAColor x1c_hudFrameColor;
    DNAColor x20_;
    DNAColor x24_;
    DNAColor x28_missileIconColorActive;
    DNAColor x2c_visorBeamMenuItemActive;
    DNAColor x30_visorBeamMenuColorInactive;
    DNAColor x34_energyBarFilledLowEnergy;
    DNAColor x38_energyBarShadowLowEnergy;
    DNAColor x3c_energyBarEmptyLowEnergy;
    DNAColor x40_hudDamageLightColor;
    DNAColor x44_;
    DNAColor x48_;
    DNAColor x4c_visorMenuTextFont;
    DNAColor x50_visorMenuTextOutline;
    DNAColor x54_beamMenuTextFont;
    DNAColor x58_beamMenuTextOutline;
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
    DNAColor xb0_visorBeamMenuLozColor;
    DNAColor xb4_energyWarningOutline;
    DNAColor xb8_threatWarningOutline;
    DNAColor xbc_missileWarningOutline;
    DNAColor xc0_;
    DNAColor xc4_damageAmbientColor;
    DNAColor xc8_scanFrameInactiveColor;
    DNAColor xcc_scanFrameActiveColor;
    DNAColor xd0_scanFrameImpulseColor;
    DNAColor xd4_scanVisorHudLightMultiply;
    DNAColor xd8_scanVisorScreenDimColor;
    DNAColor xdc_thermalVisorHudLightMultiply;
    DNAColor xe0_energyDrainFilterColor;
    DNAColor xe4_damageAmbientPulseColor;
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
    DNAColor x138_scanDataDotColor;
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
    DNAColor x174_scanDisplayImagePaneColor;
    DNAColor x178_;
    DNAColor x17c_threatIconWarningColor;
    DNAColor x180_hudCounterFill;
    DNAColor x184_hudCounterOutline;
    DNAColor x188_scanIconCriticalColor;
    DNAColor x18c_scanIconCriticalDimColor;
    DNAColor x190_scanIconNoncriticalColor;
    DNAColor x194_scanIconNoncriticalDimColor;
    DNAColor x198_scanReticuleColor;
    DNAColor x19c_threatDigitsFont;
    DNAColor x1a0_threatDigitsOutline;
    DNAColor x1a4_missileDigitsFont;
    DNAColor x1a8_missileDigitsOutline;
    DNAColor x1ac_thermalDecoColor;
    DNAColor x1b0_thermalOutlinesColor;
    DNAColor x1b4_;
    DNAColor x1b8_thermalLockColor;
    DNAColor x1bc_pauseItemAmber;
    DNAColor x1c0_pauseItemBlue;
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

    const zeus::CColor& GetPauseBlurFilterColor() const { return x4_pauseBlurFilterColor; }
    const zeus::CColor& GetRadarStuffColor() const { return x8_radarStuffColor; }
    const zeus::CColor& GetRadarPlayerPaintColor() const { return xc_radarPlayerPaintColor; }
    const zeus::CColor& GetRadarEnemyPaintColor() const { return x10_radarEnemyPaintColor; }
    const zeus::CColor& GetHudMessageFill() const { return x14_hudMessageFill; }
    const zeus::CColor& GetHudMessageOutline() const { return x18_hudMessageOutline; }
    const zeus::CColor& GetHudFrameColor() const { return x1c_hudFrameColor; }
    const zeus::CColor& GetMissileIconColorActive() const { return x28_missileIconColorActive; }
    const zeus::CColor& GetVisorBeamMenuItemActive() const { return x2c_visorBeamMenuItemActive; }
    const zeus::CColor& GetVisorBeamMenuItemInactive() const { return x30_visorBeamMenuColorInactive; }
    const zeus::CColor& GetEnergyBarFilledLowEnergy() const { return x34_energyBarFilledLowEnergy; }
    const zeus::CColor& GetEnergyBarShadowLowEnergy() const { return x38_energyBarShadowLowEnergy; }
    const zeus::CColor& GetEnergyBarEmptyLowEnergy() const { return x3c_energyBarEmptyLowEnergy; }
    const zeus::CColor& GetHudDamageLightColor() const { return x40_hudDamageLightColor; }
    const zeus::CColor& GetVisorMenuTextFont() const { return x4c_visorMenuTextFont; }
    const zeus::CColor& GetVisorMenuTextOutline() const { return x50_visorMenuTextOutline; }
    const zeus::CColor& GetBeamMenuTextFont() const { return x54_beamMenuTextFont; }
    const zeus::CColor& GetBeamMenuTextOutline() const { return x58_beamMenuTextOutline; }
    const zeus::CColor& GetEnergyWarningFont() const { return x5c_energyWarningFont; }
    const zeus::CColor& GetThreatWarningFont() const { return x60_threatWarningFont; }
    const zeus::CColor& GetMissileWarningFont() const { return x64_missileWarningFont; }
    const zeus::CColor& GetThreatBarFilled() const { return x68_threatBarFilled; }
    const zeus::CColor& GetThreatBarShadow() const { return x6c_threatBarShadow; }
    const zeus::CColor& GetThreatBarEmpty() const { return x70_threatBarEmpty; }
    const zeus::CColor& GetMissileBarFilled() const { return x74_missileBarFilled; }
    const zeus::CColor& GetMissileBarShadow() const { return x78_missileBarShadow; }
    const zeus::CColor& GetMissileBarEmpty() const { return x7c_missileBarEmpty; }
    const zeus::CColor& GetThreatIconColor() const { return x80_threatIconColor; }
    const zeus::CColor& GetTickDecoColor() const { return x88_tickDecoColor; }
    const zeus::CColor& GetHelmetLightColor() const { return x8c_helmetLightColor; }
    const zeus::CColor& GetThreatIconSafeColor() const { return x90_threatIconSafeColor; }
    const zeus::CColor& GetMissileIconColorInactive() const { return x94_missileIconColorInactive; }
    const zeus::CColor& GetMissileIconColorChargedCanAlt() const { return x98_missileIconColorChargedCanAlt; }
    const zeus::CColor& GetMissileIconColorChargedNoAlt() const { return x9c_missileIconColorChargedNoAlt; }
    const zeus::CColor& GetMissileIconColorDepleteAlt() const { return xa0_missileIconColorDepleteAlt; }
    const zeus::CColor& GetVisorBeamMenuLozColor() const { return xb0_visorBeamMenuLozColor; }
    const zeus::CColor& GetEnergyWarningOutline() const { return xb4_energyWarningOutline; }
    const zeus::CColor& GetThreatWarningOutline() const { return xb8_threatWarningOutline; }
    const zeus::CColor& GetMissileWarningOutline() const { return xbc_missileWarningOutline; }
    const zeus::CColor& GetDamageAmbientColor() const { return xc4_damageAmbientColor; }
    const zeus::CColor& GetScanFrameInactiveColor() const { return xc8_scanFrameInactiveColor; }
    const zeus::CColor& GetScanFrameActiveColor() const { return xcc_scanFrameActiveColor; }
    const zeus::CColor& GetScanFrameImpulseColor() const { return xd0_scanFrameImpulseColor; }
    const zeus::CColor& GetScanVisorHudLightMultiply() const { return xd4_scanVisorHudLightMultiply; }
    const zeus::CColor& GetScanVisorScreenDimColor() const { return xd8_scanVisorScreenDimColor; }
    const zeus::CColor& GetThermalVisorHudLightMultiply() const { return xdc_thermalVisorHudLightMultiply; }
    const zeus::CColor& GetEnergyDrainFilterColor() const { return xe0_energyDrainFilterColor; }
    const zeus::CColor& GetDamageAmbientPulseColor() const { return xe4_damageAmbientPulseColor; }
    const zeus::CColor& GetEnergyBarFlashColor() const { return xe8_energyBarFlashColor; }
    const zeus::CColor& GetXRayEnergyDecoColor() const { return x100_xrayEnergyDecoColor; }
    const zeus::CColor& GetScanDataDotColor() const { return x138_scanDataDotColor; }
    const zeus::CColor& GetPowerBombDigitAvailableFont() const { return x13c_powerBombDigitAvailableFont; }
    const zeus::CColor& GetPowerBombDigitAvailableOutline() const { return x140_powerBombDigitAvailableOutline; }
    const zeus::CColor& GetBallBombFilledColor() const { return x148_ballBombFilled; }
    const zeus::CColor& GetBallBombEmptyColor() const { return x14c_ballBombEmpty; }
    const zeus::CColor& GetPowerBombIconAvailableColor() const { return x150_powerBombIconAvailable; }
    const zeus::CColor& GetBallBombEnergyColor() const { return x158_ballEnergyDeco; }
    const zeus::CColor& GetBallBombDecoColor() const { return x15c_ballBombDeco; }
    const zeus::CColor& GetPowerBombDigitDelpetedFont() const { return x160_powerBombDigitDepletedFont; }
    const zeus::CColor& GetPowerBombDigitDelpetedOutline() const { return x164_powerBombDigitDepletedOutline; }
    const zeus::CColor& GetPowerBombIconDepletedColor() const { return x168_powerBombIconUnavailable; }
    const zeus::CColor& GetScanDisplayImagePaneColor() const { return x174_scanDisplayImagePaneColor; }
    const zeus::CColor& GetThreatIconWarningColor() const { return x17c_threatIconWarningColor; }
    const zeus::CColor& GetHudCounterFill() const { return x180_hudCounterFill; }
    const zeus::CColor& GetHudCounterOutline() const { return x184_hudCounterOutline; }
    const zeus::CColor& GetScanIconCriticalColor() const { return x188_scanIconCriticalColor; }
    const zeus::CColor& GetScanIconCriticalDimColor() const { return x18c_scanIconCriticalDimColor; }
    const zeus::CColor& GetScanIconNoncriticalColor() const { return x190_scanIconNoncriticalColor; }
    const zeus::CColor& GetScanIconNoncriticalDimColor() const { return x194_scanIconNoncriticalDimColor; }
    const zeus::CColor& GetScanReticuleColor() const { return x198_scanReticuleColor; }
    const zeus::CColor& GetThreatDigitsFont() const { return x19c_threatDigitsFont; }
    const zeus::CColor& GetThreatDigitsOutline() const { return x1a0_threatDigitsOutline; }
    const zeus::CColor& GetMissileDigitsFont() const { return x1a4_missileDigitsFont; }
    const zeus::CColor& GetMissileDigitsOutline() const { return x1a8_missileDigitsOutline; }
    const zeus::CColor& GetThermalDecoColor() const { return x1ac_thermalDecoColor; }
    const zeus::CColor& GetThermalOutlinesColor() const { return x1b0_thermalOutlinesColor; }
    const zeus::CColor& GetThermalLockColor() const { return x1b8_thermalLockColor; }
    const zeus::CColor& GetPauseItemAmberColor() const { return x1bc_pauseItemAmber; }
    const zeus::CColor& GetPauseItemBlueColor() const { return x1c0_pauseItemBlue; }

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

#endif // __DNAMP1_CTWEAKGUICOLORS_HPP__
