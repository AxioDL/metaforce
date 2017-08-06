#include "CFluidPlaneCPU.hpp"
#include "CSimplePool.hpp"
#include "GameGlobalObjects.hpp"

#define kTableSize 2048

namespace urde
{
CFluidPlaneCPU::CTurbulence::CTurbulence(float speed, float distance, float freqMax, float freqMin,
                                         float phaseMax, float phaseMin, float amplitudeMax, float amplitudeMin)
: x0_speed(speed), x4_distance(distance), x8_freqMax(freqMax), xc_freqMin(freqMin),
  x10_phaseMax(phaseMax), x14_phaseMin(phaseMin), x18_amplitudeMax(amplitudeMax), x1c_amplitudeMin(amplitudeMin),
  x2c_ooTurbSpeed(1.f / x0_speed), x30_ooTurbDistance(1.f / x4_distance)
{
    if (x18_amplitudeMax != 0.f || x1c_amplitudeMin != 0.f)
    {
        x24_tableCount = kTableSize;
        x28_heightSelPitch = x24_tableCount;
        x20_table.reset(new float[x24_tableCount]);
        float anglePitch = 2.f * M_PIF / x28_heightSelPitch;
        float freqConstant = 0.5f * (x8_freqMax + xc_freqMin);
        float freqLinear = 0.5f * (x8_freqMax - xc_freqMin);
        float phaseConstant = 0.5f * (x10_phaseMax + x14_phaseMin);
        float phaseLinear = 0.5f * (x10_phaseMax - x14_phaseMin);
        float amplitudeConstant = 0.5f * (x18_amplitudeMax + x1c_amplitudeMin);
        float amplitudeLinear = 0.5f * (x18_amplitudeMax - x1c_amplitudeMin);

        float curAng = 0.f;
        for (int i=0 ; i<x24_tableCount ; ++i, curAng += anglePitch)
        {
            float angCos = std::cos(curAng);
            x20_table[i] = (amplitudeLinear * angCos + amplitudeConstant) *
                std::sin((freqLinear * angCos + freqConstant) * curAng + (phaseLinear * angCos + phaseConstant));
        }

        x34_hasTurbulence = true;
    }
}

CFluidPlaneCPU::CFluidPlaneCPU(ResId texPattern1, ResId texPattern2, ResId texColor, ResId bumpMap, ResId envMap,
                               ResId envBumpMap, ResId unkMap, float f1, float tileSize, u32 tileSubdivisions,
                               EFluidType fluidType, float alpha, const zeus::CVector3f& bumpLightDir, float bumpScale,
                               const CFluidUVMotion& mot, float turbSpeed, float turbDistance, float turbFreqMax,
                               float turbFreqMin, float turbPhaseMax, float turbPhaseMin, float turbAmplitudeMax,
                               float turbAmplitudeMin, float specularMin, float specularMax, float reflectionBlend,
                               float reflectionSize, float fluidPlaneF2)
: CFluidPlane(texPattern1, texPattern2, texColor, alpha, fluidType, fluidPlaneF2, mot),
  xa0_texIdBumpMap(bumpMap), xa4_texIdEnvMap(envMap), xa8_texIdEnvBumpMap(envBumpMap), xac_texId4(unkMap),
  xf0_bumpLightDir(bumpLightDir), xfc_bumpScale(bumpScale), x100_tileSize(tileSize),
  x104_tileSubdivisions(tileSubdivisions & ~0x1),
  x108_rippleResolution(x100_tileSize / float(x104_tileSubdivisions)),
  x10c_specularMin(specularMin), x110_specularMax(specularMax), x114_reflectionBlend(reflectionBlend),
  x118_reflectionSize(reflectionSize), x11c_f1(f1),
  x120_turbulence(turbSpeed, turbDistance, turbFreqMax, turbFreqMin, turbPhaseMax,
                  turbPhaseMin, turbAmplitudeMax, turbAmplitudeMin)
{
    if (g_ResFactory->GetResourceTypeById(xa0_texIdBumpMap) == FOURCC('TXTR'))
        xb0_bumpMap.emplace(g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), xa0_texIdBumpMap}));
    if (g_ResFactory->GetResourceTypeById(xa4_texIdEnvMap) == FOURCC('TXTR'))
        xc0_envMap.emplace(g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), xa4_texIdEnvMap}));
    if (g_ResFactory->GetResourceTypeById(xa8_texIdEnvBumpMap) == FOURCC('TXTR'))
        xd0_envBumpMap.emplace(g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), xa8_texIdEnvBumpMap}));
    if (g_ResFactory->GetResourceTypeById(xac_texId4) == FOURCC('TXTR'))
        xe0_tex4.emplace(g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), xac_texId4}));
}

void CFluidPlaneCPU::CreateRipple(const CRipple& ripple, CStateManager& mgr)
{

}

void CFluidPlaneCPU::RenderSetup(const CStateManager& mgr, float, const zeus::CTransform& xf,
                                 const zeus::CTransform& areaXf, const zeus::CAABox& aabb, CScriptWater* water) const
{

}

void CFluidPlaneCPU::Render(const CStateManager& mgr, const zeus::CAABox& aabb, const zeus::CTransform& xf,
                            const zeus::CTransform& areaXf, bool noSubdiv, const zeus::CFrustum& frustum,
                            const std::experimental::optional<CRippleManager>& rippleManager, TUniqueId waterId,
                            const bool* gridFlags, u32 gridDimX, u32 gridDimY, const zeus::CVector3f& areaCenter) const
{

}

void CFluidPlaneCPU::RenderCleanup() const
{

}

}
