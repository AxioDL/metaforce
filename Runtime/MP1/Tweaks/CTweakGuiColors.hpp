#pragma once

#include "Runtime/Tweaks/ITweakGuiColors.hpp"
#include "Runtime/rstl.hpp"

namespace metaforce::MP1 {
struct CTweakGuiColors final : public Tweaks::ITweakGuiColors {
  zeus::CColor x4_pauseBlurFilterColor;
  zeus::CColor x8_radarStuffColor;
  zeus::CColor xc_radarPlayerPaintColor;
  zeus::CColor x10_radarEnemyPaintColor;
  zeus::CColor x14_hudMessageFill;
  zeus::CColor x18_hudMessageOutline;
  zeus::CColor x1c_hudFrameColor;
  zeus::CColor x20_;
  zeus::CColor x24_;
  zeus::CColor x28_missileIconColorActive;
  zeus::CColor x2c_visorBeamMenuItemActive;
  zeus::CColor x30_visorBeamMenuColorInactive;
  zeus::CColor x34_energyBarFilledLowEnergy;
  zeus::CColor x38_energyBarShadowLowEnergy;
  zeus::CColor x3c_energyBarEmptyLowEnergy;
  zeus::CColor x40_hudDamageLightColor;
  zeus::CColor x44_;
  zeus::CColor x48_;
  zeus::CColor x4c_visorMenuTextFont;
  zeus::CColor x50_visorMenuTextOutline;
  zeus::CColor x54_beamMenuTextFont;
  zeus::CColor x58_beamMenuTextOutline;
  zeus::CColor x5c_energyWarningFont;
  zeus::CColor x60_threatWarningFont;
  zeus::CColor x64_missileWarningFont;
  zeus::CColor x68_threatBarFilled;
  zeus::CColor x6c_threatBarShadow;
  zeus::CColor x70_threatBarEmpty;
  zeus::CColor x74_missileBarFilled;
  zeus::CColor x78_missileBarShadow;
  zeus::CColor x7c_missileBarEmpty;
  zeus::CColor x80_threatIconColor;
  zeus::CColor x84_;
  zeus::CColor x88_tickDecoColor;
  zeus::CColor x8c_helmetLightColor;
  zeus::CColor x90_threatIconSafeColor;
  zeus::CColor x94_missileIconColorInactive;
  zeus::CColor x98_missileIconColorChargedCanAlt;
  zeus::CColor x9c_missileIconColorChargedNoAlt;
  zeus::CColor xa0_missileIconColorDepleteAlt;
  zeus::CColor xa4_;
  zeus::CColor xa8_;
  zeus::CColor xac_;
  zeus::CColor xb0_visorBeamMenuLozColor;
  zeus::CColor xb4_energyWarningOutline;
  zeus::CColor xb8_threatWarningOutline;
  zeus::CColor xbc_missileWarningOutline;
  zeus::CColor xc0_;
  zeus::CColor xc4_damageAmbientColor;
  zeus::CColor xc8_scanFrameInactiveColor;
  zeus::CColor xcc_scanFrameActiveColor;
  zeus::CColor xd0_scanFrameImpulseColor;
  zeus::CColor xd4_scanVisorHudLightMultiply;
  zeus::CColor xd8_scanVisorScreenDimColor;
  zeus::CColor xdc_thermalVisorHudLightMultiply;
  zeus::CColor xe0_energyDrainFilterColor;
  zeus::CColor xe4_damageAmbientPulseColor;
  zeus::CColor xe8_energyBarFlashColor;
  zeus::CColor xec_;
  zeus::CColor xf0_;
  zeus::CColor xf4_;
  zeus::CColor xf8_;
  zeus::CColor xfc_;
  zeus::CColor x100_xrayEnergyDecoColor;
  zeus::CColor x104_;
  zeus::CColor x108_;
  zeus::CColor x10c_;
  zeus::CColor x110_;
  zeus::CColor x114_;
  zeus::CColor x118_;
  zeus::CColor x11c_;
  zeus::CColor x120_;
  zeus::CColor x124_;
  zeus::CColor x128_;
  zeus::CColor x12c_;
  zeus::CColor x130_;
  zeus::CColor x134_;
  zeus::CColor x138_scanDataDotColor;
  zeus::CColor x13c_powerBombDigitAvailableFont;
  zeus::CColor x140_powerBombDigitAvailableOutline;
  zeus::CColor x144_;
  zeus::CColor x148_ballBombFilled;
  zeus::CColor x14c_ballBombEmpty;
  zeus::CColor x150_powerBombIconAvailable;
  zeus::CColor x154_;
  zeus::CColor x158_ballEnergyDeco;
  zeus::CColor x15c_ballBombDeco;
  zeus::CColor x160_powerBombDigitDepletedFont;
  zeus::CColor x164_powerBombDigitDepletedOutline;
  zeus::CColor x168_powerBombIconUnavailable;
  zeus::CColor x16c_;
  zeus::CColor x170_;
  zeus::CColor x174_scanDisplayImagePaneColor;
  zeus::CColor x178_;
  zeus::CColor x17c_threatIconWarningColor;
  zeus::CColor x180_hudCounterFill;
  zeus::CColor x184_hudCounterOutline;
  zeus::CColor x188_scanIconCriticalColor;
  zeus::CColor x18c_scanIconCriticalDimColor;
  zeus::CColor x190_scanIconNoncriticalColor;
  zeus::CColor x194_scanIconNoncriticalDimColor;
  zeus::CColor x198_scanReticuleColor;
  zeus::CColor x19c_threatDigitsFont;
  zeus::CColor x1a0_threatDigitsOutline;
  zeus::CColor x1a4_missileDigitsFont;
  zeus::CColor x1a8_missileDigitsOutline;
  zeus::CColor x1ac_thermalDecoColor;
  zeus::CColor x1b0_thermalOutlinesColor;
  zeus::CColor x1b4_;
  zeus::CColor x1b8_thermalLockColor;
  zeus::CColor x1bc_pauseItemAmber;
  zeus::CColor x1c0_pauseItemBlue;
  struct SPerVisorColors {
    zeus::CColor x0_energyBarFilled;
    zeus::CColor x4_energyBarEmpty;
    zeus::CColor x8_energyBarShadow;
    zeus::CColor xc_energyTankFilled;
    zeus::CColor x10_energyTankEmpty;
    zeus::CColor x14_energyDigitsFont;
    zeus::CColor x18_energyDigitsOutline;
    explicit SPerVisorColors() = default;
    explicit SPerVisorColors(CInputStream& in);
  };
  /* Combat, Scan, XRay, Thermal, Ball */
  rstl::reserved_vector<SPerVisorColors, 5> x1c4_perVisorColors{};

  CTweakGuiColors() = default;
  CTweakGuiColors(CInputStream& r);
  const zeus::CColor& GetPauseBlurFilterColor() const override { return x4_pauseBlurFilterColor; }
  const zeus::CColor& GetRadarStuffColor() const override { return x8_radarStuffColor; }
  const zeus::CColor& GetRadarPlayerPaintColor() const override { return xc_radarPlayerPaintColor; }
  const zeus::CColor& GetRadarEnemyPaintColor() const override { return x10_radarEnemyPaintColor; }
  const zeus::CColor& GetHudMessageFill() const override { return x14_hudMessageFill; }
  const zeus::CColor& GetHudMessageOutline() const override { return x18_hudMessageOutline; }
  const zeus::CColor& GetHudFrameColor() const override { return x1c_hudFrameColor; }
  const zeus::CColor& GetMissileIconColorActive() const override { return x28_missileIconColorActive; }
  const zeus::CColor& GetVisorBeamMenuItemActive() const override { return x2c_visorBeamMenuItemActive; }
  const zeus::CColor& GetVisorBeamMenuItemInactive() const override { return x30_visorBeamMenuColorInactive; }
  const zeus::CColor& GetEnergyBarFilledLowEnergy() const override { return x34_energyBarFilledLowEnergy; }
  const zeus::CColor& GetEnergyBarShadowLowEnergy() const override { return x38_energyBarShadowLowEnergy; }
  const zeus::CColor& GetEnergyBarEmptyLowEnergy() const override { return x3c_energyBarEmptyLowEnergy; }
  const zeus::CColor& GetHudDamageLightColor() const override { return x40_hudDamageLightColor; }
  const zeus::CColor& GetVisorMenuTextFont() const override { return x4c_visorMenuTextFont; }
  const zeus::CColor& GetVisorMenuTextOutline() const override { return x50_visorMenuTextOutline; }
  const zeus::CColor& GetBeamMenuTextFont() const override { return x54_beamMenuTextFont; }
  const zeus::CColor& GetBeamMenuTextOutline() const override { return x58_beamMenuTextOutline; }
  const zeus::CColor& GetEnergyWarningFont() const override { return x5c_energyWarningFont; }
  const zeus::CColor& GetThreatWarningFont() const override { return x60_threatWarningFont; }
  const zeus::CColor& GetMissileWarningFont() const override { return x64_missileWarningFont; }
  const zeus::CColor& GetThreatBarFilled() const override { return x68_threatBarFilled; }
  const zeus::CColor& GetThreatBarShadow() const override { return x6c_threatBarShadow; }
  const zeus::CColor& GetThreatBarEmpty() const override { return x70_threatBarEmpty; }
  const zeus::CColor& GetMissileBarFilled() const override { return x74_missileBarFilled; }
  const zeus::CColor& GetMissileBarShadow() const override { return x78_missileBarShadow; }
  const zeus::CColor& GetMissileBarEmpty() const override { return x7c_missileBarEmpty; }
  const zeus::CColor& GetThreatIconColor() const override { return x80_threatIconColor; }
  const zeus::CColor& GetTickDecoColor() const override { return x88_tickDecoColor; }
  const zeus::CColor& GetHelmetLightColor() const override { return x8c_helmetLightColor; }
  const zeus::CColor& GetThreatIconSafeColor() const override { return x90_threatIconSafeColor; }
  const zeus::CColor& GetMissileIconColorInactive() const override { return x94_missileIconColorInactive; }
  const zeus::CColor& GetMissileIconColorChargedCanAlt() const override { return x98_missileIconColorChargedCanAlt; }
  const zeus::CColor& GetMissileIconColorChargedNoAlt() const override { return x9c_missileIconColorChargedNoAlt; }
  const zeus::CColor& GetMissileIconColorDepleteAlt() const override { return xa0_missileIconColorDepleteAlt; }
  const zeus::CColor& GetVisorBeamMenuLozColor() const override { return xb0_visorBeamMenuLozColor; }
  const zeus::CColor& GetEnergyWarningOutline() const override { return xb4_energyWarningOutline; }
  const zeus::CColor& GetThreatWarningOutline() const override { return xb8_threatWarningOutline; }
  const zeus::CColor& GetMissileWarningOutline() const override { return xbc_missileWarningOutline; }
  const zeus::CColor& GetDamageAmbientColor() const override { return xc4_damageAmbientColor; }
  const zeus::CColor& GetScanFrameInactiveColor() const override { return xc8_scanFrameInactiveColor; }
  const zeus::CColor& GetScanFrameActiveColor() const override { return xcc_scanFrameActiveColor; }
  const zeus::CColor& GetScanFrameImpulseColor() const override { return xd0_scanFrameImpulseColor; }
  const zeus::CColor& GetScanVisorHudLightMultiply() const override { return xd4_scanVisorHudLightMultiply; }
  const zeus::CColor& GetScanVisorScreenDimColor() const override { return xd8_scanVisorScreenDimColor; }
  const zeus::CColor& GetThermalVisorHudLightMultiply() const override { return xdc_thermalVisorHudLightMultiply; }
  const zeus::CColor& GetEnergyDrainFilterColor() const override { return xe0_energyDrainFilterColor; }
  const zeus::CColor& GetDamageAmbientPulseColor() const override { return xe4_damageAmbientPulseColor; }
  const zeus::CColor& GetEnergyBarFlashColor() const override { return xe8_energyBarFlashColor; }
  const zeus::CColor& GetXRayEnergyDecoColor() const override { return x100_xrayEnergyDecoColor; }
  const zeus::CColor& GetScanDataDotColor() const override { return x138_scanDataDotColor; }
  const zeus::CColor& GetPowerBombDigitAvailableFont() const override { return x13c_powerBombDigitAvailableFont; }
  const zeus::CColor& GetPowerBombDigitAvailableOutline() const override { return x140_powerBombDigitAvailableOutline; }
  const zeus::CColor& GetBallBombFilledColor() const override { return x148_ballBombFilled; }
  const zeus::CColor& GetBallBombEmptyColor() const override { return x14c_ballBombEmpty; }
  const zeus::CColor& GetPowerBombIconAvailableColor() const override { return x150_powerBombIconAvailable; }
  const zeus::CColor& GetBallBombEnergyColor() const override { return x158_ballEnergyDeco; }
  const zeus::CColor& GetBallBombDecoColor() const override { return x15c_ballBombDeco; }
  const zeus::CColor& GetPowerBombDigitDelpetedFont() const override { return x160_powerBombDigitDepletedFont; }
  const zeus::CColor& GetPowerBombDigitDelpetedOutline() const override { return x164_powerBombDigitDepletedOutline; }
  const zeus::CColor& GetPowerBombIconDepletedColor() const override { return x168_powerBombIconUnavailable; }
  const zeus::CColor& GetScanDisplayImagePaneColor() const override { return x174_scanDisplayImagePaneColor; }
  const zeus::CColor& GetThreatIconWarningColor() const override { return x17c_threatIconWarningColor; }
  const zeus::CColor& GetHudCounterFill() const override { return x180_hudCounterFill; }
  const zeus::CColor& GetHudCounterOutline() const override { return x184_hudCounterOutline; }
  const zeus::CColor& GetScanIconCriticalColor() const override { return x188_scanIconCriticalColor; }
  const zeus::CColor& GetScanIconCriticalDimColor() const override { return x18c_scanIconCriticalDimColor; }
  const zeus::CColor& GetScanIconNoncriticalColor() const override { return x190_scanIconNoncriticalColor; }
  const zeus::CColor& GetScanIconNoncriticalDimColor() const override { return x194_scanIconNoncriticalDimColor; }
  const zeus::CColor& GetScanReticuleColor() const override { return x198_scanReticuleColor; }
  const zeus::CColor& GetThreatDigitsFont() const override { return x19c_threatDigitsFont; }
  const zeus::CColor& GetThreatDigitsOutline() const override { return x1a0_threatDigitsOutline; }
  const zeus::CColor& GetMissileDigitsFont() const override { return x1a4_missileDigitsFont; }
  const zeus::CColor& GetMissileDigitsOutline() const override { return x1a8_missileDigitsOutline; }
  const zeus::CColor& GetThermalDecoColor() const override { return x1ac_thermalDecoColor; }
  const zeus::CColor& GetThermalOutlinesColor() const override { return x1b0_thermalOutlinesColor; }
  const zeus::CColor& GetThermalLockColor() const override { return x1b8_thermalLockColor; }
  const zeus::CColor& GetPauseItemAmberColor() const override { return x1bc_pauseItemAmber; }
  const zeus::CColor& GetPauseItemBlueColor() const override { return x1c0_pauseItemBlue; }

  SVisorEnergyInitColors GetVisorEnergyInitColors(int idx) const override {
    const SPerVisorColors& colors = x1c4_perVisorColors[idx];
    return {colors.xc_energyTankFilled, colors.x10_energyTankEmpty, colors.x14_energyDigitsFont,
            colors.x18_energyDigitsOutline};
  }
  SVisorEnergyBarColors GetVisorEnergyBarColors(int idx) const override {
    const SPerVisorColors& colors = x1c4_perVisorColors[idx];
    return {colors.x0_energyBarFilled, colors.x4_energyBarEmpty, colors.x8_energyBarShadow};
  }
};
} // namespace metaforce::MP1
