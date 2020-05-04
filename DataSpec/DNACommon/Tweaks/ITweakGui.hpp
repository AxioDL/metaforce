#pragma once

#include "ITweak.hpp"
#include "zeus/CVector2f.hpp"

namespace DataSpec {

struct ITweakGui : ITweak {
  enum class EHudVisMode : atUint32 { Zero, One, Two, Three };

  enum class EHelmetVisMode : atUint32 { ReducedUpdate, NotVisible, Deco, HelmetDeco, GlowHelmetDeco, HelmetOnly };

  virtual float GetMapAlphaInterpolant() const = 0;
  virtual float GetPauseBlurFactor() const = 0;
  virtual float GetRadarXYRadius() const = 0;
  virtual float GetRadarZRadius() const = 0;
  virtual float GetRadarZCloseRadius() const = 0;
  virtual float GetEnergyBarFilledSpeed() const = 0;
  virtual float GetEnergyBarShadowSpeed() const = 0;
  virtual float GetEnergyBarDrainDelay() const = 0;
  virtual bool GetEnergyBarAlwaysResetDelay() const = 0;
  virtual float GetHudDamagePracticalsGainConstant() const = 0;
  virtual float GetHudDamagePracticalsGainLinear() const = 0;
  virtual float GetHudDamagePracticalsInitConstant() const = 0;
  virtual float GetHudDamagePracticalsInitLinear() const = 0;
  virtual float GetHudDamageLightSpotAngle() const = 0;
  virtual float GetDamageLightAngleC() const = 0;
  virtual float GetDamageLightAngleL() const = 0;
  virtual float GetDamageLightAngleQ() const = 0;
  virtual atVec3f GetDamageLightPreTranslate() const = 0;
  virtual atVec3f GetDamageLightCenterTranslate() const = 0;
  virtual float GetDamageLightXfXAngle() const = 0;
  virtual float GetDamageLightXfZAngle() const = 0;
  virtual float GetHudDecoShakeTranslateVelConstant() const = 0;
  virtual float GetHudDecoShakeTranslateVelLinear() const = 0;
  virtual float GetMaxDecoDamageShakeTranslate() const = 0;
  virtual float GetDecoDamageShakeDeceleration() const = 0;
  virtual float GetDecoShakeGainConstant() const = 0;
  virtual float GetDecoShakeGainLinear() const = 0;
  virtual float GetDecoShakeInitConstant() const = 0;
  virtual float GetDecoShakeInitLinear() const = 0;
  virtual float GetMaxDecoDamageShakeRotate() const = 0;
  virtual atUint32 GetHudCamFovTweak() const = 0;
  virtual atUint32 GetHudCamYTweak() const = 0;
  virtual atUint32 GetHudCamZTweak() const = 0;
  virtual float GetBeamVisorMenuAnimTime() const = 0;
  virtual float GetVisorBeamMenuItemActiveScale() const = 0;
  virtual float GetVisorBeamMenuItemInactiveScale() const = 0;
  virtual float GetVisorBeamMenuItemTranslate() const = 0;
  virtual float GetThreatRange() const = 0;
  virtual float GetRadarScopeCoordRadius() const = 0;
  virtual float GetRadarPlayerPaintRadius() const = 0;
  virtual float GetRadarEnemyPaintRadius() const = 0;
  virtual float GetMissileArrowVisTime() const = 0;
  virtual EHudVisMode GetHudVisMode() const = 0;
  virtual EHelmetVisMode GetHelmetVisMode() const = 0;
  virtual atUint32 GetEnableAutoMapper() const = 0;
  virtual atUint32 GetEnableTargetingManager() const = 0;
  virtual atUint32 GetEnablePlayerVisor() const = 0;
  virtual float GetThreatWarningFraction() const = 0;
  virtual float GetMissileWarningFraction() const = 0;
  virtual float GetFreeLookFadeTime() const = 0;
  virtual float GetFreeLookSfxPitchScale() const = 0;
  virtual bool GetNoAbsoluteFreeLookSfxPitch() const = 0;
  virtual float GetFaceReflectionOrthoWidth() const = 0;
  virtual float GetFaceReflectionOrthoHeight() const = 0;
  virtual float GetFaceReflectionDistance() const = 0;
  virtual float GetFaceReflectionHeight() const = 0;
  virtual float GetFaceReflectionAspect() const = 0;
  virtual float GetMissileWarningPulseTime() const = 0;
  virtual float GetExplosionLightFalloffMultConstant() const = 0;
  virtual float GetExplosionLightFalloffMultLinear() const = 0;
  virtual float GetExplosionLightFalloffMultQuadratic() const = 0;
  virtual float GetHudDamagePeakFactor() const = 0;
  virtual float GetHudDamageFilterGainConstant() const = 0;
  virtual float GetHudDamageFilterGainLinear() const = 0;
  virtual float GetHudDamageFilterInitConstant() const = 0;
  virtual float GetHudDamageFilterInitLinear() const = 0;
  virtual float GetEnergyDrainModPeriod() const = 0;
  virtual bool GetEnergyDrainSinusoidalPulse() const = 0;
  virtual bool GetEnergyDrainFilterAdditive() const = 0;
  virtual float GetHudDamagePulseDuration() const = 0;
  virtual float GetHudDamageColorGain() const = 0;
  virtual float GetHudDecoShakeTranslateGain() const = 0;
  virtual float GetHudLagOffsetScale() const = 0;
  virtual float GetScanAppearanceDuration() const = 0;
  virtual float GetScanPaneFlashFactor() const = 0;
  virtual float GetScanPaneFadeInTime() const = 0;
  virtual float GetScanPaneFadeOutTime() const = 0;
  virtual float GetBallViewportYReduction() const = 0;
  virtual float GetScanWindowIdleWidth() const = 0;
  virtual float GetScanWindowIdleHeight() const = 0;
  virtual float GetScanWindowActiveWidth() const = 0;
  virtual float GetScanWindowActiveHeight() const = 0;
  virtual float GetScanWindowMagnification() const = 0;
  virtual float GetScanWindowScanningAspect() const = 0;
  virtual float GetScanSpeed(int idx) const = 0;
  virtual float GetXrayBlurScaleLinear() const = 0;
  virtual float GetXrayBlurScaleQuadratic() const = 0;
  virtual float GetScanSidesAngle() const = 0;
  virtual float GetScanSidesXScale() const = 0;
  virtual float GetScanSidesPositionEnd() const = 0;
  virtual float GetScanSidesDuration() const = 0;
  virtual float GetScanSidesStartTime() const = 0;
  virtual float GetScanSidesEndTime() const = 0;
  virtual float GetScanDataDotRadius() const = 0;
  virtual float GetScanDataDotPosRandMagnitude() const = 0;
  virtual float GetScanDataDotSeekDurationMin() const = 0;
  virtual float GetScanDataDotSeekDurationMax() const = 0;
  virtual float GetScanDataDotHoldDurationMin() const = 0;
  virtual float GetScanDataDotHoldDurationMax() const = 0;
  virtual float GetScanSidesPositionStart() const = 0;
  virtual bool GetShowAutomapperInMorphball() const = 0;
  virtual bool GetLatchArticleText() const = 0;
  virtual float GetWorldTransManagerCharsPerSfx() const = 0;
  virtual atUint32 GetXRayFogMode() const = 0;
  virtual float GetXRayFogNearZ() const = 0;
  virtual float GetXRayFogFarZ() const = 0;
  virtual const zeus::CColor& GetXRayFogColor() const = 0;
  virtual float GetThermalVisorLevel() const = 0;
  virtual const zeus::CColor& GetThermalVisorColor() const = 0;
  virtual const zeus::CColor& GetVisorHudLightAdd(int v) const = 0;
  virtual const zeus::CColor& GetVisorHudLightMultiply(int v) const = 0;
  virtual const zeus::CColor& GetHudReflectivityLightColor() const = 0;
  virtual float GetHudLightAttMulConstant() const = 0;
  virtual float GetHudLightAttMulLinear() const = 0;
  virtual float GetHudLightAttMulQuadratic() const = 0;
  virtual std::string_view GetCreditsTable() const = 0;
  virtual std::string_view GetCreditsFont() const =0;
  virtual std::string_view GetJapaneseCreditsFont() const=0;
  virtual const zeus::CColor& GetCreditsTextFontColor() const=0;
  virtual const zeus::CColor& GetCreditsTextBorderColor() const=0;

  static float FaceReflectionDistanceDebugValueToActualValue(float v) { return 0.015f * v + 0.2f; }
  static float FaceReflectionHeightDebugValueToActualValue(float v) { return 0.005f * v - 0.05f; }
  static float FaceReflectionAspectDebugValueToActualValue(float v) { return 0.05f * v + 1.f; }
  static float FaceReflectionOrthoWidthDebugValueToActualValue(float v) { return 0.007f * v + 0.02f; }
  static float FaceReflectionOrthoHeightDebugValueToActualValue(float v) { return 0.007f * v + 0.02f; }
};

} // namespace DataSpec
