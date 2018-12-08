#pragma once

#include "rstl.hpp"
#include "RetroTypes.hpp"

namespace urde {
class CFluidUVMotion {
public:
  enum class EFluidUVMotion {
    Linear,
    Circular,
    Oscillate,
  };

  struct SFluidLayerMotion {
    EFluidUVMotion x0_motion = EFluidUVMotion::Linear;
    float x4_ooTimeToWrap = 0.16666667f;
    float x8_orientation = 0.f;
    float xc_magnitude = 1.f;
    float x10_uvMul = 5.f;
    float x14_uvScale = 0.2f;

    SFluidLayerMotion() = default;
    SFluidLayerMotion(EFluidUVMotion motion, float timeToWrap, float orientation, float magnitude, float uvMul)
    : x0_motion(motion)
    , x4_ooTimeToWrap(1.f / timeToWrap)
    , x8_orientation(orientation)
    , xc_magnitude(magnitude)
    , x10_uvMul(uvMul)
    , x14_uvScale(1.f / uvMul) {}

    float GetUVScale() const { return x14_uvScale; }
  };

private:
  rstl::reserved_vector<SFluidLayerMotion, 3> x0_fluidLayers;
  float x4c_ooTimeToWrap;
  float x50_orientation;

public:
  CFluidUVMotion(float timeToWrap, float orientation, const SFluidLayerMotion& colorLayer,
                 const SFluidLayerMotion& pattern1Layer, const SFluidLayerMotion& pattern2Layer);
  CFluidUVMotion(float timeToWrap, float orientation);

  const rstl::reserved_vector<SFluidLayerMotion, 3>& GetFluidLayers() const { return x0_fluidLayers; }
  float GetOrientation() const { return x50_orientation; }
  float GetOOTimeToWrapTexPage() const { return x4c_ooTimeToWrap; }
  void CalculateFluidTextureOffset(float, float[3][2]) const;
};
} // namespace urde
