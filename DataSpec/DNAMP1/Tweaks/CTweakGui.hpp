#pragma once

#include "../../DNACommon/Tweaks/ITweakGui.hpp"

namespace DataSpec::DNAMP1 {
struct CTweakGui final : ITweakGui {
  AT_DECL_DNA_YAML
  Value<bool> x4_;
  Value<float> x8_mapAlphaInterp;
  Value<float> xc_pauseBlurFactor;
  Value<float> x10_radarXYRadius;
  Value<float> x14_;
  Value<float> x18_;
  Value<float> x1c_;
  Value<float> x20_;
  Value<float> x24_radarZRadius;
  Value<float> x28_radarZCloseRadius;
  atUint32 x2c_ = 0;
  Value<float> x30_;
  Value<float> x34_energyBarFilledSpeed;
  Value<float> x38_energyBarShadowSpeed;
  Value<float> x3c_energyBarDrainDelay;
  Value<bool> x40_energyBarAlwaysResetDelay;
  Value<float> x44_hudDamagePracticalsGainConstant;
  Value<float> x48_hudDamagePracticalsGainLinear;
  Value<float> x4c_hudDamagePracticalsInitConstant;
  Value<float> x50_hudDamagePracticalsInitLinear;
  Value<float> x54_hudDamageLightSpotAngle;
  Value<float> x58_damageLightAngleC;
  Value<float> x5c_damageLightAngleL;
  Value<float> x60_damageLightAngleQ;
  Value<atVec3f> x64_damageLightPreTranslate;
  Value<atVec3f> x70_damageLightCenterTranslate;
  Value<float> x7c_damageLightXfXAngle;
  Value<float> x80_damageLightXfZAngle;
  Value<float> x84_hudDecoShakeTranslateVelConstant;
  Value<float> x88_hudDecoShakeTranslateVelLinear;
  Value<float> x8c_maxDecoDamageShakeTranslate;
  Value<float> x90_decoDamageShakeDeceleration;
  Value<float> x94_decoShakeGainConstant;
  Value<float> x98_decoShakeGainLinear;
  Value<float> x9c_decoShakeInitConstant;
  Value<float> xa0_decoShakeInitLinear;
  Value<float> xa4_maxDecoDamageShakeRotate;
  Value<atUint32> xa8_hudCamFovTweak;
  Value<atUint32> xac_hudCamYTweak;
  Value<atUint32> xb0_hudCamZTweak;
  Value<float> xb4_;
  Value<float> xb8_;
  Value<float> xbc_;
  Value<float> xc0_beamVisorMenuAnimTime;
  Value<float> xc4_visorBeamMenuItemActiveScale;
  Value<float> xc8_visorBeamMenuItemInactiveScale;
  Value<float> xcc_visorBeamMenuItemTranslate;
  Value<float> xd0_;
  Value<atUint32> xd4_;
  Value<float> xd8_;
  Value<float> xdc_;
  Value<float> xe0_;
  Value<float> xe4_threatRange;
  Value<float> xe8_radarScopeCoordRadius;
  Value<float> xec_radarPlayerPaintRadius;
  Value<float> xf0_radarEnemyPaintRadius;
  Value<float> xf4_missileArrowVisTime;
  Value<EHudVisMode> xf8_hudVisMode;
  Value<EHelmetVisMode> xfc_helmetVisMode;
  Value<atUint32> x100_enableAutoMapper;
  Value<atUint32> x104_;
  Value<atUint32> x108_enableTargetingManager;
  Value<atUint32> x10c_enablePlayerVisor;
  Value<float> x110_threatWarningFraction;
  Value<float> x114_missileWarningFraction;
  Value<float> x118_freeLookFadeTime;
  Value<float> x11c_;
  Value<float> x120_;
  Value<float> x124_;
  Value<float> x128_;
  Value<float> x12c_freeLookSfxPitchScale;
  Value<bool> x130_noAbsoluteFreeLookSfxPitch;
  Value<float> x134_;
  Value<float> x138_;
  Value<atUint32> x13c_faceReflectionOrthoWidth;
  Value<atUint32> x140_faceReflectionOrthoHeight;
  Value<atUint32> x144_faceReflectionDistance;
  Value<atUint32> x148_faceReflectionHeight;
  Value<atUint32> x14c_faceReflectionAspect;
  String<-1> x150_;
  String<-1> x160_;
  String<-1> x170_;
  String<-1> x180_;
  String<-1> x190_;
  Value<float> x1a0_missileWarningPulseTime;
  Value<float> x1a4_explosionLightFalloffMultConstant;
  Value<float> x1a8_explosionLightFalloffMultLinear;
  Value<float> x1ac_explosionLightFalloffMultQuadratic;
  Value<float> x1b0_;
  Value<float> x1b4_hudDamagePeakFactor;
  Value<float> x1b8_hudDamageFilterGainConstant;
  Value<float> x1bc_hudDamageFilterGainLinear;
  Value<float> x1c0_hudDamageFilterInitConstant;
  Value<float> x1c4_hudDamageFilterInitLinear;
  Value<float> x1c8_energyDrainModPeriod;
  Value<bool> x1cc_energyDrainSinusoidalPulse;
  Value<bool> x1cd_energyDrainFilterAdditive;
  Value<float> x1d0_hudDamagePulseDuration;
  Value<float> x1d4_hudDamageColorGain;
  Value<float> x1d8_hudDecoShakeTranslateGain;
  Value<float> x1dc_hudLagOffsetScale;
  Value<float> x1e0_;
  Value<float> x1e4_;
  Value<float> x1e8_;
  Value<float> x1ec_;
  Value<float> x1f0_;
  Value<float> x1f4_;
  Value<float> x1f8_;
  Value<float> x1fc_;
  zeus::CColor x200_;
  float x204_xrayBlurScaleLinear = 0.0014f;
  float x208_xrayBlurScaleQuadratic = 0.0000525f;
  Value<float> x20c_;
  Value<float> x210_scanSidesAngle;
  Value<float> x214_scanSidesXScale;
  Value<float> x218_scanSidesPositionEnd;
  Value<float> x21c_;
  Value<float> x220_scanSidesDuration;
  Value<float> x224_scanSidesStartTime;
  float x228_scanSidesEndTime;
  Value<float> x22c_scanDataDotRadius;
  Value<float> x230_scanDataDotPosRandMag;
  Value<float> x234_scanDataDotSeekDurationMin;
  Value<float> x238_scanDataDotSeekDurationMax;
  Value<float> x23c_scanDataDotHoldDurationMin;
  Value<float> x240_scanDataDotHoldDurationMax;
  Value<float> x244_scanAppearanceDuration;
  Value<float> x248_scanPaneFlashFactor;
  Value<float> x24c_scanPaneFadeInTime;
  Value<float> x250_scanPaneFadeOutTime;
  Value<float> x254_ballViewportYReduction;
  Value<float> x258_scanWindowIdleW;
  Value<float> x25c_scanWindowIdleH;
  Value<float> x260_scanWindowActiveW;
  Value<float> x264_scanWindowActiveH;
  Value<float> x268_scanWindowMagnification;
  Value<float> x26c_scanWindowScanningAspect;
  Value<float> x270_scanSidesPositionStart;
  Value<bool> x274_showAutomapperInMorphball;
  bool x275_latchArticleText = true;
  Value<float> x278_wtMgrCharsPerSfx;
  Value<atUint32> x27c_xrayFogMode;
  Value<float> x280_xrayFogNearZ;
  Value<float> x284_xrayFogFarZ;
  DNAColor x288_xrayFogColor;
  Value<float> x28c_thermalVisorLevel;
  DNAColor x290_thermalVisorColor;
  DNAColor x294_hudLightAddPerVisor[4];
  DNAColor x2a4_hudLightMultiplyPerVisor[4];
  DNAColor x2b4_hudReflectivityLightColor;
  Value<float> x2b8_hudLightAttMulConstant;
  Value<float> x2bc_hudLightAttMulLinear;
  Value<float> x2c0_hudLightAttMulQuadratic;
  Value<atUint32> m_scanSpeedsCount;
  Vector<float, AT_DNA_COUNT(m_scanSpeedsCount)> x2c4_scanSpeeds;
  String<-1> x2d0_creditsTable;
  String<-1> x2e0_creditsFont;
  String<-1> x2f0_japaneseCreditsFont;
  DNAColor x300_;
  DNAColor x304_;
  Value<float> x308_;
  Value<float> x30c_;
  Value<float> x310_;
  String<-1> x314_;
  String<-1> x324_;
  String<-1> x334_;
  DNAColor x344_;
  DNAColor x348_;
  DNAColor x34c_;
  DNAColor x350_;
  DNAColor x354_;
  DNAColor x358_;
  Value<float> x35c_;
  Value<float> x360_;
  Value<float> x364_;

  CTweakGui() = default;
  CTweakGui(athena::io::IStreamReader& r) {
    this->read(r);
    FixupValues();
  }

  float GetMapAlphaInterpolant() const override { return x8_mapAlphaInterp; }
  float GetPauseBlurFactor() const override { return xc_pauseBlurFactor; }
  float GetRadarXYRadius() const override { return x10_radarXYRadius; }
  float GetRadarZRadius() const override { return x24_radarZRadius; }
  float GetRadarZCloseRadius() const override { return x28_radarZCloseRadius; }
  float GetEnergyBarFilledSpeed() const override { return x34_energyBarFilledSpeed; }
  float GetEnergyBarShadowSpeed() const override { return x38_energyBarShadowSpeed; }
  float GetEnergyBarDrainDelay() const override { return x3c_energyBarDrainDelay; }
  bool GetEnergyBarAlwaysResetDelay() const override { return x40_energyBarAlwaysResetDelay; }
  float GetHudDamagePracticalsGainConstant() const override { return x44_hudDamagePracticalsGainConstant; }
  float GetHudDamagePracticalsGainLinear() const override { return x48_hudDamagePracticalsGainLinear; }
  float GetHudDamagePracticalsInitConstant() const override { return x4c_hudDamagePracticalsInitConstant; }
  float GetHudDamagePracticalsInitLinear() const override { return x50_hudDamagePracticalsInitLinear; }
  float GetHudDamageLightSpotAngle() const override { return x54_hudDamageLightSpotAngle; }
  float GetDamageLightAngleC() const override { return x58_damageLightAngleC; }
  float GetDamageLightAngleL() const override { return x5c_damageLightAngleL; }
  float GetDamageLightAngleQ() const override { return x60_damageLightAngleQ; }
  atVec3f GetDamageLightPreTranslate() const override { return x64_damageLightPreTranslate; }
  atVec3f GetDamageLightCenterTranslate() const override { return x70_damageLightCenterTranslate; }
  float GetDamageLightXfXAngle() const override { return x7c_damageLightXfXAngle; }
  float GetDamageLightXfZAngle() const override { return x80_damageLightXfZAngle; }
  float GetHudDecoShakeTranslateVelConstant() const override { return x84_hudDecoShakeTranslateVelConstant; }
  float GetHudDecoShakeTranslateVelLinear() const override { return x88_hudDecoShakeTranslateVelLinear; }
  float GetMaxDecoDamageShakeTranslate() const override { return x8c_maxDecoDamageShakeTranslate; }
  float GetDecoDamageShakeDeceleration() const override { return x90_decoDamageShakeDeceleration; }
  float GetDecoShakeGainConstant() const override { return x94_decoShakeGainConstant; }
  float GetDecoShakeGainLinear() const override { return x98_decoShakeGainLinear; }
  float GetDecoShakeInitConstant() const override { return x9c_decoShakeInitConstant; }
  float GetDecoShakeInitLinear() const override { return xa0_decoShakeInitLinear; }
  float GetMaxDecoDamageShakeRotate() const override { return xa4_maxDecoDamageShakeRotate; }
  atUint32 GetHudCamFovTweak() const override { return xa8_hudCamFovTweak; }
  atUint32 GetHudCamYTweak() const override { return xac_hudCamYTweak; }
  atUint32 GetHudCamZTweak() const override { return xb0_hudCamZTweak; }
  float GetBeamVisorMenuAnimTime() const override { return xc0_beamVisorMenuAnimTime; }
  float GetVisorBeamMenuItemActiveScale() const override { return xc4_visorBeamMenuItemActiveScale; }
  float GetVisorBeamMenuItemInactiveScale() const override { return xc8_visorBeamMenuItemInactiveScale; }
  float GetVisorBeamMenuItemTranslate() const override { return xcc_visorBeamMenuItemTranslate; }
  float GetThreatRange() const override { return xe4_threatRange; }
  float GetRadarScopeCoordRadius() const override { return xe8_radarScopeCoordRadius; }
  float GetRadarPlayerPaintRadius() const override { return xec_radarPlayerPaintRadius; }
  float GetRadarEnemyPaintRadius() const override { return xf0_radarEnemyPaintRadius; }
  float GetMissileArrowVisTime() const override { return xf4_missileArrowVisTime; }
  EHudVisMode GetHudVisMode() const override { return xf8_hudVisMode; }
  EHelmetVisMode GetHelmetVisMode() const override { return xfc_helmetVisMode; }
  atUint32 GetEnableAutoMapper() const override { return x100_enableAutoMapper; }
  atUint32 GetEnableTargetingManager() const override { return x108_enableTargetingManager; }
  atUint32 GetEnablePlayerVisor() const override { return x10c_enablePlayerVisor; }
  float GetThreatWarningFraction() const override { return x110_threatWarningFraction; }
  float GetMissileWarningFraction() const override { return x114_missileWarningFraction; }
  float GetFreeLookFadeTime() const override { return x118_freeLookFadeTime; }
  float GetFreeLookSfxPitchScale() const override { return x12c_freeLookSfxPitchScale; }
  bool GetNoAbsoluteFreeLookSfxPitch() const override { return x130_noAbsoluteFreeLookSfxPitch; }
  float GetFaceReflectionOrthoWidth() const override { return x13c_faceReflectionOrthoWidth; }
  float GetFaceReflectionOrthoHeight() const override { return x140_faceReflectionOrthoHeight; }
  float GetFaceReflectionDistance() const override { return x144_faceReflectionDistance; }
  float GetFaceReflectionHeight() const override { return x148_faceReflectionHeight; }
  float GetFaceReflectionAspect() const override { return x14c_faceReflectionAspect; }
  float GetMissileWarningPulseTime() const override { return x1a0_missileWarningPulseTime; }
  float GetExplosionLightFalloffMultConstant() const override { return x1a4_explosionLightFalloffMultConstant; }
  float GetExplosionLightFalloffMultLinear() const override { return x1a8_explosionLightFalloffMultLinear; }
  float GetExplosionLightFalloffMultQuadratic() const override { return x1ac_explosionLightFalloffMultQuadratic; }
  float GetHudDamagePeakFactor() const override { return x1b4_hudDamagePeakFactor; }
  float GetHudDamageFilterGainConstant() const override { return x1b8_hudDamageFilterGainConstant; }
  float GetHudDamageFilterGainLinear() const override { return x1bc_hudDamageFilterGainLinear; }
  float GetHudDamageFilterInitConstant() const override { return x1c0_hudDamageFilterInitConstant; }
  float GetHudDamageFilterInitLinear() const override { return x1c4_hudDamageFilterInitLinear; }
  float GetEnergyDrainModPeriod() const override { return x1c8_energyDrainModPeriod; }
  bool GetEnergyDrainSinusoidalPulse() const override { return x1cc_energyDrainSinusoidalPulse; }
  bool GetEnergyDrainFilterAdditive() const override { return x1cd_energyDrainFilterAdditive; }
  float GetHudDamagePulseDuration() const override { return x1d0_hudDamagePulseDuration; }
  float GetHudDamageColorGain() const override { return x1d4_hudDamageColorGain; }
  float GetHudDecoShakeTranslateGain() const override { return x1d8_hudDecoShakeTranslateGain; }
  float GetHudLagOffsetScale() const override { return x1dc_hudLagOffsetScale; }
  float GetXrayBlurScaleLinear() const override { return x204_xrayBlurScaleLinear; }
  float GetXrayBlurScaleQuadratic() const override { return x208_xrayBlurScaleQuadratic; }
  float GetScanSidesAngle() const override { return x210_scanSidesAngle; }
  float GetScanSidesXScale() const override { return x214_scanSidesXScale; }
  float GetScanSidesPositionEnd() const override { return x218_scanSidesPositionEnd; }
  float GetScanSidesDuration() const override { return x220_scanSidesDuration; }
  float GetScanSidesStartTime() const override { return x224_scanSidesStartTime; }
  float GetScanSidesEndTime() const override { return x228_scanSidesEndTime; }
  float GetScanDataDotRadius() const override { return x22c_scanDataDotRadius; }
  float GetScanDataDotPosRandMagnitude() const override { return x230_scanDataDotPosRandMag; }
  float GetScanDataDotSeekDurationMin() const override { return x234_scanDataDotSeekDurationMin; }
  float GetScanDataDotSeekDurationMax() const override { return x238_scanDataDotSeekDurationMax; }
  float GetScanDataDotHoldDurationMin() const override { return x23c_scanDataDotHoldDurationMin; }
  float GetScanDataDotHoldDurationMax() const override { return x240_scanDataDotHoldDurationMax; }
  float GetScanAppearanceDuration() const override { return x244_scanAppearanceDuration; }
  float GetScanPaneFlashFactor() const override { return x248_scanPaneFlashFactor; }
  float GetScanPaneFadeInTime() const override { return x24c_scanPaneFadeInTime; }
  float GetScanPaneFadeOutTime() const override { return x250_scanPaneFadeOutTime; }
  float GetBallViewportYReduction() const override { return x254_ballViewportYReduction; }
  float GetScanWindowIdleWidth() const override { return x258_scanWindowIdleW; }
  float GetScanWindowIdleHeight() const override { return x25c_scanWindowIdleH; }
  float GetScanWindowActiveWidth() const override { return x260_scanWindowActiveW; }
  float GetScanWindowActiveHeight() const override { return x264_scanWindowActiveH; }
  float GetScanWindowMagnification() const override { return x268_scanWindowMagnification; }
  float GetScanWindowScanningAspect() const override { return x26c_scanWindowScanningAspect; }
  float GetScanSidesPositionStart() const override { return x270_scanSidesPositionStart; }
  bool GetShowAutomapperInMorphball() const override { return x274_showAutomapperInMorphball; }
  bool GetLatchArticleText() const override { return x275_latchArticleText; }
  float GetWorldTransManagerCharsPerSfx() const override { return x278_wtMgrCharsPerSfx; }
  atUint32 GetXRayFogMode() const override { return x27c_xrayFogMode; }
  float GetXRayFogNearZ() const override { return x280_xrayFogNearZ; }
  float GetXRayFogFarZ() const override { return x284_xrayFogFarZ; }
  const zeus::CColor& GetXRayFogColor() const override { return x288_xrayFogColor; }
  float GetThermalVisorLevel() const override { return x28c_thermalVisorLevel; }
  const zeus::CColor& GetThermalVisorColor() const override { return x290_thermalVisorColor; }
  const zeus::CColor& GetVisorHudLightAdd(int v) const override { return x294_hudLightAddPerVisor[v]; }
  const zeus::CColor& GetVisorHudLightMultiply(int v) const override { return x2a4_hudLightMultiplyPerVisor[v]; }
  const zeus::CColor& GetHudReflectivityLightColor() const override { return x2b4_hudReflectivityLightColor; }
  float GetHudLightAttMulConstant() const override { return x2b8_hudLightAttMulConstant; }
  float GetHudLightAttMulLinear() const override { return x2bc_hudLightAttMulLinear; }
  float GetHudLightAttMulQuadratic() const override { return x2c0_hudLightAttMulQuadratic; }
  std::string_view GetCreditsTable() const override { return x2d0_creditsTable; }
  std::string_view GetCreditsFont() const override { return x2e0_creditsFont; }
  std::string_view GetJapaneseCreditsFont() const override { return x2f0_japaneseCreditsFont; }
  const zeus::CColor& GetCreditsTextFontColor() const override { return x300_; }
  const zeus::CColor& GetCreditsTextBorderColor() const override { return x304_; }

  float GetScanSpeed(int idx) const override {
    if (idx < 0 || size_t(idx) >= x2c4_scanSpeeds.size())
      return 0.f;
    return x2c4_scanSpeeds[idx];
  }

  void FixupValues() {
    xd8_ = zeus::degToRad(xd8_);
    xdc_ = zeus::degToRad(xdc_);

    x200_ = zeus::CColor(x1f4_ * 0.25f, x1f8_ * 0.25f, x1fc_ * 0.25f, 1.f);

    x210_scanSidesAngle = zeus::degToRad(x210_scanSidesAngle);
    x228_scanSidesEndTime = x220_scanSidesDuration + x224_scanSidesStartTime;

    if (x27c_xrayFogMode == 1)
      x27c_xrayFogMode = 2;
    else if (x27c_xrayFogMode == 2)
      x27c_xrayFogMode = 4;
    else if (x27c_xrayFogMode == 3)
      x27c_xrayFogMode = 5;
    else
      x27c_xrayFogMode = 0;

    x84_hudDecoShakeTranslateVelConstant *= 2.0f;
  }
};

} // namespace DataSpec::DNAMP1
