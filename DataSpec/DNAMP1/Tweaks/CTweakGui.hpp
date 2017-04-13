#ifndef _DNAMP1_CTWEAKGUI_HPP_
#define _DNAMP1_CTWEAKGUI_HPP_

#include "../../DNACommon/Tweaks/ITweakGui.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct CTweakGui : ITweakGui
{
    DECL_YAML
    Value<bool> x4_;
    Value<float> x8_mapAlphaInterp;
    Value<float> xc_;
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
    Value<atUint32> x13c_;
    Value<atUint32> x140_;
    Value<atUint32> x144_faceReflectionDistance;
    Value<atUint32> x148_faceReflectionHeight;
    Value<atUint32> x14c_;
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
    Value<float> x1c8_;
    Value<bool> x1cc_;
    Value<bool> x1cd_;
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
    float x200_;
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
    Value<float> x22c_;
    Value<float> x230_;
    Value<float> x234_;
    Value<float> x238_;
    Value<float> x23c_;
    Value<float> x240_;
    Value<float> x244_scanAppearanceOffset;
    Value<float> x248_;
    Value<float> x24c_;
    Value<float> x250_;
    Value<float> x254_ballViewportYReduction;
    Value<float> x258_;
    Value<float> x25c_;
    Value<float> x260_;
    Value<float> x264_;
    Value<float> x268_;
    Value<float> x26c_;
    Value<float> x270_scanSidesPositionStart;
    Value<bool> x274_;
    bool x275_ = true;
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
    Vector<float, DNA_COUNT(m_scanSpeedsCount)> x2c4_scanSpeeds;
    String<-1> x2d0_;
    String<-1> x2e0_;
    String<-1> x2f0_;
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
    CTweakGui(athena::io::IStreamReader& r) { this->read(r); }

    float GetMapAlphaInterpolant() const { return x8_mapAlphaInterp; }
    float GetRadarXYRadius() const { return x10_radarXYRadius; }
    float GetRadarZRadius() const { return x24_radarZRadius; }
    float GetRadarZCloseRadius() const { return x28_radarZCloseRadius; }
    float GetEnergyBarFilledSpeed() const { return x34_energyBarFilledSpeed; }
    float GetEnergyBarShadowSpeed() const { return x38_energyBarShadowSpeed; }
    float GetEnergyBarDrainDelay() const { return x3c_energyBarDrainDelay; }
    bool GetEnergyBarAlwaysResetDelay() const { return x40_energyBarAlwaysResetDelay; }
    float GetHudDamagePracticalsGainConstant() const { return x44_hudDamagePracticalsGainConstant; }
    float GetHudDamagePracticalsGainLinear() const { return x48_hudDamagePracticalsGainLinear; }
    float GetHudDamagePracticalsInitConstant() const { return x4c_hudDamagePracticalsInitConstant; }
    float GetHudDamagePracticalsInitLinear() const { return x50_hudDamagePracticalsInitLinear; }
    float GetHudDamageLightSpotAngle() const { return x54_hudDamageLightSpotAngle; }
    float GetDamageLightAngleC() const { return x58_damageLightAngleC; }
    float GetDamageLightAngleL() const { return x5c_damageLightAngleL; }
    float GetDamageLightAngleQ() const { return x60_damageLightAngleQ; }
    atVec3f GetDamageLightPreTranslate() const { return x64_damageLightPreTranslate; }
    atVec3f GetDamageLightCenterTranslate() const { return x70_damageLightCenterTranslate; }
    float GetDamageLightXfXAngle() const { return x7c_damageLightXfXAngle; }
    float GetDamageLightXfZAngle() const { return x80_damageLightXfZAngle; }
    float GetHudDecoShakeTranslateVelConstant() const { return x84_hudDecoShakeTranslateVelConstant; }
    float GetHudDecoShakeTranslateVelLinear() const { return x88_hudDecoShakeTranslateVelLinear; }
    float GetMaxDecoDamageShakeTranslate() const { return x8c_maxDecoDamageShakeTranslate; }
    float GetDecoDamageShakeDeceleration() const { return x90_decoDamageShakeDeceleration; }
    float GetDecoShakeGainConstant() const { return x94_decoShakeGainConstant; }
    float GetDecoShakeGainLinear() const { return x98_decoShakeGainLinear; }
    float GetDecoShakeInitConstant() const { return x9c_decoShakeInitConstant; }
    float GetDecoShakeInitLinear() const { return xa0_decoShakeInitLinear; }
    float GetMaxDecoDamageShakeRotate() const { return xa4_maxDecoDamageShakeRotate; }
    atUint32 GetHudCamFovTweak() const { return xa8_hudCamFovTweak; }
    atUint32 GetHudCamYTweak() const { return xac_hudCamYTweak; }
    atUint32 GetHudCamZTweak() const { return xb0_hudCamZTweak; }
    float GetBeamVisorMenuAnimTime() const { return xc0_beamVisorMenuAnimTime; }
    float GetVisorBeamMenuItemActiveScale() const { return xc4_visorBeamMenuItemActiveScale; }
    float GetVisorBeamMenuItemInactiveScale() const { return xc8_visorBeamMenuItemInactiveScale; }
    float GetVisorBeamMenuItemTranslate() const { return xcc_visorBeamMenuItemTranslate; }
    float GetThreatRange() const { return xe4_threatRange; }
    float GetRadarScopeCoordRadius() const { return xe8_radarScopeCoordRadius; }
    float GetRadarPlayerPaintRadius() const { return xec_radarPlayerPaintRadius; }
    float GetRadarEnemyPaintRadius() const { return xf0_radarEnemyPaintRadius; }
    float GetMissileArrowVisTime() const { return xf4_missileArrowVisTime; }
    EHudVisMode GetHudVisMode() const { return xf8_hudVisMode; }
    EHelmetVisMode GetHelmetVisMode() const { return xfc_helmetVisMode; }
    atUint32 GetEnableAutoMapper() const { return x100_enableAutoMapper; }
    atUint32 GetEnableTargetingManager() const { return x108_enableTargetingManager; }
    atUint32 GetEnablePlayerVisor() const { return x10c_enablePlayerVisor; }
    float GetThreatWarningFraction() const { return x110_threatWarningFraction; }
    float GetMissileWarningFraction() const { return x114_missileWarningFraction; }
    float GetFreeLookFadeTime() const { return x118_freeLookFadeTime; }
    float GetFreeLookSfxPitchScale() const { return x12c_freeLookSfxPitchScale; }
    bool GetNoAbsoluteFreeLookSfxPitch() const { return x130_noAbsoluteFreeLookSfxPitch; }
    float GetFaceReflectionDistance() const { return x144_faceReflectionDistance; }
    float GetFaceReflectionHeight() const { return x148_faceReflectionHeight; }
    float GetMissileWarningPulseTime() const { return x1a0_missileWarningPulseTime; }
    float GetExplosionLightFalloffMultConstant() const { return x1a4_explosionLightFalloffMultConstant; }
    float GetExplosionLightFalloffMultLinear() const { return x1a8_explosionLightFalloffMultLinear; }
    float GetExplosionLightFalloffMultQuadratic() const { return x1ac_explosionLightFalloffMultQuadratic; }
    float GetHudDamagePeakFactor() const { return x1b4_hudDamagePeakFactor; }
    float GetHudDamageFilterGainConstant() const { return x1b8_hudDamageFilterGainConstant; }
    float GetHudDamageFilterGainLinear() const { return x1bc_hudDamageFilterGainLinear; }
    float GetHudDamageFilterInitConstant() const { return x1c0_hudDamageFilterInitConstant; }
    float GetHudDamageFilterInitLinear() const { return x1c4_hudDamageFilterInitLinear; }
    float GetHudDamagePulseDuration() const { return x1d0_hudDamagePulseDuration; }
    float GetHudDamageColorGain() const { return x1d4_hudDamageColorGain; }
    float GetHudDecoShakeTranslateGain() const { return x1d8_hudDecoShakeTranslateGain; }
    float GetHudLagOffsetScale() const { return x1dc_hudLagOffsetScale; }
    float GetScanAppearanceOffset() const { return x244_scanAppearanceOffset; }
    float GetBallViewportYReduction() const { return x254_ballViewportYReduction; }
    float GetXrayBlurScaleLinear() const { return x204_xrayBlurScaleLinear; }
    float GetXrayBlurScaleQuadratic() const { return x208_xrayBlurScaleQuadratic; }
    float GetScanSidesAngle() const { return x210_scanSidesAngle; }
    float GetScanSidesXScale() const { return x214_scanSidesXScale; }
    float GetScanSidesPositionEnd() const { return x218_scanSidesPositionEnd; }
    float GetScanSidesDuration() const { return x220_scanSidesDuration; }
    float GetScanSidesStartTime() const { return x224_scanSidesStartTime; }
    float GetScanSidesEndTime() const { return x228_scanSidesEndTime; }
    float GetScanSidesPositionStart() const { return x270_scanSidesPositionStart; }
    float GetWorldTransManagerCharsPerSfx() const { return x278_wtMgrCharsPerSfx; }
    atUint32 GetXRayFogMode() const { return x27c_xrayFogMode; }
    float GetXRayFogNearZ() const { return x280_xrayFogNearZ; }
    float GetXRayFogFarZ() const { return x284_xrayFogFarZ; }
    const zeus::CColor& GetXRayFogColor() const { return x288_xrayFogColor; }
    float GetThermalVisorLevel() const { return x28c_thermalVisorLevel; }
    const zeus::CColor& GetThermalVisorColor() const { return x290_thermalVisorColor; }
    const zeus::CColor& GetVisorHudLightAdd(int v) const { return x294_hudLightAddPerVisor[v]; }
    const zeus::CColor& GetVisorHudLightMultiply(int v) const { return x2a4_hudLightMultiplyPerVisor[v]; }
    const zeus::CColor& GetHudReflectivityLightColor() const { return x2b4_hudReflectivityLightColor; }
    float GetHudLightAttMulConstant() const { return x2b8_hudLightAttMulConstant; }
    float GetHudLightAttMulLinear() const { return x2bc_hudLightAttMulLinear; }
    float GetHudLightAttMulQuadratic() const { return x2c0_hudLightAttMulQuadratic; }

    float GetScanSpeed(int idx) const
    {
        if (idx < 0 || idx >= x2c4_scanSpeeds.size())
            return 0.f;
        return x2c4_scanSpeeds[idx];
    }

    void FixupValues()
    {
        xd8_ = zeus::degToRad(xd8_);
        xdc_ = zeus::degToRad(xdc_);

        x200_ = x1f4_ * 0.25f;
        x204_xrayBlurScaleLinear = x1f8_ * 0.25f;
        x208_xrayBlurScaleQuadratic = x1fc_ * 0.25f;

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

}
}

#endif // _DNAMP1_CTWEAKGUI_HPP_
