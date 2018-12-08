#pragma once

#include "../../DNACommon/DNACommon.hpp"

namespace DataSpec::DNAMP1 {
struct CTweakCameraBob : BigDNA {
  AT_DECL_DNA_YAML
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
} // namespace DataSpec::DNAMP1
