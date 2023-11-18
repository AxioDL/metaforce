#include "Runtime/MP1/Tweaks/CTweakGui.hpp"
#include "Runtime/Streams/IOStreams.hpp"

namespace metaforce::MP1 {
CTweakGui::CTweakGui(CInputStream& in)
: x4_(in.ReadBool())
, x8_mapAlphaInterp(in.ReadFloat())
, xc_pauseBlurFactor(in.ReadFloat())
, x10_radarXYRadius(in.ReadFloat())
, x14_(in.ReadFloat())
, x18_(in.ReadFloat())
, x1c_(in.ReadFloat())
, x20_(in.ReadFloat())
, x24_radarZRadius(in.ReadFloat())
, x28_radarZCloseRadius(in.ReadFloat())
, x30_(in.ReadFloat())
, x34_energyBarFilledSpeed(in.ReadFloat())
, x38_energyBarShadowSpeed(in.ReadFloat())
, x3c_energyBarDrainDelay(in.ReadFloat())
, x40_energyBarAlwaysResetDelay(in.ReadBool())
, x44_hudDamagePracticalsGainConstant(in.ReadFloat())
, x48_hudDamagePracticalsGainLinear(in.ReadFloat())
, x4c_hudDamagePracticalsInitConstant(in.ReadFloat())
, x50_hudDamagePracticalsInitLinear(in.ReadFloat())
, x54_hudDamageLightSpotAngle(in.ReadFloat())
, x58_damageLightAngleC(in.ReadFloat())
, x5c_damageLightAngleL(in.ReadFloat())
, x60_damageLightAngleQ(in.ReadFloat())
, x64_damageLightPreTranslate(in.Get<zeus::CVector3f>())
, x70_damageLightCenterTranslate(in.Get<zeus::CVector3f>())
, x7c_damageLightXfXAngle(in.ReadFloat())
, x80_damageLightXfZAngle(in.ReadFloat())
, x84_hudDecoShakeTranslateVelConstant(in.ReadFloat())
, x88_hudDecoShakeTranslateVelLinear(in.ReadFloat())
, x8c_maxDecoDamageShakeTranslate(in.ReadFloat())
, x90_decoDamageShakeDeceleration(in.ReadFloat())
, x94_decoShakeGainConstant(in.ReadFloat())
, x98_decoShakeGainLinear(in.ReadFloat())
, x9c_decoShakeInitConstant(in.ReadFloat())
, xa0_decoShakeInitLinear(in.ReadFloat())
, xa4_maxDecoDamageShakeRotate(in.ReadFloat())
, xa8_hudCamFovTweak(in.ReadLong())
, xac_hudCamYTweak(in.ReadLong())
, xb0_hudCamZTweak(in.ReadLong())
, xb4_(in.ReadFloat())
, xb8_(in.ReadFloat())
, xbc_(in.ReadFloat())
, xc0_beamVisorMenuAnimTime(in.ReadFloat())
, xc4_visorBeamMenuItemActiveScale(in.ReadFloat())
, xc8_visorBeamMenuItemInactiveScale(in.ReadFloat())
, xcc_visorBeamMenuItemTranslate(in.ReadFloat())
, xd0_(in.ReadFloat())
, xd4_(in.ReadLong())
, xd8_(in.ReadFloat())
, xdc_(in.ReadFloat())
, xe0_(in.ReadFloat())
, xe4_threatRange(in.ReadFloat())
, xe8_radarScopeCoordRadius(in.ReadFloat())
, xec_radarPlayerPaintRadius(in.ReadFloat())
, xf0_radarEnemyPaintRadius(in.ReadFloat())
, xf4_missileArrowVisTime(in.ReadFloat())
, xf8_hudVisMode(EHudVisMode(in.ReadLong()))
, xfc_helmetVisMode(EHelmetVisMode(in.ReadLong()))
, x100_enableAutoMapper(in.ReadLong())
, x104_(in.ReadLong())
, x108_enableTargetingManager(in.ReadLong())
, x10c_enablePlayerVisor(in.ReadLong())
, x110_threatWarningFraction(in.ReadFloat())
, x114_missileWarningFraction(in.ReadFloat())
, x118_freeLookFadeTime(in.ReadFloat())
, x11c_(in.ReadFloat())
, x120_(in.ReadFloat())
, x124_(in.ReadFloat())
, x128_(in.ReadFloat())
, x12c_freeLookSfxPitchScale(in.ReadFloat())
, x130_noAbsoluteFreeLookSfxPitch(in.ReadBool())
, x134_(in.ReadFloat())
, x138_(in.ReadFloat())
, x13c_faceReflectionOrthoWidth(in.ReadLong())
, x140_faceReflectionOrthoHeight(in.ReadLong())
, x144_faceReflectionDistance(in.ReadLong())
, x148_faceReflectionHeight(in.ReadLong())
, x14c_faceReflectionAspect(in.ReadLong())
, x150_(in.Get<std::string>())
, x160_(in.Get<std::string>())
, x170_(in.Get<std::string>())
, x180_(in.Get<std::string>())
, x190_(in.Get<std::string>())
, x1a0_missileWarningPulseTime(in.ReadFloat())
, x1a4_explosionLightFalloffMultConstant(in.ReadFloat())
, x1a8_explosionLightFalloffMultLinear(in.ReadFloat())
, x1ac_explosionLightFalloffMultQuadratic(in.ReadFloat())
, x1b0_(in.ReadFloat())
, x1b4_hudDamagePeakFactor(in.ReadFloat())
, x1b8_hudDamageFilterGainConstant(in.ReadFloat())
, x1bc_hudDamageFilterGainLinear(in.ReadFloat())
, x1c0_hudDamageFilterInitConstant(in.ReadFloat())
, x1c4_hudDamageFilterInitLinear(in.ReadFloat())
, x1c8_energyDrainModPeriod(in.ReadFloat())
, x1cc_energyDrainSinusoidalPulse(in.ReadBool())
, x1cd_energyDrainFilterAdditive(in.ReadBool())
, x1d0_hudDamagePulseDuration(in.ReadFloat())
, x1d4_hudDamageColorGain(in.ReadFloat())
, x1d8_hudDecoShakeTranslateGain(in.ReadFloat())
, x1dc_hudLagOffsetScale(in.ReadFloat())
, x1e0_(in.ReadFloat())
, x1e4_(in.ReadFloat())
, x1e8_(in.ReadFloat())
, x1ec_(in.ReadFloat())
, x1f0_(in.ReadFloat())
, x1f4_(in.ReadFloat())
, x1f8_(in.ReadFloat())
, x1fc_(in.ReadFloat())
, x20c_(in.ReadFloat())
, x210_scanSidesAngle(in.ReadFloat())
, x214_scanSidesXScale(in.ReadFloat())
, x218_scanSidesPositionEnd(in.ReadFloat())
, x21c_(in.ReadFloat())
, x220_scanSidesDuration(in.ReadFloat())
, x224_scanSidesStartTime(in.ReadFloat())
, x22c_scanDataDotRadius(in.ReadFloat())
, x230_scanDataDotPosRandMag(in.ReadFloat())
, x234_scanDataDotSeekDurationMin(in.ReadFloat())
, x238_scanDataDotSeekDurationMax(in.ReadFloat())
, x23c_scanDataDotHoldDurationMin(in.ReadFloat())
, x240_scanDataDotHoldDurationMax(in.ReadFloat())
, x244_scanAppearanceDuration(in.ReadFloat())
, x248_scanPaneFlashFactor(in.ReadFloat())
, x24c_scanPaneFadeInTime(in.ReadFloat())
, x250_scanPaneFadeOutTime(in.ReadFloat())
, x254_ballViewportYReduction(in.ReadFloat())
, x258_scanWindowIdleW(in.ReadFloat())
, x25c_scanWindowIdleH(in.ReadFloat())
, x260_scanWindowActiveW(in.ReadFloat())
, x264_scanWindowActiveH(in.ReadFloat())
, x268_scanWindowMagnification(in.ReadFloat())
, x26c_scanWindowScanningAspect(in.ReadFloat())
, x270_scanSidesPositionStart(in.ReadFloat())
, x274_showAutomapperInMorphball(in.ReadBool())
, x278_wtMgrCharsPerSfx(in.ReadFloat())
, x27c_xrayFogMode(in.ReadLong())
, x280_xrayFogNearZ(in.ReadFloat())
, x284_xrayFogFarZ(in.ReadFloat())
, x288_xrayFogColor(in.Get<zeus::CColor>())
, x28c_thermalVisorLevel(in.ReadFloat())
, x290_thermalVisorColor(in.Get<zeus::CColor>()) {
  for (u32 i = 0; i < 4; ++i) {
    x294_hudLightAddPerVisor[i] = in.Get<zeus::CColor>();
  }
  for (u32 i = 0; i < 4; ++i) {
    x2a4_hudLightMultiplyPerVisor[i] = in.Get<zeus::CColor>();
  }

  x2b4_hudReflectivityLightColor = in.Get<zeus::CColor>();
  x2b8_hudLightAttMulConstant = in.ReadFloat();
  x2bc_hudLightAttMulLinear = in.ReadFloat();
  x2c0_hudLightAttMulQuadratic = in.ReadFloat();
  read_reserved_vector(x2c4_scanSpeeds, in);
  x2d0_creditsTable = in.Get<std::string>();
  x2e0_creditsFont = in.Get<std::string>();
  x2f0_japaneseCreditsFont = in.Get<std::string>();
  x300_ = in.Get<zeus::CColor>();
  x304_ = in.Get<zeus::CColor>();
  x308_ = in.ReadFloat();
  x30c_ = in.ReadFloat();
  x310_ = in.ReadFloat();
  x314_ = in.Get<std::string>();
  x324_ = in.Get<std::string>();
  x334_ = in.Get<std::string>();
  x344_ = in.Get<zeus::CColor>();
  x348_ = in.Get<zeus::CColor>();
  x34c_ = in.Get<zeus::CColor>();
  x350_ = in.Get<zeus::CColor>();
  x354_ = in.Get<zeus::CColor>();
  x358_ = in.Get<zeus::CColor>();
  x35c_ = in.ReadFloat();
  x360_ = in.ReadFloat();
  x364_ = in.ReadFloat();

  FixupValues();
}

} // namespace metaforce::MP1