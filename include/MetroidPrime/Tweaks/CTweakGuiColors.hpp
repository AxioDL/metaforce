#ifndef _CTWEAKGUICOLORS
#define _CTWEAKGUICOLORS

#include "MetroidPrime/Tweaks/ITweakObject.hpp"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/TOneStatic.hpp"


#include "rstl/reserved_vector.hpp"

class CTweakGuiColors;
class CTweakGuiColors : public ITweakObject, public TOneStatic< CTweakGuiColors > {
public:
  struct SPerVisorColors {
    CColor mEnergyBarFilled;
    CColor mEnergyBarEmpty;
    CColor mEnergyBarShadow;
    CColor mEnergyTankFilled;
    CColor mEnergyTankEmpty;
    CColor mEnergyDigitsFont;
    CColor mEnergyDigitsOutline;

    explicit SPerVisorColors(CInputStream& in)
    : mEnergyBarFilled(in)
    , mEnergyBarEmpty(in)
    , mEnergyBarShadow(in)
    , mEnergyTankFilled(in)
    , mEnergyTankEmpty(in)
    , mEnergyDigitsFont(in)
    , mEnergyDigitsOutline(in) {}
  };
  const SPerVisorColors& GetVisorColors(int visor) const { return mPerVisorColors[visor]; }

  ~CTweakGuiColors() override;
  CTweakGuiColors(CInputStream& in);

  const CColor& GetPauseBlurFilterColor() const { return mPauseBlurFilterColor; }
  const CColor& GetRadarStuffColor() const { return mRadarStuffColor; }
  const CColor& GetRadarPlayerPaintColor() const { return mRadarPlayerPaintColor; }
  const CColor& GetRadarEnemyPaintColor() const { return mRadarEnemyPaintColor; }
  const CColor& GetHudMessageFill() const { return mHudMessageFill; }
  const CColor& GetHudMessageOutline() const { return mHudMessageOutline; }
  const CColor& GetHudFrameColor() const { return mHudFrameColor; }
  const CColor& GetMissileIconColorActive() const { return mMissileIconColorActive; }
  const CColor& GetVisorBeamMenuItemActive() const { return mVisorBeamMenuItemActive; }
  const CColor& GetVisorBeamMenuItemInactive() const { return mVisorBeamMenuColorInactive; }
  const CColor& GetEnergyBarFilledLowEnergy() const { return mEnergyBarFilledLowEnergy; }
  const CColor& GetEnergyBarShadowLowEnergy() const { return mEnergyBarShadowLowEnergy; }
  const CColor& GetEnergyBarEmptyLowEnergy() const { return mEnergyBarEmptyLowEnergy; }
  const CColor& GetHudDamageLightColor() const { return mHudDamageLightColor; }
  const CColor& GetVisorMenuTextFont() const { return mVisorMenuTextFont; }
  const CColor& GetVisorMenuTextOutline() const { return mVisorMenuTextOutline; }
  const CColor& GetBeamMenuTextFont() const { return mBeamMenuTextFont; }
  const CColor& GetBeamMenuTextOutline() const { return mBeamMenuTextOutline; }
  const CColor& GetEnergyWarningFont() const { return mEnergyWarningFont; }
  const CColor& GetThreatWarningFont() const { return mThreatWarningFont; }
  const CColor& GetMissileWarningFont() const { return mMissileWarningFont; }
  const CColor& GetThreatBarFilled() const { return mThreatBarFilled; }
  const CColor& GetThreatBarShadow() const { return mThreatBarShadow; }
  const CColor& GetThreatBarEmpty() const { return mThreatBarEmpty; }
  const CColor& GetMissileBarFilled() const { return mMissileBarFilled; }
  const CColor& GetMissileBarShadow() const { return mMissileBarShadow; }
  const CColor& GetMissileBarEmpty() const { return mMissileBarEmpty; }
  const CColor& GetThreatIconColor() const { return mThreatIconColor; }
  const CColor& GetTickDecoColor() const { return mTickDecoColor; }
  const CColor& GetHelmetLightColor() const { return mHelmetLightColor; }
  const CColor& GetThreatIconSafeColor() const { return mThreatIconSafeColor; }
  const CColor& GetMissileIconColorInactive() const { return mMissileIconColorInactive; }
  const CColor& GetMissileIconColorChargedCanAlt() const {
    return mMissileIconColorChargedCanAlt;
  }
  const CColor& GetMissileIconColorChargedNoAlt() const { return mMissileIconColorChargedNoAlt; }
  const CColor& GetMissileIconColorDepleteAlt() const { return mMissileIconColorDepleteAlt; }
  const CColor& GetVisorBeamMenuLozColor() const { return mVisorBeamMenuLozColor; }
  const CColor& GetEnergyWarningOutline() const { return mEnergyWarningOutline; }
  const CColor& GetThreatWarningOutline() const { return mThreatWarningOutline; }
  const CColor& GetMissileWarningOutline() const { return mMissileWarningOutline; }
  const CColor& GetDamageAmbientColor() const { return mDamageAmbientColor; }
  const CColor& GetScanFrameInactiveColor() const { return mScanFrameInactiveColor; }
  const CColor& GetScanFrameActiveColor() const { return mScanFrameActiveColor; }
  const CColor& GetScanFrameImpulseColor() const { return mScanFrameImpulseColor; }
  const CColor& GetScanVisorHudLightMultiply() const { return mScanVisorHudLightMultiply; }
  const CColor& GetScanVisorScreenDimColor() const { return mScanVisorScreenDimColor; }
  const CColor& GetThermalVisorHudLightMultiply() const { return mThermalVisorHudLightMultiply; }
  const CColor& GetEnergyDrainFilterColor() const { return mEnergyDrainFilterColor; }
  const CColor& GetDamageAmbientPulseColor() const { return mDamageAmbientPulseColor; }
  const CColor& GetEnergyBarFlashColor() const { return mEnergyBarFlashColor; }
  const CColor& GetXRayEnergyDecoColor() const { return mXrayEnergyDecoColor; }
  const CColor& GetScanDataDotColor() const { return mScanDataDotColor; }
  const CColor& GetPowerBombDigitAvailableFont() const { return mPowerBombDigitAvailableFont; }
  const CColor& GetPowerBombDigitAvailableOutline() const {
    return mPowerBombDigitAvailableOutline;
  }
  const CColor& GetBallBombFilledColor() const { return mBallBombFilled; }
  const CColor& GetBallBombEmptyColor() const { return mBallBombEmpty; }
  const CColor& GetPowerBombIconAvailableColor() const { return mPowerBombIconAvailable; }
  const CColor& GetBallBombEnergyColor() const { return mBallEnergyDeco; }
  const CColor& GetBallBombDecoColor() const { return mBallBombDeco; }
  const CColor& GetPowerBombDigitDelpetedFont() const { return mPowerBombDigitDepletedFont; }
  const CColor& GetPowerBombDigitDelpetedOutline() const {
    return mPowerBombDigitDepletedOutline;
  }
  const CColor& GetPowerBombIconDepletedColor() const { return mPowerBombIconUnavailable; }
  const CColor& GetScanDisplayImagePaneColor() const { return mScanDisplayImagePaneColor; }
  const CColor& GetThreatIconWarningColor() const { return mThreatIconWarningColor; }
  const CColor& GetHudCounterFill() const { return mHudCounterFill; }
  const CColor& GetHudCounterOutline() const { return mHudCounterOutline; }
  const CColor& GetScanIconCriticalColor() const { return mScanIconCriticalColor; }
  const CColor& GetScanIconCriticalDimColor() const { return mScanIconCriticalDimColor; }
  const CColor& GetScanIconNoncriticalColor() const { return mScanIconNoncriticalColor; }
  const CColor& GetScanIconNoncriticalDimColor() const { return mScanIconNoncriticalDimColor; }
  const CColor& GetScanReticuleColor() const { return mScanReticuleColor; }
  const CColor& GetThreatDigitsFont() const { return mThreatDigitsFont; }
  const CColor& GetThreatDigitsOutline() const { return mThreatDigitsOutline; }
  const CColor& GetMissileDigitsFont() const { return mMissileDigitsFont; }
  const CColor& GetMissileDigitsOutline() const { return mMissileDigitsOutline; }
  const CColor& GetThermalDecoColor() const { return mThermalDecoColor; }
  const CColor& GetThermalOutlinesColor() const { return mThermalOutlinesColor; }
  const CColor& GetThermalLockColor() const { return mThermalLockColor; }
  const CColor& GetPauseItemAmberColor() const { return mPauseItemAmber; }
  const CColor& GetPauseItemBlueColor() const { return mPauseItemBlue; }

public:
  CColor mPauseBlurFilterColor;
  CColor mRadarStuffColor;
  CColor mRadarPlayerPaintColor;
  CColor mRadarEnemyPaintColor;
  CColor mHudMessageFill;
  CColor mHudMessageOutline;
  CColor mHudFrameColor;
  CColor x20_;
  CColor x24_;
  CColor mMissileIconColorActive;
  CColor mVisorBeamMenuItemActive;
  CColor mVisorBeamMenuColorInactive;
  CColor mEnergyBarFilledLowEnergy;
  CColor mEnergyBarShadowLowEnergy;
  CColor mEnergyBarEmptyLowEnergy;
  CColor mHudDamageLightColor;
  CColor x44_;
  CColor x48_;
  CColor mVisorMenuTextFont;
  CColor mVisorMenuTextOutline;
  CColor mBeamMenuTextFont;
  CColor mBeamMenuTextOutline;
  CColor mEnergyWarningFont;
  CColor mThreatWarningFont;
  CColor mMissileWarningFont;
  CColor mThreatBarFilled;
  CColor mThreatBarShadow;
  CColor mThreatBarEmpty;
  CColor mMissileBarFilled;
  CColor mMissileBarShadow;
  CColor mMissileBarEmpty;
  CColor mThreatIconColor;
  CColor x84_;
  CColor mTickDecoColor;
  CColor mHelmetLightColor;
  CColor mThreatIconSafeColor;
  CColor mMissileIconColorInactive;
  CColor mMissileIconColorChargedCanAlt;
  CColor mMissileIconColorChargedNoAlt;
  CColor mMissileIconColorDepleteAlt;
  CColor xa4_;
  CColor xa8_;
  CColor xac_;
  CColor mVisorBeamMenuLozColor;
  CColor mEnergyWarningOutline;
  CColor mThreatWarningOutline;
  CColor mMissileWarningOutline;
  CColor xc0_;
  CColor mDamageAmbientColor;
  CColor mScanFrameInactiveColor;
  CColor mScanFrameActiveColor;
  CColor mScanFrameImpulseColor;
  CColor mScanVisorHudLightMultiply;
  CColor mScanVisorScreenDimColor;
  CColor mThermalVisorHudLightMultiply;
  CColor mEnergyDrainFilterColor;
  CColor mDamageAmbientPulseColor;
  CColor mEnergyBarFlashColor;
  CColor xec_;
  CColor xf0_;
  CColor xf4_;
  CColor xf8_;
  CColor xfc_;
  CColor mXrayEnergyDecoColor;
  CColor x104_;
  CColor x108_;
  CColor x10c_;
  CColor x110_;
  CColor x114_;
  CColor x118_;
  CColor x11c_;
  CColor x120_;
  CColor x124_;
  CColor x128_;
  CColor x12c_;
  CColor x130_;
  CColor x134_;
  CColor mScanDataDotColor;
  CColor mPowerBombDigitAvailableFont;
  CColor mPowerBombDigitAvailableOutline;
  CColor x144_;
  CColor mBallBombFilled;
  CColor mBallBombEmpty;
  CColor mPowerBombIconAvailable;
  CColor x154_;
  CColor mBallEnergyDeco;
  CColor mBallBombDeco;
  CColor mPowerBombDigitDepletedFont;
  CColor mPowerBombDigitDepletedOutline;
  CColor mPowerBombIconUnavailable;
  CColor x16c_;
  CColor x170_;
  CColor mScanDisplayImagePaneColor;
  CColor x178_;
  CColor mThreatIconWarningColor;
  CColor mHudCounterFill;
  CColor mHudCounterOutline;
  CColor mScanIconCriticalColor;
  CColor mScanIconCriticalDimColor;
  CColor mScanIconNoncriticalColor;
  CColor mScanIconNoncriticalDimColor;
  CColor mScanReticuleColor;
  CColor mThreatDigitsFont;
  CColor mThreatDigitsOutline;
  CColor mMissileDigitsFont;
  CColor mMissileDigitsOutline;
  CColor mThermalDecoColor;
  CColor mThermalOutlinesColor;
  CColor x1b4_;
  CColor mThermalLockColor;
  CColor mPauseItemAmber;
  CColor mPauseItemBlue;
  /* Combat, Scan, XRay, Thermal, Ball */
  rstl::reserved_vector< SPerVisorColors, 5 > mPerVisorColors;
};
CHECK_SIZEOF(CTweakGuiColors, 0x254)

extern CTweakGuiColors* gpTweakGuiColors;

#endif // _CTWEAKGUICOLORS
