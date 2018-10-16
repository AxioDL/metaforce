#pragma once

#include "ITweak.hpp"
namespace DataSpec
{
struct ITweakTargeting : public ITweak
{
    virtual atUint32 GetTargetRadiusMode() const=0;
    virtual float GetCurrLockOnExitDuration() const=0;
    virtual float GetCurrLockOnEnterDuration() const=0;
    virtual float GetCurrLockOnSwitchDuration() const=0;
    virtual float GetLockConfirmScale() const=0;
    virtual float GetNextLockOnEnterDuration() const=0;
    virtual float GetNextLockOnExitDuration() const=0;
    virtual float GetNextLockOnSwitchDuration() const=0;
    virtual float GetSeekerScale() const=0;
    virtual float GetSeekerAngleSpeed() const=0;
    virtual float GetXRayRetAngleSpeed() const=0;
    virtual float GetOrbitPointZOffset() const=0;
    virtual float GetOrbitPointInTime() const=0;
    virtual float GetOrbitPointOutTime() const=0;
    virtual const zeus::CColor& GetThermalReticuleColor() const=0;
    virtual float GetTargetFlowerScale() const=0;
    virtual const zeus::CColor& GetTargetFlowerColor() const=0;
    virtual float GetMissileBracketDuration() const=0;
    virtual float GetMissileBracketScaleStart() const=0;
    virtual float GetMissileBracketScaleEnd() const=0;
    virtual float GetMissileBracketScaleDuration() const=0;
    virtual const zeus::CColor& GetMissileBracketColor() const=0;
    virtual float GetChargeGaugeOvershootOffset() const=0;
    virtual float GetChargeGaugeOvershootDuration() const=0;
    virtual float GetOuterBeamSquaresScale() const=0;
    virtual const zeus::CColor& GetOuterBeamSquareColor() const=0;
    virtual float GetLockonDuration() const=0;
    virtual float GetInnerBeamScale() const=0;
    virtual const zeus::CColor& GetInnerBeamColorPower() const=0;
    virtual const zeus::CColor& GetInnerBeamColorIce() const=0;
    virtual const zeus::CColor& GetInnerBeamColorWave() const=0;
    virtual const zeus::CColor& GetInnerBeamColorPlasma() const=0;
    virtual const float* GetOuterBeamSquareAngles(int i) const=0;
    virtual float GetChargeGaugeAngle(int i) const=0;
    virtual float GetChargeGaugeScale() const=0;
    virtual const zeus::CColor& GetChargeGaugeNonFullColor() const=0;
    virtual atUint32 GetChargeTickCount() const=0;
    virtual float GetChargeTickAnglePitch() const=0;
    virtual float GetLockFireScale() const=0;
    virtual float GetLockFireDuration() const=0;
    virtual const zeus::CColor& GetLockFireColor() const=0;
    virtual float GetLockDaggerScaleStart() const=0;
    virtual float GetLockDaggerScaleEnd() const=0;
    virtual const zeus::CColor& GetLockDaggerColor() const=0;
    virtual float GetLockDaggerAngle0() const=0;
    virtual float GetLockDaggerAngle1() const=0;
    virtual float GetLockDaggerAngle2() const=0;
    virtual const zeus::CColor& GetLockConfirmColor() const=0;
    virtual const zeus::CColor& GetSeekerColor() const=0;
    virtual float GetLockConfirmClampMin() const=0;
    virtual float GetLockConfirmClampMax() const=0;
    virtual float GetTargetFlowerClampMin() const=0;
    virtual float GetTargetFlowerClampMax() const=0;
    virtual float GetSeekerClampMin() const=0;
    virtual float GetSeekerClampMax() const=0;
    virtual float GetMissileBracketClampMin() const=0;
    virtual float GetMissileBracketClampMax() const=0;
    virtual float GetInnerBeamClampMin() const=0;
    virtual float GetInnerBeamClampMax() const=0;
    virtual float GetChargeGaugeClampMin() const=0;
    virtual float GetChargeGaugeClampMax() const=0;
    virtual float GetLockFireClampMin() const=0;
    virtual float GetLockFireClampMax() const=0;
    virtual float GetLockDaggerClampMin() const=0;
    virtual float GetLockDaggerClampMax() const=0;
    virtual float GetGrappleSelectScale() const=0;
    virtual float GetGrappleScale() const=0;
    virtual float GetGrappleClampMin() const=0;
    virtual float GetGrappleClampMax() const=0;
    virtual const zeus::CColor& GetGrapplePointSelectColor() const=0;
    virtual const zeus::CColor& GetGrapplePointColor() const=0;
    virtual const zeus::CColor& GetLockedGrapplePointSelectColor() const=0;
    virtual float GetGrappleMinClampScale() const=0;
    virtual const zeus::CColor& GetChargeGaugePulseColorHigh() const=0;
    virtual float GetFullChargeFadeDuration() const=0;
    virtual const zeus::CColor& GetOrbitPointColor() const=0;
    virtual const zeus::CColor& GetCrosshairsColor() const=0;
    virtual float GetCrosshairsScaleDuration() const=0;
    virtual bool DrawOrbitPoint() const=0;
    virtual const zeus::CColor& GetChargeGaugePulseColorLow() const=0;
    virtual float GetChargeGaugePulsePeriod() const=0;
    virtual float GetReticuleClampMin() const=0;
    virtual float GetReticuleClampMax() const=0;
    virtual const zeus::CColor& GetXRayRetRingColor() const=0;
    virtual float GetReticuleScale() const=0;
    virtual float GetScanTargetClampMin() const=0;
    virtual float GetScanTargetClampMax() const=0;
    virtual float GetAngularLagSpeed() const=0;
};
}

