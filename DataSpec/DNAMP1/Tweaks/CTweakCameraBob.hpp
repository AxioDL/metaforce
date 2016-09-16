#ifndef __URDE_CTWEAKCAMERABOB_HPP__
#define __URDE_CTWEAKCAMERABOB_HPP__

#include "../../DNACommon/DNACommon.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct CTweakCameraBob : BigYAML
{
    DECL_YAML
    Value<float> cameraBobExtentX;
    Value<float> cameraBobExtentY;
    Value<float> cameraBobPeriod;
    Value<float> orbitScale;
    Value<float> maxOrbitScale;
    Value<float> slowSpeedPeriodScale;
    Value<float> targetMagnitudeTrackingRate;
    Value<float> landingBobSpringConstant;
    Value<float> viewWanderRadius;
    Value<float> viewWanderSpeedMin;
    Value<float> viewWanderSpeedMax;
    Value<float> viewWanderRollVariation;
    Value<float> gunBobMagnitude;
    Value<float> helmetBobMagnitude;
};
}
}

#endif // __URDE_CTWEAKCAMERABOB_HPP__
