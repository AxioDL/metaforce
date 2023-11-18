#pragma once

#include "Runtime/Tweaks/ITweak.hpp"

namespace metaforce::Tweaks {
struct ITweakGuiColors : ITweak {
  struct SVisorEnergyInitColors {
    const zeus::CColor& tankFilled;
    const zeus::CColor& tankEmpty;
    const zeus::CColor& digitsFont;
    const zeus::CColor& digitsOutline;
  };

  struct SVisorEnergyBarColors {
    const zeus::CColor& filled;
    const zeus::CColor& empty;
    const zeus::CColor& shadow;
  };

  virtual const zeus::CColor& GetPauseBlurFilterColor() const = 0;
  virtual const zeus::CColor& GetRadarStuffColor() const = 0;
  virtual const zeus::CColor& GetRadarPlayerPaintColor() const = 0;
  virtual const zeus::CColor& GetRadarEnemyPaintColor() const = 0;
  virtual const zeus::CColor& GetHudMessageFill() const = 0;
  virtual const zeus::CColor& GetHudMessageOutline() const = 0;
  virtual const zeus::CColor& GetHudFrameColor() const = 0;
  virtual const zeus::CColor& GetMissileIconColorActive() const = 0;
  virtual const zeus::CColor& GetVisorBeamMenuItemActive() const = 0;
  virtual const zeus::CColor& GetVisorBeamMenuItemInactive() const = 0;
  virtual const zeus::CColor& GetEnergyBarFilledLowEnergy() const = 0;
  virtual const zeus::CColor& GetEnergyBarShadowLowEnergy() const = 0;
  virtual const zeus::CColor& GetEnergyBarEmptyLowEnergy() const = 0;
  virtual const zeus::CColor& GetHudDamageLightColor() const = 0;
  virtual const zeus::CColor& GetVisorMenuTextFont() const = 0;
  virtual const zeus::CColor& GetVisorMenuTextOutline() const = 0;
  virtual const zeus::CColor& GetBeamMenuTextFont() const = 0;
  virtual const zeus::CColor& GetBeamMenuTextOutline() const = 0;
  virtual const zeus::CColor& GetEnergyWarningFont() const = 0;
  virtual const zeus::CColor& GetThreatWarningFont() const = 0;
  virtual const zeus::CColor& GetMissileWarningFont() const = 0;
  virtual const zeus::CColor& GetThreatBarFilled() const = 0;
  virtual const zeus::CColor& GetThreatBarShadow() const = 0;
  virtual const zeus::CColor& GetThreatBarEmpty() const = 0;
  virtual const zeus::CColor& GetMissileBarFilled() const = 0;
  virtual const zeus::CColor& GetMissileBarShadow() const = 0;
  virtual const zeus::CColor& GetMissileBarEmpty() const = 0;
  virtual const zeus::CColor& GetThreatIconColor() const = 0;
  virtual const zeus::CColor& GetTickDecoColor() const = 0;
  virtual const zeus::CColor& GetHelmetLightColor() const = 0;
  virtual const zeus::CColor& GetThreatIconSafeColor() const = 0;
  virtual const zeus::CColor& GetMissileIconColorInactive() const = 0;
  virtual const zeus::CColor& GetMissileIconColorChargedCanAlt() const = 0;
  virtual const zeus::CColor& GetMissileIconColorChargedNoAlt() const = 0;
  virtual const zeus::CColor& GetMissileIconColorDepleteAlt() const = 0;
  virtual const zeus::CColor& GetVisorBeamMenuLozColor() const = 0;
  virtual const zeus::CColor& GetEnergyWarningOutline() const = 0;
  virtual const zeus::CColor& GetThreatWarningOutline() const = 0;
  virtual const zeus::CColor& GetMissileWarningOutline() const = 0;
  virtual const zeus::CColor& GetDamageAmbientColor() const = 0;
  virtual const zeus::CColor& GetScanFrameInactiveColor() const = 0;
  virtual const zeus::CColor& GetScanFrameActiveColor() const = 0;
  virtual const zeus::CColor& GetScanFrameImpulseColor() const = 0;
  virtual const zeus::CColor& GetScanVisorHudLightMultiply() const = 0;
  virtual const zeus::CColor& GetScanVisorScreenDimColor() const = 0;
  virtual const zeus::CColor& GetThermalVisorHudLightMultiply() const = 0;
  virtual const zeus::CColor& GetEnergyDrainFilterColor() const = 0;
  virtual const zeus::CColor& GetDamageAmbientPulseColor() const = 0;
  virtual const zeus::CColor& GetEnergyBarFlashColor() const = 0;
  virtual const zeus::CColor& GetXRayEnergyDecoColor() const = 0;
  virtual const zeus::CColor& GetScanDataDotColor() const = 0;
  virtual const zeus::CColor& GetPowerBombDigitAvailableFont() const = 0;
  virtual const zeus::CColor& GetPowerBombDigitAvailableOutline() const = 0;
  virtual const zeus::CColor& GetBallBombFilledColor() const = 0;
  virtual const zeus::CColor& GetBallBombEmptyColor() const = 0;
  virtual const zeus::CColor& GetPowerBombIconAvailableColor() const = 0;
  virtual const zeus::CColor& GetBallBombEnergyColor() const = 0;
  virtual const zeus::CColor& GetBallBombDecoColor() const = 0;
  virtual const zeus::CColor& GetPowerBombDigitDelpetedFont() const = 0;
  virtual const zeus::CColor& GetPowerBombDigitDelpetedOutline() const = 0;
  virtual const zeus::CColor& GetPowerBombIconDepletedColor() const = 0;
  virtual const zeus::CColor& GetScanDisplayImagePaneColor() const = 0;
  virtual const zeus::CColor& GetThreatIconWarningColor() const = 0;
  virtual const zeus::CColor& GetHudCounterFill() const = 0;
  virtual const zeus::CColor& GetHudCounterOutline() const = 0;
  virtual const zeus::CColor& GetScanIconCriticalColor() const = 0;
  virtual const zeus::CColor& GetScanIconCriticalDimColor() const = 0;
  virtual const zeus::CColor& GetScanIconNoncriticalColor() const = 0;
  virtual const zeus::CColor& GetScanIconNoncriticalDimColor() const = 0;
  virtual const zeus::CColor& GetScanReticuleColor() const = 0;
  virtual const zeus::CColor& GetThreatDigitsFont() const = 0;
  virtual const zeus::CColor& GetThreatDigitsOutline() const = 0;
  virtual const zeus::CColor& GetMissileDigitsFont() const = 0;
  virtual const zeus::CColor& GetMissileDigitsOutline() const = 0;
  virtual const zeus::CColor& GetThermalDecoColor() const = 0;
  virtual const zeus::CColor& GetThermalOutlinesColor() const = 0;
  virtual const zeus::CColor& GetThermalLockColor() const = 0;
  virtual const zeus::CColor& GetPauseItemAmberColor() const = 0;
  virtual const zeus::CColor& GetPauseItemBlueColor() const = 0;
  virtual SVisorEnergyInitColors GetVisorEnergyInitColors(int idx) const = 0;
  virtual SVisorEnergyBarColors GetVisorEnergyBarColors(int idx) const = 0;
};
} // namespace DataSpec
