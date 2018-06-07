#include "CFluidPlaneGPU.hpp"

namespace urde
{

CFluidPlaneGPU::CFluidPlaneGPU(CAssetId texPattern1, CAssetId texPattern2, CAssetId texColor, CAssetId bumpMap,
                               CAssetId envMap, CAssetId envBumpMap, CAssetId lightMap, float unitsPerLightmapTexel,
                               float tileSize, u32 tileSubdivisions, EFluidType fluidType, float alpha,
                               const zeus::CVector3f& bumpLightDir, float bumpScale, const CFluidUVMotion& mot,
                               float turbSpeed, float turbDistance, float turbFreqMax, float turbFreqMin,
                               float turbPhaseMax, float turbPhaseMin, float turbAmplitudeMax, float turbAmplitudeMin,
                               float specularMin, float specularMax, float reflectionBlend, float reflectionSize,
                               float rippleIntensity, u32 maxVertCount)
: CFluidPlaneCPU(texPattern1, texPattern2, texColor, bumpMap, envMap, envBumpMap, lightMap, unitsPerLightmapTexel,
                 tileSize, tileSubdivisions, fluidType, alpha, bumpLightDir, bumpScale, mot, turbSpeed, turbDistance,
                 turbFreqMax, turbFreqMin, turbPhaseMax, turbPhaseMin, turbAmplitudeMax, turbAmplitudeMin,
                 specularMin, specularMax, reflectionBlend, reflectionSize, rippleIntensity, maxVertCount)
{
    m_tessellation = true;
}

void CFluidPlaneGPU::RenderStripWithRipples(float curY, const CFluidPlaneRender::SHFieldSample (&heights)[46][46],
                                            const u8 (&flags)[9][9], int startYDiv,
                                            const CFluidPlaneRender::SPatchInfo& info,
                                            std::vector<CFluidPlaneShader::Vertex>& vOut,
                                            std::vector<CFluidPlaneShader::PatchVertex>& pvOut) const
{
    m_shader->bindTessellation();

    int yTile = (startYDiv + CFluidPlaneRender::numSubdivisionsInTile - 1) /
                CFluidPlaneRender::numSubdivisionsInTile;
    int endXTile = (info.x0_xSubdivs + CFluidPlaneRender::numSubdivisionsInTile - 4) /
                   CFluidPlaneRender::numSubdivisionsInTile;

    float yMin = curY;
    float subdivF = CFluidPlaneRender::numSubdivisionsInTile;

    float curX = info.x4_localMin.x;
    int gridCell = info.x28_tileX + info.x2a_gridDimX * (info.x2e_tileY + yTile - 1);
    int xTile = 1;
    int tileSpan;
    for (int i = 1 ; i < info.x0_xSubdivs - 2 ;
         i += CFluidPlaneRender::numSubdivisionsInTile * tileSpan, gridCell += tileSpan,
         xTile += tileSpan, curX += info.x14_tileSize * tileSpan)
    {
        tileSpan = 1;
        if (info.x30_gridFlags && !info.x30_gridFlags[gridCell])
            continue;

        CFluidPlaneShader::PatchVertex pv;
        size_t start = pvOut.size();

        if ((flags[yTile][xTile] & 0x1f) == 0x1f)
        {
            for (; xTile+tileSpan<=endXTile ; ++tileSpan)
            {
                if ((flags[yTile][xTile+tileSpan] & 0x1f) != 0x1f)
                    break;
                if (info.x30_gridFlags && !info.x30_gridFlags[gridCell+tileSpan])
                    break;
            }

            std::fill(std::begin(pv.m_outerLevels), std::end(pv.m_outerLevels), subdivF);
            std::fill(std::begin(pv.m_innerLevels), std::end(pv.m_innerLevels), subdivF);
        }
        else
        {
            bool r19 = (flags[yTile+1][xTile] & 0x2) != 0; // North
            bool r16 = (flags[yTile][xTile-1] & 0x8) != 0; // West
            bool r18 = (flags[yTile][xTile+1] & 0x4) != 0; // East
            bool r17 = (flags[yTile-1][xTile] & 0x1) != 0; // South

            pv.m_outerLevels[0] = r16 ? subdivF : 1.f;
            pv.m_outerLevels[1] = r17 ? subdivF : 1.f;
            pv.m_outerLevels[2] = r18 ? subdivF : 1.f;
            pv.m_outerLevels[3] = r19 ? subdivF : 1.f;
            std::fill(std::begin(pv.m_innerLevels), std::end(pv.m_innerLevels), subdivF);
        }

        float curTileY = yMin;
        float curTileX = curX;
        for (int t=0 ; t<tileSpan ; ++t)
        {
            pv.m_pos = zeus::CVector4f(curTileX, curTileY, curTileX + info.x14_tileSize, curTileY + info.x14_tileSize);
            pvOut.push_back(pv);
            curTileX += info.x14_tileSize;
        }

        CGraphics::DrawArray(start, pvOut.size() - start);
    }
}

}