#ifndef _CTWEAKGUI
#define _CTWEAKGUI

#include "MetroidPrime/Tweaks/ITweakObject.hpp"

#include "Kyoto/TOneStatic.hpp"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/reserved_vector.hpp"
#include "rstl/string.hpp"

class CTweakGui : public ITweakObject, public TOneStatic< CTweakGui > {
public:
  enum EHudVisMode {
    kHud_Zero,
    kHud_One,
    kHud_Two,
    kHud_Three,
  };

  enum EHelmetVisMode {
    kHelmet_ReducedUpdate,
    kHelmet_NotVisible,
    kHelmet_Deco,
    kHelmet_HelmetDeco,
    kHelmet_GlowHelmetDeco,
    kHelmet_HelmetOnly
  };

  CTweakGui(CInputStream& in);

  float GetMapAlphaInterpolant() const { return mMapAlphaInterp; }
  float GetPauseBlurFactor() const { return mPauseBlurFactor; }
  float GetRadarXYRadius() const { return mRadarXYRadius; }
  float GetRadarZRadius() const { return mRadarZRadius; }
  float GetRadarZCloseRadius() const { return mRadarZCloseRadius; }
  float GetEnergyBarFilledSpeed() const { return mEnergyBarFilledSpeed; }
  float GetEnergyBarShadowSpeed() const { return mEnergyBarShadowSpeed; }
  float GetEnergyBarDrainDelay() const { return mEnergyBarDrainDelay; }
  bool GetEnergyBarAlwaysResetDelay() const { return mEnergyBarAlwaysResetDelay; }
  float GetHudDamagePracticalsGainConstant() const { return mHudDamagePracticalsGainConstant; }
  float GetHudDamagePracticalsGainLinear() const { return mHudDamagePracticalsGainLinear; }
  float GetHudDamagePracticalsInitConstant() const { return mHudDamagePracticalsInitConstant; }
  float GetHudDamagePracticalsInitLinear() const { return mHudDamagePracticalsInitLinear; }
  float GetHudDamageLightSpotAngle() const { return mHudDamageLightSpotAngle; }
  float GetDamageLightAngleC() const { return mDamageLightAngleC; }
  float GetDamageLightAngleL() const { return mDamageLightAngleL; }
  float GetDamageLightAngleQ() const { return mDamageLightAngleQ; }
  const CVector3f& GetDamageLightPreTranslate() const { return mDamageLightPreTranslate; }
  const CVector3f& GetDamageLightCenterTranslate() const { return mDamageLightCenterTranslate; }
  float GetDamageLightXfXAngle() const { return mDamageLightXfXAngle; }
  float GetDamageLightXfZAngle() const { return mDamageLightXfZAngle; }
  float GetHudDecoShakeTranslateVelConstant() const { return mHudDecoShakeTranslateVelConstant; }
  float GetHudDecoShakeTranslateVelLinear() const { return mHudDecoShakeTranslateVelLinear; }
  const float& GetMaxDecoDamageShakeTranslate() const { return mMaxDecoDamageShakeTranslate; }
  float GetDecoDamageShakeDeceleration() const { return mDecoDamageShakeDeceleration; }
  float GetDecoShakeGainConstant() const { return mDecoShakeGainConstant; }
  float GetDecoShakeGainLinear() const { return mDecoShakeGainLinear; }
  float GetDecoShakeInitConstant() const { return mDecoShakeInitConstant; }
  float GetDecoShakeInitLinear() const { return mDecoShakeInitLinear; }
  const float& GetMaxDecoDamageShakeRotate() const { return mMaxDecoDamageShakeRotate; }
  uint GetHudCamFovTweak() const { return mHudCamFovTweak; }
  uint GetHudCamYTweak() const { return mHudCamYTweak; }
  uint GetHudCamZTweak() const { return mHudCamZTweak; }
  float GetBeamVisorMenuAnimTime() const { return mBeamVisorMenuAnimTime; }
  float GetVisorBeamMenuItemActiveScale() const { return mVisorBeamMenuItemActiveScale; }
  float GetVisorBeamMenuItemInactiveScale() const { return mVisorBeamMenuItemInactiveScale; }
  float GetVisorBeamMenuItemTranslate() const { return mVisorBeamMenuItemTranslate; }
  float GetThreatRange() const { return mThreatRange; }
  const float& GetRadarScopeCoordRadius() const { return mRadarScopeCoordRadius; }
  const float& GetRadarPlayerPaintRadius() const { return mRadarPlayerPaintRadius; }
  const float& GetRadarEnemyPaintRadius() const { return mRadarEnemyPaintRadius; }
  float GetMissileArrowVisTime() const { return mMissileArrowVisTime; }
  EHudVisMode GetHudVisMode() const { return mHudVisMode; }
  EHelmetVisMode GetHelmetVisMode() const { return mHelmetVisMode; }
  uint GetEnableAutoMapper() const { return mEnableAutoMapper; }
  uint GetEnableTargetingManager() const { return mEnableTargetingManager; }
  uint GetEnablePlayerVisor() const { return mEnablePlayerVisor; }
  float GetThreatWarningFraction() const { return mThreatWarningFraction; }
  float GetMissileWarningFraction() const { return mMissileWarningFraction; }
  float GetFreeLookFadeTime() const { return mFreeLookFadeTime; }
  float GetFreeLookSfxPitchScale() const { return mFreeLookSfxPitchScale; }
  bool GetNoAbsoluteFreeLookSfxPitch() const { return mNoAbsoluteFreeLookSfxPitch; }
  float GetFaceReflectionOrthoWidth() const { return mFaceReflectionOrthoWidth; }
  float GetFaceReflectionOrthoHeight() const { return mFaceReflectionOrthoHeight; }
  float GetFaceReflectionDistance() const { return mFaceReflectionDistance; }
  float GetFaceReflectionHeight() const { return mFaceReflectionHeight; }
  float GetFaceReflectionAspect() const { return mFaceReflectionAspect; }
  float GetMissileWarningPulseTime() const { return mMissileWarningPulseTime; }
  float GetExplosionLightFalloffMultConstant() const {
    return mExplosionLightFalloffMultConstant;
  }
  float GetExplosionLightFalloffMultLinear() const { return mExplosionLightFalloffMultLinear; }
  float GetExplosionLightFalloffMultQuadratic() const {
    return mExplosionLightFalloffMultQuadratic;
  }
  float GetHudDamagePeakFactor() const { return mHudDamagePeakFactor; }
  float GetHudDamageFilterGainConstant() const { return mHudDamageFilterGainConstant; }
  float GetHudDamageFilterGainLinear() const { return mHudDamageFilterGainLinear; }
  float GetHudDamageFilterInitConstant() const { return mHudDamageFilterInitConstant; }
  float GetHudDamageFilterInitLinear() const { return mHudDamageFilterInitLinear; }
  float GetEnergyDrainModPeriod() const { return mEnergyDrainModPeriod; }
  bool GetEnergyDrainSinusoidalPulse() const { return mEnergyDrainSinusoidalPulse; }
  bool GetEnergyDrainFilterAdditive() const { return mEnergyDrainFilterAdditive; }
  float GetHudDamagePulseDuration() const { return mHudDamagePulseDuration; }
  float GetHudDamageColorGain() const { return mHudDamageColorGain; }
  float GetHudDecoShakeTranslateGain() const { return mHudDecoShakeTranslateGain; }
  float GetHudLagOffsetScale() const { return mHudLagOffsetScale; }
  float GetXrayBlurScaleLinear() const { return mXrayBlurScaleLinear; }
  float GetXrayBlurScaleQuadratic() const { return mXrayBlurScaleQuadratic; }
  float GetScanSidesAngle() const { return mScanSidesAngle; }
  float GetScanSidesXScale() const { return mScanSidesXScale; }
  float GetScanSidesPositionEnd() const { return mScanSidesPositionEnd; }
  const float& GetScanSidesDuration() const { return mScanSidesDuration; }
  const float& GetScanSidesStartTime() const { return mScanSidesStartTime; }
  const float& GetScanSidesEndTime() const { return mScanSidesEndTime; }
  float GetScanDataDotRadius() const { return mScanDataDotRadius; }
  const float& GetScanDataDotPosRandMagnitude() const { return mScanDataDotPosRandMag; }
  float GetScanDataDotSeekDurationMin() const { return mScanDataDotSeekDurationMin; }
  float GetScanDataDotSeekDurationMax() const { return mScanDataDotSeekDurationMax; }
  float GetScanDataDotHoldDurationMin() const { return mScanDataDotHoldDurationMin; }
  float GetScanDataDotHoldDurationMax() const { return mScanDataDotHoldDurationMax; }
  float GetScanAppearanceDuration() const { return mScanAppearanceDuration; }
  float GetScanPaneFlashFactor() const { return mScanPaneFlashFactor; }
  float GetScanPaneFadeInTime() const { return mScanPaneFadeInTime; }
  float GetScanPaneFadeOutTime() const { return mScanPaneFadeOutTime; }
  float GetBallViewportYReduction() const { return mBallViewportYReduction; }
  float GetScanWindowIdleWidth() const { return mScanWindowIdleW; }
  float GetScanWindowIdleHeight() const { return mScanWindowIdleH; }
  float GetScanWindowActiveWidth() const { return mScanWindowActiveW; }
  float GetScanWindowActiveHeight() const { return mScanWindowActiveH; }
  float GetScanWindowMagnification() const { return mScanWindowMagnification; }
  float GetScanWindowScanningAspect() const { return mScanWindowScanningAspect; }
  float GetScanSidesPositionStart() const { return mScanSidesPositionStart; }
  bool GetShowAutomapperInMorphball() const { return mShowAutomapperInMorphball; }
  bool GetLatchArticleText() const { return mLatchArticleText; }
  float GetWorldTransManagerCharsPerSfx() const { return mWtMgrCharsPerSfx; }
  uint GetXRayFogMode() const { return mXrayFogMode; }
  float GetXRayFogNearZ() const { return mXrayFogNearZ; }
  float GetXRayFogFarZ() const { return mXrayFogFarZ; }
  const CColor& GetXRayFogColor() const { return mXrayFogColor; }
  float GetThermalVisorLevel() const { return mThermalVisorLevel; }
  const CColor& GetThermalVisorColor() const { return mThermalVisorColor; }
  const CColor& GetCombatVisorHudLightAdd() const { return mCombatVisorHudLightAdd; }
  const CColor& GetXRayVisorHudLightAdd() const { return mXRayVisorHudLightAdd; }
  const CColor& GetScanVisorHudLightAdd() const { return mScanVisorHudLightAdd; }
  const CColor& GetThermalVisorHudLightAdd() const { return mThermalVisorHudLightAdd; }
  const CColor& GetCombatVisorHudLightMultiply() const { return mCombatVisorHudLightMultiply; }
  const CColor& GetXRayVisorHudLightMultiply() const { return mXRayVisorHudLightMultiply; }
  const CColor& GetScanVisorHudLightMultiply() const { return mScanVisorHudLightMultiply; }
  const CColor& GetThermalVisorHudLightMultiply() const {
    return mThermalVisorHudLightMultiply;
  }
  const CColor& GetHudReflectivityLightColor() const { return mHudReflectivityLightColor; }
  float GetHudLightAttMulConstant() const { return mHudLightAttMulConstant; }
  float GetHudLightAttMulLinear() const { return mHudLightAttMulLinear; }
  float GetHudLightAttMulQuadratic() const { return mHudLightAttMulQuadratic; }
  float GetScanSpeed(uint speed) const { return mScanSpeeds[speed]; }
  const rstl::string& GetCreditsTable() const { return mCreditsTable; }
  const rstl::string& GetCreditsFont() const { return mCreditsFont; }
  const rstl::string& GetJapaneseCreditsFont() const { return mJapaneseCreditsFont; }
  const CColor& GetCreditsTextFontColor() const { return mCreditsTextFontColor; }
  const CColor& GetCreditsTextBorderColor() const { return mCreditsTextBorderColor; }
  const float& GetCredits_x308() const { return x308_; }
  const float& GetCredits_x30c() const { return x30c_; }
  const float& GetCredits_x310() const { return x310_; }

  const rstl::string& GetCompletionScreenTable() const { return x314_; }
  const rstl::string& GetCompletionScreenTitleFont() const { return x324_; }
  const rstl::string& GetCompletionScreenBodyFont() const { return x334_; }
  const float& GetCompletionScreenPulseTime() const { return x35c_; }
  const float& GetCompletionScreenTextDelay() const { return x364_; }

  static float FaceReflectionDistanceDebugValueToActualValue(float v);
  static float FaceReflectionHeightDebugValueToActualValue(float v);
  static float FaceReflectionAspectDebugValueToActualValue(float v);
  static float FaceReflectionOrthoWidthDebugValueToActualValue(float v);
  static float FaceReflectionOrthoHeightDebugValueToActualValue(float v);

  bool x4_;
  float mMapAlphaInterp;
  float mPauseBlurFactor;
  float mRadarXYRadius;
  float x14_;
  float x18_;
  float x1c_;
  float x20_;
  float mRadarZRadius;
  float mRadarZCloseRadius;
  uint x2c_;
  float x30_;
  float mEnergyBarFilledSpeed;
  float mEnergyBarShadowSpeed;
  float mEnergyBarDrainDelay;
  bool mEnergyBarAlwaysResetDelay;
  float mHudDamagePracticalsGainConstant;
  float mHudDamagePracticalsGainLinear;
  float mHudDamagePracticalsInitConstant;
  float mHudDamagePracticalsInitLinear;
  float mHudDamageLightSpotAngle;
  float mDamageLightAngleC;
  float mDamageLightAngleL;
  float mDamageLightAngleQ;
  CVector3f mDamageLightPreTranslate;
  CVector3f mDamageLightCenterTranslate;
  float mDamageLightXfXAngle;
  float mDamageLightXfZAngle;
  float mHudDecoShakeTranslateVelConstant;
  float mHudDecoShakeTranslateVelLinear;
  float mMaxDecoDamageShakeTranslate;
  float mDecoDamageShakeDeceleration;
  float mDecoShakeGainConstant;
  float mDecoShakeGainLinear;
  float mDecoShakeInitConstant;
  float mDecoShakeInitLinear;
  float mMaxDecoDamageShakeRotate;
  uint mHudCamFovTweak;
  uint mHudCamYTweak;
  uint mHudCamZTweak;
  float xb4_;
  float xb8_;
  float xbc_;
  float mBeamVisorMenuAnimTime;
  float mVisorBeamMenuItemActiveScale;
  float mVisorBeamMenuItemInactiveScale;
  float mVisorBeamMenuItemTranslate;
  float xd0_;
  uint xd4_;
  float xd8_;
  float xdc_;
  float xe0_;
  float mThreatRange;
  float mRadarScopeCoordRadius;
  float mRadarPlayerPaintRadius;
  float mRadarEnemyPaintRadius;
  float mMissileArrowVisTime;
  EHudVisMode mHudVisMode;
  EHelmetVisMode mHelmetVisMode;
  uint mEnableAutoMapper;
  uint x104_;
  uint mEnableTargetingManager;
  uint mEnablePlayerVisor;
  float mThreatWarningFraction;
  float mMissileWarningFraction;
  float mFreeLookFadeTime;
  float x11c_;
  float x120_;
  float x124_;
  float x128_;
  float mFreeLookSfxPitchScale;
  bool mNoAbsoluteFreeLookSfxPitch;
  float x134_;
  float x138_;
  float mFaceReflectionOrthoWidth;
  float mFaceReflectionOrthoHeight;
  float mFaceReflectionDistance;
  float mFaceReflectionHeight;
  float mFaceReflectionAspect;
  rstl::string x150_;
  rstl::string x160_;
  rstl::string x170_;
  rstl::string x180_;
  rstl::string x190_;
  float mMissileWarningPulseTime;
  float mExplosionLightFalloffMultConstant;
  float mExplosionLightFalloffMultLinear;
  float mExplosionLightFalloffMultQuadratic;
  float x1b0_;
  float mHudDamagePeakFactor;
  float mHudDamageFilterGainConstant;
  float mHudDamageFilterGainLinear;
  float mHudDamageFilterInitConstant;
  float mHudDamageFilterInitLinear;
  float mEnergyDrainModPeriod;
  bool mEnergyDrainSinusoidalPulse;
  bool mEnergyDrainFilterAdditive;
  float mHudDamagePulseDuration;
  float mHudDamageColorGain;
  float mHudDecoShakeTranslateGain;
  float mHudLagOffsetScale;
  float x1e0_;
  float x1e4_;
  float x1e8_;
  float x1ec_;
  float x1f0_;
  float x1f4_;
  float x1f8_;
  float x1fc_;
  CColor x200_;
  float mXrayBlurScaleLinear;
  float mXrayBlurScaleQuadratic;
  float x20c_;
  float mScanSidesAngle;
  float mScanSidesXScale;
  float mScanSidesPositionEnd;
  float x21c_;
  float mScanSidesDuration;
  float mScanSidesStartTime;
  float mScanSidesEndTime;
  float mScanDataDotRadius;
  float mScanDataDotPosRandMag;
  float mScanDataDotSeekDurationMin;
  float mScanDataDotSeekDurationMax;
  float mScanDataDotHoldDurationMin;
  float mScanDataDotHoldDurationMax;
  float mScanAppearanceDuration;
  float mScanPaneFlashFactor;
  float mScanPaneFadeInTime;
  float mScanPaneFadeOutTime;
  float mBallViewportYReduction;
  float mScanWindowIdleW;
  float mScanWindowIdleH;
  float mScanWindowActiveW;
  float mScanWindowActiveH;
  float mScanWindowMagnification;
  float mScanWindowScanningAspect;
  float mScanSidesPositionStart;
  bool mShowAutomapperInMorphball;
  bool mLatchArticleText;
  float mWtMgrCharsPerSfx;
  uint mXrayFogMode;
  float mXrayFogNearZ;
  float mXrayFogFarZ;
  CColor mXrayFogColor;
  float mThermalVisorLevel;
  CColor mThermalVisorColor;
  CColor mCombatVisorHudLightAdd;
  CColor mXRayVisorHudLightAdd;
  CColor mScanVisorHudLightAdd;
  CColor mThermalVisorHudLightAdd;
  CColor mCombatVisorHudLightMultiply;
  CColor mXRayVisorHudLightMultiply;
  CColor mScanVisorHudLightMultiply;
  CColor mThermalVisorHudLightMultiply;
  CColor mHudReflectivityLightColor;
  float mHudLightAttMulConstant;
  float mHudLightAttMulLinear;
  float mHudLightAttMulQuadratic;
  rstl::reserved_vector< float, 2 > mScanSpeeds;
  rstl::string mCreditsTable;
  rstl::string mCreditsFont;
  rstl::string mJapaneseCreditsFont;
  CColor mCreditsTextFontColor;
  CColor mCreditsTextBorderColor;
  float x308_;
  float x30c_;
  float x310_;
  rstl::string x314_;
  rstl::string x324_;
  rstl::string x334_;
  CColor mCompletionTitleColor;
  CColor mCompletionTitleOutlineColor;
  CColor mCompletionBodyColor;
  CColor mCompletionBodyOutlineColor;
  CColor mCompletionUnlockColor;
  CColor mCompletionUnlockOutlineColor;
  float x35c_;
  float x360_;
  float x364_;
};

CHECK_SIZEOF(CTweakGui, 0x368)

extern CTweakGui* gpTweakGui;

#endif // _CTWEAKGUI
