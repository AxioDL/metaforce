#include "CFluidPlaneCPU.hpp"
#include "CSimplePool.hpp"
#include "GameGlobalObjects.hpp"
#include "CFluidPlaneManager.hpp"
#include "CStateManager.hpp"
#include "CWorld.hpp"
#include "World/CScriptWater.hpp"
#include "TCastTo.hpp"

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
                               ResId envBumpMap, ResId unkMap, float unitsPerLightmapTexel, float tileSize,
                               u32 tileSubdivisions, EFluidType fluidType, float alpha,
                               const zeus::CVector3f& bumpLightDir, float bumpScale, const CFluidUVMotion& mot,
                               float turbSpeed, float turbDistance, float turbFreqMax, float turbFreqMin,
                               float turbPhaseMax, float turbPhaseMin, float turbAmplitudeMax, float turbAmplitudeMin,
                               float specularMin, float specularMax, float reflectionBlend, float reflectionSize,
                               float fluidPlaneF2)
: CFluidPlane(texPattern1, texPattern2, texColor, alpha, fluidType, fluidPlaneF2, mot),
  xa0_texIdBumpMap(bumpMap), xa4_texIdEnvMap(envMap), xa8_texIdEnvBumpMap(envBumpMap), xac_texId4(unkMap),
  xf0_bumpLightDir(bumpLightDir), xfc_bumpScale(bumpScale), x100_tileSize(tileSize),
  x104_tileSubdivisions(tileSubdivisions & ~0x1),
  x108_rippleResolution(x100_tileSize / float(x104_tileSubdivisions)),
  x10c_specularMin(specularMin), x110_specularMax(specularMax), x114_reflectionBlend(reflectionBlend),
  x118_reflectionSize(reflectionSize), x11c_unitsPerLightmapTexel(unitsPerLightmapTexel),
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
        xe0_lightmap.emplace(g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), xac_texId4}));
}

void CFluidPlaneCPU::CreateRipple(const CRipple& ripple, CStateManager& mgr)
{

}

void CFluidPlaneCPU::CalculateLightmapMatrix(const zeus::CTransform& areaXf, const zeus::CTransform& xf,
                                             const zeus::CAABox& aabb, zeus::CMatrix4f& mtxOut) const
{
    int width = GetLightMap().GetWidth();
    int height = GetLightMap().GetHeight();

    zeus::CTransform toLocal = areaXf.getRotation().inverse();
    zeus::CAABox areaLocalAABB = aabb.getTransformedAABox(toLocal);
    float f26 = (areaLocalAABB.max.x - areaLocalAABB.min.x) / (width * x11c_unitsPerLightmapTexel);
    float f25 = (areaLocalAABB.max.y - areaLocalAABB.min.y) / (height * x11c_unitsPerLightmapTexel);
    float f24 = (1.f + std::fmod(areaLocalAABB.min.x + xf.origin.x, x11c_unitsPerLightmapTexel)) / width;
    float f23 = (2.f - std::fmod(areaLocalAABB.max.x + xf.origin.x, x11c_unitsPerLightmapTexel)) / width;
    float f29 = (1.f + std::fmod(areaLocalAABB.min.y + xf.origin.y, x11c_unitsPerLightmapTexel)) / height;
    float f6 = (2.f - std::fmod(areaLocalAABB.max.y + xf.origin.y, x11c_unitsPerLightmapTexel)) / height;

    float scaleX = (f26 - f24 - f23) / (areaLocalAABB.max.x - areaLocalAABB.min.x);
    float scaleY = -(f25 - f29 - f6) / (areaLocalAABB.max.y - areaLocalAABB.min.y);
    float offX = f24 + f26 * -areaLocalAABB.min.x / (areaLocalAABB.max.x - areaLocalAABB.min.x);
    float offY = f25 * areaLocalAABB.min.y / (areaLocalAABB.max.y - areaLocalAABB.min.y) - f6;
    mtxOut = (zeus::CTransform(zeus::CMatrix3f(zeus::CVector3f(scaleX, scaleY, 0.f)),
                               zeus::CVector3f(offX, offY, 0.f)) * toLocal).toMatrix4f();
}

#define kEnableWaterBumpMaps true

CFluidPlaneCPU::RenderSetupInfo
CFluidPlaneCPU::RenderSetup(const CStateManager& mgr, float alpha, const zeus::CTransform& xf,
                            const zeus::CTransform& areaXf, const zeus::CAABox& aabb, const CScriptWater* water) const
{
    RenderSetupInfo out;

    float uvT = mgr.GetFluidPlaneManager()->GetUVT();
    bool hasBumpMap = HasBumpMap() && kEnableWaterBumpMaps;
    bool doubleLightmapBlend = false;
    bool hasEnvMap = mgr.GetCameraManager()->GetFluidCounter() == 0 && HasEnvMap();
    bool hasEnvBumpMap = HasEnvBumpMap();
    CGraphics::SetModelMatrix(xf);

    if (hasBumpMap)
    {
        // Build 50% grey directional light with xf0_bumpLightDir and load into LIGHT_3
        // Light 3 in channel 1
        // Vertex colors in channel 0
    }
    else
    {
        // Normal light mask in channel 1
        // Vertex colors in channel 0
    }

    if (x10_texPattern1)
    {
        // Load into 0
    }
    else
    {
        // Load black tex into 0
    }

    if (x20_texPattern2)
    {
        // Load into 1
    }
    else
    {
        // Load black tex into 1
    }

    if (x30_texColor)
    {
        // Load into 2
    }
    else
    {
        // Load black tex into 2
    }

    int curTex = 3;
    int bumpMapId;
    int envMapId;
    int envBumpMapId;
    int lightmapId;

    if (hasBumpMap)
    {
        // Load into next
        bumpMapId = curTex++;
    }

    if (hasEnvMap)
    {
        // Load into next
        envMapId = curTex++;
    }

    if (hasEnvBumpMap)
    {
        // Load into next
        envBumpMapId = curTex++;
    }

    float fluidUVs[3][2];
    x4c_uvMotion.CalculateFluidTextureOffset(uvT, fluidUVs);

    out.texMtxs[0][0][0] = out.texMtxs[0][1][1] = x4c_uvMotion.GetFluidLayers()[1].GetUVScale();
    out.texMtxs[0][3][0] = fluidUVs[1][0];
    out.texMtxs[0][3][1] = fluidUVs[1][1];

    out.texMtxs[1][0][0] = out.texMtxs[1][1][1] = x4c_uvMotion.GetFluidLayers()[2].GetUVScale();
    out.texMtxs[1][3][0] = fluidUVs[2][0];
    out.texMtxs[1][3][1] = fluidUVs[2][1];

    out.texMtxs[2][0][0] = out.texMtxs[2][1][1] = x4c_uvMotion.GetFluidLayers()[0].GetUVScale();
    out.texMtxs[2][3][0] = fluidUVs[0][0];
    out.texMtxs[2][3][1] = fluidUVs[0][1];

    // Load normal mtx 0 with
    out.normMtx =
        (zeus::CTransform::Scale(xfc_bumpScale) * CGraphics::g_ViewMatrix.getRotation().inverse()).toMatrix4f();

    // Setup TCGs
    int nextTexMtx = 3;

    if (hasEnvBumpMap)
    {
        float pttScale;
        if (hasEnvMap)
            pttScale = 0.5f * (1.f - x118_reflectionSize);
        else
            pttScale = g_tweakGame->GetFluidEnvBumpScale() * x4c_uvMotion.GetFluidLayers()[0].GetUVScale();

        // Load GX_TEXMTX3 with identity
        zeus::CMatrix4f& texMtx = out.texMtxs[nextTexMtx++];
        // Load GX_PTTEXMTX0 with scale of pttScale
        // Next: GX_TG_MTX2x4 GX_TG_NRM, GX_TEXMTX3, true, GX_PTTEXMTX0

        out.indScale = 0.5f * (hasEnvMap ? x118_reflectionSize : 1.f);
        // Load ind mtx with scale of (indScale, -indScale)
        // Load envBumpMap into ind stage 0 with previous TCG
    }

    if (hasEnvMap)
    {
        float scale = std::max(aabb.max.x - aabb.min.x, aabb.max.y - aabb.min.y);
        zeus::CMatrix4f& texMtx = out.texMtxs[nextTexMtx++];
        texMtx[0][0] = texMtx[1][1] = 1.f / scale;
        zeus::CVector3f center = aabb.center();
        texMtx[3][0] = 0.5f + -center.x / scale;
        texMtx[3][1] = 0.5f + -center.y / scale;
        // Next: GX_TG_MTX2x4 GX_TG_POS, mtxNext, false, GX_PTIDENTITY
    }

    if (HasLightMap())
    {
        float lowLightBlend = 1.f;
        const CGameArea* area = mgr.GetWorld()->GetAreaAlways(mgr.GetNextAreaId());
        float lightLevel = area->GetPostConstructed()->x1128_worldLightingLevel;
        const CScriptWater* nextWater = water->GetNextConnectedWater(mgr);
        if (std::fabs(water->GetLightmapDoubleBlendFactor()) < 0.00001f || !nextWater ||
                      !nextWater->GetFluidPlane().HasLightMap())
        {
            lightmapId = curTex;
            // Load lightmap
            CalculateLightmapMatrix(areaXf, xf, aabb, out.texMtxs[nextTexMtx++]);
            // Next: GX_TG_MTX2x4 GX_TG_POS, mtxNext, false, GX_PTIDENTITY
        }
        else if (nextWater && nextWater->GetFluidPlane().HasLightMap())
        {
            if (std::fabs(water->GetLightmapDoubleBlendFactor() - 1.f) < 0.00001f)
            {
                lightmapId = curTex;
                // Load lightmap
                CalculateLightmapMatrix(areaXf, xf, aabb, out.texMtxs[nextTexMtx++]);
                // Next: GX_TG_MTX2x4 GX_TG_POS, mtxNext, false, GX_PTIDENTITY
            }
            else
            {
                lightmapId = curTex;
                // Load lightmap
                CalculateLightmapMatrix(areaXf, xf, aabb, out.texMtxs[nextTexMtx++]);
                // Next: GX_TG_MTX2x4 GX_TG_POS, mtxNext, false, GX_PTIDENTITY
                // Load lightmap
                CalculateLightmapMatrix(areaXf, xf, aabb, out.texMtxs[nextTexMtx++]);
                // Next: GX_TG_MTX2x4 GX_TG_POS, mtxNext, false, GX_PTIDENTITY

                float lum = lightLevel * water->GetLightmapDoubleBlendFactor();
                out.kColors[3] = zeus::CColor(lum, 1.f);
                lowLightBlend = (1.f - water->GetLightmapDoubleBlendFactor()) / (1.f - lum);
                doubleLightmapBlend = true;
            }
        }

        out.kColors[2] = zeus::CColor(lowLightBlend * lightLevel, 1.f);
    }

    float waterPlaneOrthoDot = xf.transposeRotate(zeus::CVector3f::skUp).
        dot(CGraphics::g_ViewMatrix.inverse().transposeRotate(zeus::CVector3f::skForward));
    if (waterPlaneOrthoDot < 0.f)
        waterPlaneOrthoDot = -waterPlaneOrthoDot;

    out.kColors[0] =
        zeus::CColor((1.f - waterPlaneOrthoDot) * (x110_specularMax - x10c_specularMin) + x10c_specularMin, alpha);
    out.kColors[1] = zeus::CColor(x114_reflectionBlend, 1.f);

    // TODO: Detect parameter changes and rebuild if needed
    if (!m_shader)
        m_shader.emplace(x44_fluidType,
                         x10_texPattern1, x20_texPattern2, x30_texColor, xb0_bumpMap, xc0_envMap, xd0_envBumpMap,
                         xe0_lightmap, doubleLightmapBlend, mgr.GetParticleFlags() == 0);
    out.lights = water->GetActorLights()->BuildLightVector();

    return out;
}

void CFluidPlaneCPU::Render(const CStateManager& mgr, float alpha, const zeus::CAABox& aabb, const zeus::CTransform& xf,
                            const zeus::CTransform& areaXf, bool noSubdiv, const zeus::CFrustum& frustum,
                            const std::experimental::optional<CRippleManager>& rippleManager, TUniqueId waterId,
                            const bool* gridFlags, u32 gridDimX, u32 gridDimY, const zeus::CVector3f& areaCenter) const
{
    TCastToConstPtr<CScriptWater> water = mgr.GetObjectById(waterId);
    RenderSetupInfo setupInfo = RenderSetup(mgr, alpha, xf, areaXf, aabb, water.GetPtr());
    m_shader->draw(setupInfo.texMtxs, setupInfo.normMtx, setupInfo.indScale, setupInfo.lights, setupInfo.kColors);
}

void CFluidPlaneCPU::RenderCleanup() const
{

}

}
