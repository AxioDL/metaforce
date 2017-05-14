#ifndef __DNACOMMON_ITWEAKGUI_HPP__
#define __DNACOMMON_ITWEAKGUI_HPP__

#include "ITweak.hpp"
#include "zeus/CVector2f.hpp"

namespace DataSpec
{

struct ITweakGui : ITweak
{
    enum class EHudVisMode : atUint32
    {
        Zero,
        One,
        Two,
        Three
    };

    enum class EHelmetVisMode : atUint32
    {
        ReducedUpdate,
        NotVisible,
        Deco,
        HelmetDeco,
        GlowHelmetDeco,
        HelmetOnly
    };

    virtual float GetMapAlphaInterpolant() const=0;
    virtual float GetRadarXYRadius() const=0;
    virtual float GetRadarZRadius() const=0;
    virtual float GetRadarZCloseRadius() const=0;
    virtual float GetEnergyBarFilledSpeed() const=0;
    virtual float GetEnergyBarShadowSpeed() const=0;
    virtual float GetEnergyBarDrainDelay() const=0;
    virtual bool GetEnergyBarAlwaysResetDelay() const=0;
    virtual float GetHudDamagePracticalsGainConstant() const=0;
    virtual float GetHudDamagePracticalsGainLinear() const=0;
    virtual float GetHudDamagePracticalsInitConstant() const=0;
    virtual float GetHudDamagePracticalsInitLinear() const=0;
    virtual float GetHudDamageLightSpotAngle() const=0;
    virtual float GetDamageLightAngleC() const=0;
    virtual float GetDamageLightAngleL() const=0;
    virtual float GetDamageLightAngleQ() const=0;
    virtual atVec3f GetDamageLightPreTranslate() const=0;
    virtual atVec3f GetDamageLightCenterTranslate() const=0;
    virtual float GetDamageLightXfXAngle() const=0;
    virtual float GetDamageLightXfZAngle() const=0;
    virtual float GetHudDecoShakeTranslateVelConstant() const=0;
    virtual float GetHudDecoShakeTranslateVelLinear() const=0;
    virtual float GetMaxDecoDamageShakeTranslate() const=0;
    virtual float GetDecoDamageShakeDeceleration() const=0;
    virtual float GetDecoShakeGainConstant() const=0;
    virtual float GetDecoShakeGainLinear() const=0;
    virtual float GetDecoShakeInitConstant() const=0;
    virtual float GetDecoShakeInitLinear() const=0;
    virtual float GetMaxDecoDamageShakeRotate() const=0;
    virtual atUint32 GetHudCamFovTweak() const=0;
    virtual atUint32 GetHudCamYTweak() const=0;
    virtual atUint32 GetHudCamZTweak() const=0;
    virtual float GetBeamVisorMenuAnimTime() const=0;
    virtual float GetVisorBeamMenuItemActiveScale() const=0;
    virtual float GetVisorBeamMenuItemInactiveScale() const=0;
    virtual float GetVisorBeamMenuItemTranslate() const=0;
    virtual float GetThreatRange() const=0;
    virtual float GetRadarScopeCoordRadius() const=0;
    virtual float GetRadarPlayerPaintRadius() const=0;
    virtual float GetRadarEnemyPaintRadius() const=0;
    virtual float GetMissileArrowVisTime() const=0;
    virtual EHudVisMode GetHudVisMode() const=0;
    virtual EHelmetVisMode GetHelmetVisMode() const=0;
    virtual atUint32 GetEnableAutoMapper() const=0;
    virtual atUint32 GetEnableTargetingManager() const=0;
    virtual atUint32 GetEnablePlayerVisor() const=0;
    virtual float GetThreatWarningFraction() const=0;
    virtual float GetMissileWarningFraction() const=0;
    virtual float GetFreeLookFadeTime() const=0;
    virtual float GetFreeLookSfxPitchScale() const=0;
    virtual bool GetNoAbsoluteFreeLookSfxPitch() const=0;
    virtual float GetFaceReflectionDistance() const=0;
    virtual float GetFaceReflectionHeight() const=0;
    virtual float GetMissileWarningPulseTime() const=0;
    virtual float GetExplosionLightFalloffMultConstant() const=0;
    virtual float GetExplosionLightFalloffMultLinear() const=0;
    virtual float GetExplosionLightFalloffMultQuadratic() const=0;
    virtual float GetHudDamagePeakFactor() const=0;
    virtual float GetHudDamageFilterGainConstant() const=0;
    virtual float GetHudDamageFilterGainLinear() const=0;
    virtual float GetHudDamageFilterInitConstant() const=0;
    virtual float GetHudDamageFilterInitLinear() const=0;
    virtual float GetHudDamagePulseDuration() const=0;
    virtual float GetHudDamageColorGain() const=0;
    virtual float GetHudDecoShakeTranslateGain() const=0;
    virtual float GetHudLagOffsetScale() const=0;
    virtual float GetScanAppearanceOffset() const=0;
    virtual float GetBallViewportYReduction() const=0;
    virtual float GetScanSpeed(int idx) const=0;
    virtual float GetXrayBlurScaleLinear() const=0;
    virtual float GetXrayBlurScaleQuadratic() const=0;
    virtual float GetScanSidesAngle() const=0;
    virtual float GetScanSidesXScale() const=0;
    virtual float GetScanSidesPositionEnd() const=0;
    virtual float GetScanSidesDuration() const=0;
    virtual float GetScanSidesStartTime() const=0;
    virtual float GetScanSidesEndTime() const=0;
    virtual float GetScanSidesPositionStart() const=0;
    virtual bool GetLatchArticleText() const=0;
    virtual float GetWorldTransManagerCharsPerSfx() const=0;
    virtual atUint32 GetXRayFogMode() const=0;
    virtual float GetXRayFogNearZ() const=0;
    virtual float GetXRayFogFarZ() const=0;
    virtual const zeus::CColor& GetXRayFogColor() const=0;
    virtual float GetThermalVisorLevel() const=0;
    virtual const zeus::CColor& GetThermalVisorColor() const=0;
    virtual const zeus::CColor& GetVisorHudLightAdd(int v) const=0;
    virtual const zeus::CColor& GetVisorHudLightMultiply(int v) const=0;
    virtual const zeus::CColor& GetHudReflectivityLightColor() const=0;
    virtual float GetHudLightAttMulConstant() const=0;
    virtual float GetHudLightAttMulLinear() const=0;
    virtual float GetHudLightAttMulQuadratic() const=0;

    static float FaceReflectionDistanceDebugValueToActualValue(float v) { return 0.015f * v + 0.2f; }
    static float FaceReflectionHeightDebugValueToActualValue(float v) { return 0.005f * v - 0.05f; }
};

}

#endif // __DNACOMMON_ITWEAKGUI_HPP__
