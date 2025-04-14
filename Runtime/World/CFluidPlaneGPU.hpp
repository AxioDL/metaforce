#pragma once

#include <vector>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/World/CFluidPlaneCPU.hpp"

namespace metaforce {

class CFluidPlaneGPU final : public CFluidPlaneCPU {
public:
  CFluidPlaneGPU(CAssetId texPattern1, CAssetId texPattern2, CAssetId texColor, CAssetId bumpMap, CAssetId envMap,
                 CAssetId envBumpMap, CAssetId lightMap, float unitsPerLightmapTexel, float tileSize,
                 u32 tileSubdivisions, EFluidType fluidType, float alpha, const zeus::CVector3f& bumpLightDir,
                 float bumpScale, const CFluidUVMotion& mot, float turbSpeed, float turbDistance, float turbFreqMax,
                 float turbFreqMin, float turbPhaseMax, float turbPhaseMin, float turbAmplitudeMax,
                 float turbAmplitudeMin, float specularMin, float specularMax, float reflectionBlend,
                 float reflectionSize, float rippleIntensity, u32 maxVertCount);

  // void RenderStripWithRipples(float curY, const Heights& heights, const Flags& flags, int startYDiv,
  //                             const CFluidPlaneRender::SPatchInfo& info, std::vector<CFluidPlaneShader::Vertex>& vOut,
  //                             std::vector<CFluidPlaneShader::PatchVertex>& pvOut) override;
};

} // namespace metaforce
