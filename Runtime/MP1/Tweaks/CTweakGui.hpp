#pragma once

#include "Runtime/Tweaks/ITweakGui.hpp"
#include "rstl.hpp"

namespace metaforce::MP1 {
struct CTweakGui final : Tweaks::ITweakGui {
  bool x4_{};
  float x8_mapAlphaInterp{};
  float xc_pauseBlurFactor{};
  float x10_radarXYRadius{};
  float x14_{};
  float x18_{};
  float x1c_{};
  float x20_{};
  float x24_radarZRadius{};
  float x28_radarZCloseRadius{};
  u32 x2c_ = 0;
  float x30_{};
  float x34_energyBarFilledSpeed{};
  float x38_energyBarShadowSpeed{};
  float x3c_energyBarDrainDelay{};
  bool x40_energyBarAlwaysResetDelay{};
  float x44_hudDamagePracticalsGainConstant{};
  float x48_hudDamagePracticalsGainLinear{};
  float x4c_hudDamagePracticalsInitConstant{};
  float x50_hudDamagePracticalsInitLinear{};
  float x54_hudDamageLightSpotAngle{};
  float x58_damageLightAngleC{};
  float x5c_damageLightAngleL{};
  float x60_damageLightAngleQ{};
  zeus::CVector3f x64_damageLightPreTranslate;
  zeus::CVector3f x70_damageLightCenterTranslate;
  float x7c_damageLightXfXAngle{};
  float x80_damageLightXfZAngle{};
  float x84_hudDecoShakeTranslateVelConstant{};
  float x88_hudDecoShakeTranslateVelLinear{};
  float x8c_maxDecoDamageShakeTranslate{};
  float x90_decoDamageShakeDeceleration{};
  float x94_decoShakeGainConstant{};
  float x98_decoShakeGainLinear{};
  float x9c_decoShakeInitConstant{};
  float xa0_decoShakeInitLinear{};
  float xa4_maxDecoDamageShakeRotate{};
  u32 xa8_hudCamFovTweak{};
  u32 xac_hudCamYTweak{};
  u32 xb0_hudCamZTweak{};
  float xb4_{};
  float xb8_{};
  float xbc_{};
  float xc0_beamVisorMenuAnimTime{};
  float xc4_visorBeamMenuItemActiveScale{};
  float xc8_visorBeamMenuItemInactiveScale{};
  float xcc_visorBeamMenuItemTranslate{};
  float xd0_{};
  u32 xd4_{};
  float xd8_{};
  float xdc_{};
  float xe0_{};
  float xe4_threatRange{};
  float xe8_radarScopeCoordRadius{};
  float xec_radarPlayerPaintRadius{};
  float xf0_radarEnemyPaintRadius{};
  float xf4_missileArrowVisTime{};
  EHudVisMode xf8_hudVisMode;
  EHelmetVisMode xfc_helmetVisMode;
  u32 x100_enableAutoMapper{};
  u32 x104_{};
  u32 x108_enableTargetingManager{};
  u32 x10c_enablePlayerVisor{};
  float x110_threatWarningFraction{};
  float x114_missileWarningFraction{};
  float x118_freeLookFadeTime{};
  float x11c_{};
  float x120_{};
  float x124_{};
  float x128_{};
  float x12c_freeLookSfxPitchScale{};
  bool x130_noAbsoluteFreeLookSfxPitch{};
  float x134_{};
  float x138_{};
  u32 x13c_faceReflectionOrthoWidth{};
  u32 x140_faceReflectionOrthoHeight{};
  u32 x144_faceReflectionDistance{};
  u32 x148_faceReflectionHeight{};
  u32 x14c_faceReflectionAspect{};
  std::string x150_;
  std::string x160_;
  std::string x170_;
  std::string x180_;
  std::string x190_;
  float x1a0_missileWarningPulseTime{};
  float x1a4_explosionLightFalloffMultConstant{};
  float x1a8_explosionLightFalloffMultLinear{};
  float x1ac_explosionLightFalloffMultQuadratic{};
  float x1b0_{};
  float x1b4_hudDamagePeakFactor{};
  float x1b8_hudDamageFilterGainConstant{};
  float x1bc_hudDamageFilterGainLinear{};
  float x1c0_hudDamageFilterInitConstant{};
  float x1c4_hudDamageFilterInitLinear{};
  float x1c8_energyDrainModPeriod{};
  bool x1cc_energyDrainSinusoidalPulse{};
  bool x1cd_energyDrainFilterAdditive{};
  float x1d0_hudDamagePulseDuration{};
  float x1d4_hudDamageColorGain{};
  float x1d8_hudDecoShakeTranslateGain{};
  float x1dc_hudLagOffsetScale{};
  float x1e0_{};
  float x1e4_{};
  float x1e8_{};
  float x1ec_{};
  float x1f0_{};
  float x1f4_{};
  float x1f8_{};
  float x1fc_{};
  zeus::CColor x200_;
  float x204_xrayBlurScaleLinear = 0.0014f;
  float x208_xrayBlurScaleQuadratic = 0.0000525f;
  float x20c_{};
  float x210_scanSidesAngle{};
  float x214_scanSidesXScale{};
  float x218_scanSidesPositionEnd{};
  float x21c_{};
  float x220_scanSidesDuration{};
  float x224_scanSidesStartTime{};
  float x228_scanSidesEndTime{};
  float x22c_scanDataDotRadius{};
  float x230_scanDataDotPosRandMag{};
  float x234_scanDataDotSeekDurationMin{};
  float x238_scanDataDotSeekDurationMax{};
  float x23c_scanDataDotHoldDurationMin{};
  float x240_scanDataDotHoldDurationMax{};
  float x244_scanAppearanceDuration{};
  float x248_scanPaneFlashFactor{};
  float x24c_scanPaneFadeInTime{};
  float x250_scanPaneFadeOutTime{};
  float x254_ballViewportYReduction{};
  float x258_scanWindowIdleW{};
  float x25c_scanWindowIdleH{};
  float x260_scanWindowActiveW{};
  float x264_scanWindowActiveH{};
  float x268_scanWindowMagnification{};
  float x26c_scanWindowScanningAspect{};
  float x270_scanSidesPositionStart{};
  bool x274_showAutomapperInMorphball{};
  bool x275_latchArticleText = true;
  float x278_wtMgrCharsPerSfx{};
  u32 x27c_xrayFogMode{};
  float x280_xrayFogNearZ{};
  float x284_xrayFogFarZ{};
  zeus::CColor x288_xrayFogColor;
  float x28c_thermalVisorLevel{};
  zeus::CColor x290_thermalVisorColor;
  std::array<zeus::CColor, 4> x294_hudLightAddPerVisor;
  std::array<zeus::CColor, 4> x2a4_hudLightMultiplyPerVisor;
  zeus::CColor x2b4_hudReflectivityLightColor;
  float x2b8_hudLightAttMulConstant{};
  float x2bc_hudLightAttMulLinear{};
  float x2c0_hudLightAttMulQuadratic{};
  rstl::reserved_vector<float, 2> x2c4_scanSpeeds;
  std::string x2d0_creditsTable;
  std::string x2e0_creditsFont;
  std::string x2f0_japaneseCreditsFont;
  zeus::CColor x300_;
  zeus::CColor x304_;
  float x308_{};
  float x30c_{};
  float x310_{};
  std::string x314_;
  std::string x324_;
  std::string x334_;
  zeus::CColor x344_;
  zeus::CColor x348_;
  zeus::CColor x34c_;
  zeus::CColor x350_;
  zeus::CColor x354_;
  zeus::CColor x358_;
  float x35c_{};
  float x360_{};
  float x364_{};

  CTweakGui() = default;
  CTweakGui(CInputStream& r);
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
  zeus::CVector3f GetDamageLightPreTranslate() const override { return x64_damageLightPreTranslate; }
  zeus::CVector3f GetDamageLightCenterTranslate() const override { return x70_damageLightCenterTranslate; }
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
  u32 GetHudCamFovTweak() const override { return xa8_hudCamFovTweak; }
  u32 GetHudCamYTweak() const override { return xac_hudCamYTweak; }
  u32 GetHudCamZTweak() const override { return xb0_hudCamZTweak; }
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
  u32 GetEnableAutoMapper() const override { return x100_enableAutoMapper; }
  u32 GetEnableTargetingManager() const override { return x108_enableTargetingManager; }
  u32 GetEnablePlayerVisor() const override { return x10c_enablePlayerVisor; }
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
  u32 GetXRayFogMode() const override { return x27c_xrayFogMode; }
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
} // namespace metaforce::MP1
