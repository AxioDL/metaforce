#include "Runtime/World/CFluidUVMotion.hpp"

#include <cmath>

#include <zeus/Math.hpp>

namespace urde {

CFluidUVMotion::CFluidUVMotion(float timeToWrap, float orientation, const SFluidLayerMotion& colorLayer,
                               const SFluidLayerMotion& pattern1Layer, const SFluidLayerMotion& pattern2Layer)
: x0_fluidLayers{{colorLayer, pattern1Layer, pattern2Layer}}
, x4c_ooTimeToWrap(1.f / timeToWrap)
, x50_orientation(orientation) {}

CFluidUVMotion::CFluidUVMotion(float timeToWrap, float orientation)
: x4c_ooTimeToWrap(1.f / timeToWrap), x50_orientation(orientation) {
  x0_fluidLayers.resize(3);
  x0_fluidLayers[0].x4_ooTimeToWrap = 0.001f;
  x0_fluidLayers[1].x4_ooTimeToWrap = 0.33333334f;
  x0_fluidLayers[2].x4_ooTimeToWrap = 0.2f;
  x0_fluidLayers[2].x8_orientation = 0.78539819f;
}

CFluidUVMotion::FluidOffsets CFluidUVMotion::CalculateFluidTextureOffset(float t) const {
  FluidOffsets offsets;
  const float totalYOffset = t * x4c_ooTimeToWrap * std::cos(x50_orientation);
  const float totalXOffset = t * x4c_ooTimeToWrap * std::sin(x50_orientation);

  for (size_t i = 0; i < x0_fluidLayers.size(); ++i) {
    const SFluidLayerMotion& layer = x0_fluidLayers[i];

    const float speedT = t * layer.x4_ooTimeToWrap;
    const float cycleT = speedT - std::floor(speedT);
    float localY;
    float localX;
    switch (layer.x0_motion) {
    case EFluidUVMotion::Linear: {
      localX = speedT;
      localY = 0.f;
    } break;
    case EFluidUVMotion::Circular: {
      const float angle = (M_PIF * 2) * cycleT;
      localY = layer.xc_magnitude * std::sin(angle);
      localX = layer.xc_magnitude * std::cos(angle);
    } break;
    case EFluidUVMotion::Oscillate: {
      localY = 0.f;
      localX = layer.xc_magnitude * std::cos((M_PIF * 2) * cycleT);
    } break;
    default:
      localY = localX = 0.f;
      break;
    }

    const float x = localX * std::sin(layer.x8_orientation) + localY * std::cos(layer.x8_orientation) + totalXOffset;
    const float y = localY * std::sin(layer.x8_orientation) + localX * std::cos(layer.x8_orientation) + totalYOffset;

    offsets[i][0] = x - std::floor(x);
    offsets[i][1] = y - std::floor(y);
  }

  return offsets;
}
} // namespace urde
