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
    virtual float GetEnergyBarFilledSpeed() const=0;
    virtual float GetEnergyBarShadowSpeed() const=0;
    virtual float GetEnergyBarDrainDelay() const=0;
    virtual bool GetEnergyBarAlwaysResetDelay() const=0;
    virtual atUint32 GetHudCamFovTweak() const=0;
    virtual atUint32 GetHudCamYTweak() const=0;
    virtual atUint32 GetHudCamZTweak() const=0;
    virtual EHudVisMode GetHudVisMode() const=0;
    virtual EHelmetVisMode GetHelmetVisMode() const=0;
    virtual atUint32 GetEnableAutoMapper() const=0;
    virtual atUint32 GetEnableTargetingManager() const=0;
    virtual atUint32 GetEnablePlayerVisor() const=0;
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
    virtual float GetWorldTransManagerCharsPerSfx() const=0;
    virtual atUint32 GetXRayFogMode() const=0;
    virtual float GetXRayFogNearZ() const=0;
    virtual float GetXRayFogFarZ() const=0;
    virtual const zeus::CColor& GetXRayFogColor() const=0;
    virtual float GetThermalVisorLevel() const=0;
    virtual const zeus::CColor& GetThermalVisorColor() const=0;
};

}

#endif // __DNACOMMON_ITWEAKGUI_HPP__
