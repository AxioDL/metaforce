#ifndef __DNACOMMON_ITWEAKTARGETING_HPP__
#define __DNACOMMON_ITWEAKTARGETING_HPP__

#include "ITweak.hpp"
namespace DataSpec
{
struct ITweakTargeting : public ITweak
{
    virtual atUint32 GetTargetRadiusMode() const=0;
    virtual float GetSeekerScale() const=0;
    virtual float GetSeekerAngleSpeed() const=0;
    virtual float GetXRayRetAngleSpeed() const=0;
    virtual float GetOrbitPointZOffset() const=0;
    virtual float GetOrbitPointInTime() const=0;
    virtual float GetOrbitPointOutTime() const=0;
    virtual const zeus::CColor& GetThermalReticuleColor() const=0;
    virtual float GetOvershootOffset() const=0;
    virtual float GetXD0() const=0;
    virtual const zeus::CColor& GetSeekerColor() const=0;
    virtual float GetSeekerClampMin() const=0;
    virtual float GetSeekerClampMax() const=0;
    virtual float GetGrappleSelectScale() const=0;
    virtual float GetGrappleScale() const=0;
    virtual float GetGrappleClampMin() const=0;
    virtual float GetGrappleClampMax() const=0;
    virtual const zeus::CColor& GetGrapplePointSelectColor() const=0;
    virtual const zeus::CColor& GetGrapplePointColor() const=0;
    virtual const zeus::CColor& GetLockedGrapplePointSelectColor() const=0;
    virtual const zeus::CColor& GetOrbitPointColor() const=0;
    virtual const zeus::CColor& GetCrosshairsColor() const=0;
    virtual bool DrawOrbitPoint() const=0;
    virtual float GetReticuleClampMin() const=0;
    virtual float GetReticuleClampMax() const=0;
    virtual const zeus::CColor& GetXRayRetRingColor() const=0;
    virtual float GetReticuleScale() const=0;
    virtual float GetScanTargetClampMin() const=0;
    virtual float GetScanTargetClampMax() const=0;
};
}

#endif // __DNACOMMON_ITWEAKTARGETING_HPP__
