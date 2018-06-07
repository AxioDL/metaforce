#ifndef __URDE_CFLUIDPLANEGPU_HPP__
#define __URDE_CFLUIDPLANEGPU_HPP__

#include "CFluidPlaneCPU.hpp"

namespace urde
{

class CFluidPlaneGPU final : public CFluidPlaneCPU
{
public:
    CFluidPlaneGPU(CAssetId texPattern1, CAssetId texPattern2, CAssetId texColor, CAssetId bumpMap, CAssetId envMap,
                    CAssetId envBumpMap, CAssetId lightMap, float unitsPerLightmapTexel, float tileSize,
                    u32 tileSubdivisions, EFluidType fluidType, float alpha, const zeus::CVector3f& bumpLightDir,
                    float bumpScale, const CFluidUVMotion& mot, float turbSpeed, float turbDistance, float turbFreqMax,
                    float turbFreqMin, float turbPhaseMax, float turbPhaseMin, float turbAmplitudeMax,
                    float turbAmplitudeMin, float specularMin, float specularMax, float reflectionBlend,
                    float reflectionSize, float rippleIntensity, u32 maxVertCount);

    void RenderStripWithRipples(float curY, const CFluidPlaneRender::SHFieldSample (&heights)[46][46],
                                const u8 (&flags)[9][9], int startYDiv,
                                const CFluidPlaneRender::SPatchInfo& info,
                                std::vector<CFluidPlaneShader::Vertex>& vOut,
                                std::vector<CFluidPlaneShader::PatchVertex>& pvOut) const;
};

}

#endif // __URDE_CFLUIDPLANEGPU_HPP__