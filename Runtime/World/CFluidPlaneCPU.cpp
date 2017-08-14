#include "CFluidPlaneCPU.hpp"
#include "CSimplePool.hpp"
#include "GameGlobalObjects.hpp"
#include "CFluidPlaneManager.hpp"
#include "CStateManager.hpp"
#include "CWorld.hpp"
#include "World/CScriptWater.hpp"
#include "TCastTo.hpp"
#include "Camera/CGameCamera.hpp"

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

CFluidPlaneCPU::CFluidPlaneCPU(CAssetId texPattern1, CAssetId texPattern2, CAssetId texColor, CAssetId bumpMap,
                               CAssetId envMap, CAssetId envBumpMap, CAssetId lightMap, float unitsPerLightmapTexel,
                               float tileSize, u32 tileSubdivisions, EFluidType fluidType, float alpha,
                               const zeus::CVector3f& bumpLightDir, float bumpScale, const CFluidUVMotion& mot,
                               float turbSpeed, float turbDistance, float turbFreqMax, float turbFreqMin,
                               float turbPhaseMax, float turbPhaseMin, float turbAmplitudeMax, float turbAmplitudeMin,
                               float specularMin, float specularMax, float reflectionBlend, float reflectionSize,
                               float rippleIntensity, u32 maxVertCount)
: CFluidPlane(texPattern1, texPattern2, texColor, alpha, fluidType, rippleIntensity, mot),
  xa0_texIdBumpMap(bumpMap), xa4_texIdEnvMap(envMap), xa8_texIdEnvBumpMap(envBumpMap), xac_texId4(lightMap),
  xf0_bumpLightDir(bumpLightDir), xfc_bumpScale(bumpScale), x100_tileSize(tileSize),
  x104_tileSubdivisions(tileSubdivisions & ~0x1),
  x108_rippleResolution(x100_tileSize / float(x104_tileSubdivisions)),
  x10c_specularMin(specularMin), x110_specularMax(specularMax), x114_reflectionBlend(reflectionBlend),
  x118_reflectionSize(reflectionSize), x11c_unitsPerLightmapTexel(unitsPerLightmapTexel),
  x120_turbulence(turbSpeed, turbDistance, turbFreqMax, turbFreqMin, turbPhaseMax,
                  turbPhaseMin, turbAmplitudeMax, turbAmplitudeMin),
  m_maxVertCount(maxVertCount)
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

static bool sSineWaveInitialized = false;
static float sGlobalSineWave[256] = {};
static const float* InitializeSineWave()
{
    if (sSineWaveInitialized)
        return sGlobalSineWave;
    for (int i=0 ; i<256 ; ++i)
        sGlobalSineWave[i] = std::sin(2.f * M_PIF * (i / 256.f));
    sSineWaveInitialized = true;
    return sGlobalSineWave;
}

#define kEnableWaterBumpMaps true

CFluidPlaneShader::RenderSetupInfo
CFluidPlaneCPU::RenderSetup(const CStateManager& mgr, float alpha, const zeus::CTransform& xf,
                            const zeus::CTransform& areaXf, const zeus::CAABox& aabb, const CScriptWater* water) const
{
    CFluidPlaneShader::RenderSetupInfo out;

    float uvT = mgr.GetFluidPlaneManager()->GetUVT();
    bool hasBumpMap = HasBumpMap() && kEnableWaterBumpMaps;
    bool doubleLightmapBlend = false;
    bool hasEnvMap = mgr.GetCameraManager()->GetFluidCounter() == 0 && HasEnvMap();
    bool hasEnvBumpMap = HasEnvBumpMap();
    InitializeSineWave();
    CGraphics::SetModelMatrix(xf);

    if (hasBumpMap)
    {
        // Build 50% grey directional light with xf0_bumpLightDir and load into LIGHT_3
        // Light 3 in channel 1
        // Vertex colors in channel 0
        out.lights.resize(4);
        out.lights[3] = CLight::BuildDirectional(xf0_bumpLightDir, zeus::CColor::skGrey);
    }
    else
    {
        // Normal light mask in channel 1
        // Vertex colors in channel 0
        out.lights = water->GetActorLights()->BuildLightVector();
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
        if (std::fabs(water->GetMorphFactor()) < 0.00001f || !nextWater ||
                      !nextWater->GetFluidPlane().HasLightMap())
        {
            lightmapId = curTex;
            // Load lightmap
            CalculateLightmapMatrix(areaXf, xf, aabb, out.texMtxs[nextTexMtx++]);
            // Next: GX_TG_MTX2x4 GX_TG_POS, mtxNext, false, GX_PTIDENTITY
        }
        else if (nextWater && nextWater->GetFluidPlane().HasLightMap())
        {
            if (std::fabs(water->GetMorphFactor() - 1.f) < 0.00001f)
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

                float lum = lightLevel * water->GetMorphFactor();
                out.kColors[3] = zeus::CColor(lum, 1.f);
                lowLightBlend = (1.f - water->GetMorphFactor()) / (1.f - lum);
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

    if (!m_shader || m_cachedDoubleLightmapBlend != doubleLightmapBlend ||
        m_cachedAdditive != (mgr.GetParticleFlags() == 0))
    {
        m_cachedDoubleLightmapBlend = doubleLightmapBlend;
        m_cachedAdditive = mgr.GetParticleFlags() == 0;
        m_shader.emplace(x44_fluidType,
                         x10_texPattern1, x20_texPattern2, x30_texColor, xb0_bumpMap, xc0_envMap, xd0_envBumpMap,
                         xe0_lightmap, m_cachedDoubleLightmapBlend, m_cachedAdditive, m_maxVertCount);
    }

    return out;
}

int CFluidPlaneCPURender::numTilesInHField;
int CFluidPlaneCPURender::numSubdivisionsInTile;
int CFluidPlaneCPURender::numSubdivisionsInHField;

static bool PrepareRipple(const CRipple& ripple, const CFluidPlaneCPURender::SPatchInfo& info,
                          CFluidPlaneCPURender::SRippleInfo& rippleOut)
{
    auto lifeIdx = int((1.f - (ripple.GetTimeFalloff() - ripple.GetTime()) / ripple.GetTimeFalloff()) * 64.f);
    float dist = CFluidPlaneManager::RippleMaxs[lifeIdx] * (ripple.GetDistanceFalloff() / 256.f);
    dist *= dist;
    if (dist != 0)
        dist = std::sqrt(dist);
    dist = info.x24_ooRippleResolution * dist + 1.f;
    float centerX = info.x24_ooRippleResolution * (ripple.GetCenter().x - info.xc_globalMin.x);
    float centerY = info.x24_ooRippleResolution * (ripple.GetCenter().y - info.xc_globalMin.y);
    int fromX = int(centerX - dist) - 1;
    int toX = int(centerX + dist) + 1;
    int fromY = int(centerY - dist) - 1;
    int toY = int(centerY + dist) + 1;
    rippleOut.x4_fromX = std::max(0, fromX);
    rippleOut.x8_toX = std::min(int(info.x0_xSubdivs), toX);
    rippleOut.xc_fromY = std::max(0, fromY);
    rippleOut.x10_toY = std::min(int(info.x1_ySubdivs), toY);
    rippleOut.x14_gfromX = std::max(rippleOut.x14_gfromX, fromX);
    rippleOut.x18_gtoX = std::min(rippleOut.x18_gtoX, toX);
    rippleOut.x1c_gfromY = std::max(rippleOut.x1c_gfromY, fromY);
    rippleOut.x20_gtoY = std::min(rippleOut.x20_gtoY, toY);
    return !(rippleOut.x14_gfromX > rippleOut.x18_gtoX || rippleOut.x1c_gfromY > rippleOut.x20_gtoY);
}

static void ApplyTurbulence(float t, CFluidPlaneCPURender::SHFieldSample (&heights)[45][45], const u8 (&flags)[9][9],
                            const float sineWave[256], const CFluidPlaneCPURender::SPatchInfo& info,
                            const CFluidPlaneCPU& fluidPane, const zeus::CVector3f& areaCenter)
{
    if (!fluidPane.HasTurbulence())
    {
        memset(&heights, 0, sizeof(heights));
        return;
    }

    float scaledT = t * fluidPane.GetOOTurbulenceSpeed();
    float curY = info.x4_localMin.y - info.x18_rippleResolution - areaCenter.y;
    int xDivs = (info.x0_xSubdivs + CFluidPlaneCPURender::numSubdivisionsInTile - 4) /
        CFluidPlaneCPURender::numSubdivisionsInTile * CFluidPlaneCPURender::numSubdivisionsInTile + 2;
    int yDivs = (info.x1_ySubdivs + CFluidPlaneCPURender::numSubdivisionsInTile - 4) /
        CFluidPlaneCPURender::numSubdivisionsInTile * CFluidPlaneCPURender::numSubdivisionsInTile + 2;
    for (int i=0 ; i<=yDivs ; ++i)
    {
        float curYSq = curY * curY;
        float curX = info.x4_localMin.x - info.x18_rippleResolution - areaCenter.x;
        for (int j=0 ; j<=xDivs ; ++j)
        {
            float distFac = curX * curX + curYSq;
            if (distFac != 0.f)
                distFac = std::sqrt(distFac);
            heights[i][j].height =
                fluidPane.GetTurbulenceHeight(fluidPane.GetOOTurbulenceDistance() * distFac + scaledT);
            curX += info.x18_rippleResolution;
        }
        curY += info.x18_rippleResolution;
    }
}

static void ApplyRipple(const CFluidPlaneCPURender::SRippleInfo& rippleInfo,
                        CFluidPlaneCPURender::SHFieldSample (&heights)[45][45],
                        u8 (&flags)[9][9], const float sineWave[256], const CFluidPlaneCPURender::SPatchInfo& info)
{
    float lookupT = 256.f * (1.f - rippleInfo.x0_ripple.GetTime() * rippleInfo.x0_ripple.GetOOTimeFalloff() *
                             rippleInfo.x0_ripple.GetOOTimeFalloff()) * rippleInfo.x0_ripple.GetFrequency();
    auto lifeIdx = int(64.f * rippleInfo.x0_ripple.GetTime() * rippleInfo.x0_ripple.GetOOTimeFalloff());
    float distMul = rippleInfo.x0_ripple.GetDistanceFalloff() / 255.f;
    float minDist = CFluidPlaneManager::RippleMins[lifeIdx] * distMul;
    float minDistSq = minDist * minDist;
    if (minDistSq != 0.f)
        minDist = std::sqrt(minDistSq);
    float maxDist = CFluidPlaneManager::RippleMaxs[lifeIdx] * distMul;
    float maxDistSq = maxDist * maxDist;
    if (maxDistSq != 0.f)
        maxDist = std::sqrt(maxDistSq);
    int fromY = (rippleInfo.x1c_gfromY + CFluidPlaneCPURender::numSubdivisionsInTile - 1) /
        CFluidPlaneCPURender::numSubdivisionsInTile;
    int fromX = (rippleInfo.x14_gfromX + CFluidPlaneCPURender::numSubdivisionsInTile - 1) /
        CFluidPlaneCPURender::numSubdivisionsInTile;
    int toY = (rippleInfo.x20_gtoY + CFluidPlaneCPURender::numSubdivisionsInTile - 1) /
        CFluidPlaneCPURender::numSubdivisionsInTile;
    int toX = (rippleInfo.x18_gtoX + CFluidPlaneCPURender::numSubdivisionsInTile - 1) /
        CFluidPlaneCPURender::numSubdivisionsInTile;

    float curY = rippleInfo.x0_ripple.GetCenter().y - info.xc_globalMin.y -
        (0.5f * info.x14_tileSize + (fromY - 1) * info.x14_tileSize);
    int curGridY = info.x2a_gridDimX * (info.x2e_tileY + fromY - 1);
    int startGridX = (info.x28_tileX + fromX - 1);
    int gridCells = info.x2a_gridDimX * info.x2c_gridDimY;
    float f11 = 64.f * rippleInfo.x0_ripple.GetOODistanceFalloff();
    int curYDiv = rippleInfo.xc_fromY;

    for (int i=fromY ; i<=toY ; ++i, curY -= info.x14_tileSize)
    {
        int nextYDiv = (i+1) * CFluidPlaneCPURender::numSubdivisionsInTile;
        float curYSq = curY * curY;
        int curGridX = startGridX;
        int curXDiv = rippleInfo.x4_fromX;
        float curX = rippleInfo.x0_ripple.GetCenter().x - info.xc_globalMin.x -
            (0.5f * info.x14_tileSize + (fromX - 1) * info.x14_tileSize);
        for (int j=fromX ; j<=toX ; ++j, curX -= info.x14_tileSize, ++curGridX)
        {
            float dist = curX * curX + curYSq;
            if (dist != 0.f)
                dist = std::sqrt(dist);
            if (maxDist < dist - info.x1c_tileHypRadius || minDist > dist + info.x1c_tileHypRadius)
                continue;

            bool addedRipple = false;
            int nextXDiv = (j+1) * CFluidPlaneCPURender::numSubdivisionsInTile;
            float curXMod =
                (rippleInfo.x0_ripple.GetCenter().x - info.xc_globalMin.x) - info.x18_rippleResolution * curXDiv;
            float curYMod =
                (rippleInfo.x0_ripple.GetCenter().y - info.xc_globalMin.y) - info.x18_rippleResolution * curYDiv;

            if (!info.x30_gridFlags || (info.x30_gridFlags && curGridY >= 0 && curGridY < gridCells && curGridX >= 0 &&
                curGridX < info.x2a_gridDimX && info.x30_gridFlags[curGridX + curGridY]))
            {
                for (int k=curYDiv ; k<=std::min(rippleInfo.x10_toY, nextYDiv-1) ; ++k,
                     curYMod -= info.x18_rippleResolution)
                {
                    float tmpXMod = curXMod;
                    float curYModSq = curYMod * curYMod;
                    for (int l=curXDiv ; l<=std::min(rippleInfo.x8_toX, nextXDiv-1) ; ++l,
                         tmpXMod -= info.x18_rippleResolution)
                    {
                        float divDistSq = tmpXMod * tmpXMod + curYModSq;
                        if (divDistSq < minDistSq || divDistSq > maxDistSq)
                            continue;
                        float divDist = (divDistSq != 0.f) ? std::sqrt(divDistSq) : 0.f;
                        if (u8 val = CFluidPlaneManager::RippleValues[lifeIdx][int(divDist * f11)])
                        {
                            heights[k][l].height += val * rippleInfo.x0_ripple.GetAmplitude() *
                                sineWave[int(divDist * rippleInfo.x0_ripple.GetLookupPhase() + lookupT)];
                        }
                        else
                        {
                            heights[k][l].height += 0.f;
                        }
                        addedRipple = true;
                    }
                }

                if (addedRipple)
                    flags[i][j] = 0x1f;
            }
            else
            {
                int yMin = nextYDiv - 1;
                int yMax = nextYDiv - CFluidPlaneCPURender::numSubdivisionsInTile + 1;
                int xMin = nextXDiv - 1;
                int xMax = nextXDiv - CFluidPlaneCPURender::numSubdivisionsInTile + 1;

                if (curGridX >= 0.f && curGridX < info.x2a_gridDimX && curGridY - info.x2a_gridDimX >= 0 &&
                    !info.x30_gridFlags[curGridX + curGridY - info.x2a_gridDimX])
                    yMax -= 2;

                if (curGridX >= 0.f && curGridX < info.x2a_gridDimX && curGridY + info.x2a_gridDimX < gridCells &&
                    !info.x30_gridFlags[curGridX + info.x2a_gridDimX])
                    yMin += 2;

                if (curGridY >= 0 && curGridY < info.x2c_gridDimY && curGridX > 0 &&
                    !info.x30_gridFlags[curGridX - 1])
                    xMax -= 2;

                if (curGridY >= 0 && curGridY < info.x2c_gridDimY && curGridX + 1 < info.x2a_gridDimX &&
                    !info.x30_gridFlags[curGridX + 1])
                    xMin += 2;

                for (int k=curYDiv ; k<=std::min(rippleInfo.x10_toY, nextYDiv-1) ; ++k,
                     curYMod -= info.x18_rippleResolution)
                {
                    float tmpXMod = curXMod;
                    float curYModSq = curYMod * curYMod;
                    for (int l=curXDiv ; l<=std::min(rippleInfo.x8_toX, nextXDiv-1) ; ++l,
                         tmpXMod -= info.x18_rippleResolution)
                    {
                        if (k <= yMax || k >= yMin || l <= xMax || l >= xMin)
                        {
                            float divDistSq = tmpXMod * tmpXMod + curYModSq;
                            if (divDistSq < minDistSq || divDistSq > maxDistSq)
                                continue;
                            float divDist = (divDistSq != 0.f) ? std::sqrt(divDistSq) : 0.f;
                            if (u8 val = CFluidPlaneManager::RippleValues[lifeIdx][int(divDist * f11)])
                            {
                                heights[k][l].height += val * rippleInfo.x0_ripple.GetAmplitude() *
                                                    sineWave[int(divDist * rippleInfo.x0_ripple.GetLookupPhase() +
                                                                     lookupT)];
                            }
                            else
                            {
                                heights[k][l].height += 0.f;
                            }
                            addedRipple = true;
                        }
                    }
                }

                if (addedRipple)
                    flags[i][j] = 0xf;
            }
            curXDiv = nextXDiv;
        }

        curYDiv = nextYDiv;
        curGridY += info.x2a_gridDimX;
    }
}

static void ApplyRipples(const rstl::reserved_vector<CFluidPlaneCPURender::SRippleInfo, 32>& rippleInfos,
                         CFluidPlaneCPURender::SHFieldSample (&heights)[45][45], u8 (&flags)[9][9],
                         const float sineWave[256], const CFluidPlaneCPURender::SPatchInfo& info)
{
    for (const CFluidPlaneCPURender::SRippleInfo& rippleInfo : rippleInfos)
        ApplyRipple(rippleInfo, heights, flags, sineWave, info);
    for (int i=0 ; i<CFluidPlaneCPURender::numTilesInHField ; ++i)
        flags[0][i+1] |= 1;
    for (int i=0 ; i<CFluidPlaneCPURender::numTilesInHField ; ++i)
        flags[i+1][0] |= 8;
    for (int i=0 ; i<CFluidPlaneCPURender::numTilesInHField ; ++i)
        flags[i+1][CFluidPlaneCPURender::numTilesInHField+1] |= 4;
    for (int i=0 ; i<CFluidPlaneCPURender::numTilesInHField ; ++i)
        flags[CFluidPlaneCPURender::numTilesInHField+1][i+1] |= 2;
}

static void UpdatePatchNoNormals(CFluidPlaneCPURender::SHFieldSample (&heights)[45][45], const u8 (&flags)[9][9],
                                 const CFluidPlaneCPURender::SPatchInfo& info)
{
    for (int i=1 ; i <= (info.x1_ySubdivs + CFluidPlaneCPURender::numSubdivisionsInTile - 2) /
         CFluidPlaneCPURender::numSubdivisionsInTile ; ++i)
    {
        int r10 = i * CFluidPlaneCPURender::numSubdivisionsInTile + 1;
        int r9 = std::max(0, r10 - CFluidPlaneCPURender::numSubdivisionsInTile);
        int x24 = std::min(r10, info.x1_ySubdivs + 1);
        for (int j=1 ; j <= (info.x0_xSubdivs + CFluidPlaneCPURender::numSubdivisionsInTile - 2) /
             CFluidPlaneCPURender::numSubdivisionsInTile ; ++j)
        {
            int r29 = j * CFluidPlaneCPURender::numSubdivisionsInTile + 1;
            int r11 = std::max(0, r29 - CFluidPlaneCPURender::numSubdivisionsInTile);
            int x28 = std::min(r29, info.x0_xSubdivs + 1);
            if ((flags[i][j] & 0x1f) == 0x1f)
            {
                for (int k=r9 ; k<x24 ; ++k)
                {
                    for (int l=r11 ; l<x28 ; ++l)
                    {
                        CFluidPlaneCPURender::SHFieldSample& sample = heights[k][l];
                        if (sample.height > 0.f)
                            sample.wavecapIntensity =
                                u8(std::min(255, int(info.x38_wavecapIntensityScale * sample.height)));
                        else
                            sample.wavecapIntensity = 0;
                    }
                }
            }
            else
            {
                if (i > 0 && i < CFluidPlaneCPURender::numTilesInHField + 1 &&
                    j > 0 && j < CFluidPlaneCPURender::numTilesInHField + 1)
                {
                    int halfSubdivs = CFluidPlaneCPURender::numSubdivisionsInTile / 2;
                    CFluidPlaneCPURender::SHFieldSample& sample = heights[halfSubdivs + r9][halfSubdivs + r11];
                    if (sample.height > 0.f)
                        sample.wavecapIntensity =
                            u8(std::min(255, int(info.x38_wavecapIntensityScale * sample.height)));
                    else
                        sample.wavecapIntensity = 0;
                }

                if (i != 0)
                {
                    for (int l=r11 ; l<x28 ; ++l)
                    {
                        CFluidPlaneCPURender::SHFieldSample& sample = heights[r9][l];
                        if (sample.height > 0.f)
                            sample.wavecapIntensity =
                                u8(std::min(255, int(info.x38_wavecapIntensityScale * sample.height)));
                        else
                            sample.wavecapIntensity = 0;
                    }
                }

                if (j != 0)
                {
                    for (int k=r9+1 ; k<x24 ; ++k)
                    {
                        CFluidPlaneCPURender::SHFieldSample& sample = heights[k][r11];
                        if (sample.height > 0.f)
                            sample.wavecapIntensity =
                                u8(std::min(255, int(info.x38_wavecapIntensityScale * sample.height)));
                        else
                            sample.wavecapIntensity = 0;
                    }
                }
            }
        }
    }
}

static void UpdatePatchWithNormals(CFluidPlaneCPURender::SHFieldSample (& heights)[45][45], const u8 (& flags)[9][9],
                                   const CFluidPlaneCPURender::SPatchInfo& info)
{
    float normalScale = -(2.f * info.x18_rippleResolution);
    float nz = 0.25f * 2.f * info.x18_rippleResolution;
    int curGridY = info.x2e_tileY * info.x2a_gridDimX - 1 + info.x28_tileX;
    for (int i=1 ; i <= (info.x1_ySubdivs + CFluidPlaneCPURender::numSubdivisionsInTile - 2) /
         CFluidPlaneCPURender::numSubdivisionsInTile ; ++i, curGridY += info.x2a_gridDimX)
    {
        int r11 = i * CFluidPlaneCPURender::numSubdivisionsInTile + 1;
        int r9 = std::max(0, r11 - CFluidPlaneCPURender::numSubdivisionsInTile);
        int x38 = std::min(r11, info.x1_ySubdivs + 1);
        for (int j=1 ; j <= (info.x0_xSubdivs + CFluidPlaneCPURender::numSubdivisionsInTile - 2) /
                            CFluidPlaneCPURender::numSubdivisionsInTile ; ++j)
        {
            int r12 = j * CFluidPlaneCPURender::numSubdivisionsInTile + 1;
            int x3c = std::min(r12, info.x0_xSubdivs + 1);
            r12 -= CFluidPlaneCPURender::numSubdivisionsInTile;
            if ((flags[i][j] & 0x1f) == 0x1f)
            {
                for (int k=r9 ; k<x38 ; ++k)
                {
                    for (int l=r12 ; l<x3c ; ++l)
                    {
                        CFluidPlaneCPURender::SHFieldSample& sample = heights[k][l];
                        CFluidPlaneCPURender::SHFieldSample& up = heights[k+1][l];
                        CFluidPlaneCPURender::SHFieldSample& down = heights[k-1][l];
                        CFluidPlaneCPURender::SHFieldSample& right = heights[k][l+1];
                        CFluidPlaneCPURender::SHFieldSample& left = heights[k][l-1];
                        float nx = (right.height - left.height) * normalScale;
                        float ny = (up.height - down.height) * normalScale;
                        float normalizer = ny * ny + nx * nx + nz * nz;
                        if (normalizer != 0.f)
                            normalizer = std::sqrt(normalizer);
                        normalizer = 63.f / normalizer;
                        sample.nx = s8(nx * normalizer);
                        sample.ny = s8(ny * normalizer);
                        sample.nz = s8(nz * normalizer);
                        if (sample.height > 0.f)
                            sample.wavecapIntensity =
                                u8(std::min(255, int(info.x38_wavecapIntensityScale * sample.height)));
                        else
                            sample.wavecapIntensity = 0;
                    }
                }
            }
            else
            {
                if (!info.x30_gridFlags || info.x30_gridFlags[curGridY+j])
                {
                    if (i > 0 && i < CFluidPlaneCPURender::numTilesInHField + 1 &&
                        j > 0 && j < CFluidPlaneCPURender::numTilesInHField + 1)
                    {
                        int halfSubdivs = CFluidPlaneCPURender::numSubdivisionsInTile / 2;
                        int k = halfSubdivs + r9;
                        int l = halfSubdivs + r12;
                        CFluidPlaneCPURender::SHFieldSample& sample = heights[k][l];
                        CFluidPlaneCPURender::SHFieldSample& up = heights[k+1][l];
                        CFluidPlaneCPURender::SHFieldSample& down = heights[k-1][l];
                        CFluidPlaneCPURender::SHFieldSample& right = heights[k][l+1];
                        CFluidPlaneCPURender::SHFieldSample& left = heights[k][l-1];
                        float nx = (right.height - left.height) * normalScale;
                        float ny = (up.height - down.height) * normalScale;
                        float normalizer = ny * ny + nx * nx + nz * nz;
                        if (normalizer != 0.f)
                            normalizer = std::sqrt(normalizer);
                        normalizer = 63.f / normalizer;
                        sample.nx = s8(nx * normalizer);
                        sample.ny = s8(ny * normalizer);
                        sample.nz = s8(nz * normalizer);
                        if (sample.height > 0.f)
                            sample.wavecapIntensity =
                                u8(std::min(255, int(info.x38_wavecapIntensityScale * sample.height)));
                        else
                            sample.wavecapIntensity = 0;
                    }
                }

                if (j != 0 && i != 0)
                {
                    if ((flags[i][j] & 2) != 0 || (flags[i-1][j] & 1) != 0 ||
                        (flags[i][j] & 4) != 0 || (flags[i][j-1] & 8) != 0)
                    {
                        for (int l=r12 ; l<x3c ; ++l)
                        {
                            CFluidPlaneCPURender::SHFieldSample& sample = heights[r9][l];
                            CFluidPlaneCPURender::SHFieldSample& up = heights[r9+1][l];
                            CFluidPlaneCPURender::SHFieldSample& down = heights[r9-1][l];
                            CFluidPlaneCPURender::SHFieldSample& right = heights[r9][l+1];
                            CFluidPlaneCPURender::SHFieldSample& left = heights[r9][l-1];
                            float nx = (right.height - left.height) * normalScale;
                            float ny = (up.height - down.height) * normalScale;
                            float normalizer = ny * ny + nx * nx + nz * nz;
                            if (normalizer != 0.f)
                                normalizer = std::sqrt(normalizer);
                            normalizer = 63.f / normalizer;
                            sample.nx = s8(nx * normalizer);
                            sample.ny = s8(ny * normalizer);
                            sample.nz = s8(nz * normalizer);
                            if (sample.height > 0.f)
                                sample.wavecapIntensity =
                                    u8(std::min(255, int(info.x38_wavecapIntensityScale * sample.height)));
                            else
                                sample.wavecapIntensity = 0;
                        }

                        for (int k=r9 ; k<x38 ; ++k)
                        {
                            CFluidPlaneCPURender::SHFieldSample& sample = heights[k][r12];
                            CFluidPlaneCPURender::SHFieldSample& up = heights[k+1][r12];
                            CFluidPlaneCPURender::SHFieldSample& down = heights[k-1][r12];
                            CFluidPlaneCPURender::SHFieldSample& right = heights[k][r12+1];
                            CFluidPlaneCPURender::SHFieldSample& left = heights[k][r12-1];
                            float nx = (right.height - left.height) * normalScale;
                            float ny = (up.height - down.height) * normalScale;
                            float normalizer = ny * ny + nx * nx + nz * nz;
                            if (normalizer != 0.f)
                                normalizer = std::sqrt(normalizer);
                            normalizer = 63.f / normalizer;
                            sample.nx = s8(nx * normalizer);
                            sample.ny = s8(ny * normalizer);
                            sample.nz = s8(nz * normalizer);
                            if (sample.height > 0.f)
                                sample.wavecapIntensity =
                                    u8(std::min(255, int(info.x38_wavecapIntensityScale * sample.height)));
                            else
                                sample.wavecapIntensity = 0;
                        }
                    }
                    else
                    {
                        CFluidPlaneCPURender::SHFieldSample& sample = heights[r9][r12];
                        CFluidPlaneCPURender::SHFieldSample& up = heights[r9+1][r12];
                        CFluidPlaneCPURender::SHFieldSample& down = heights[r9-1][r12];
                        CFluidPlaneCPURender::SHFieldSample& right = heights[r9][r12+1];
                        CFluidPlaneCPURender::SHFieldSample& left = heights[r9][r12-1];
                        float nx = (right.height - left.height) * normalScale;
                        float ny = (up.height - down.height) * normalScale;
                        float normalizer = ny * ny + nx * nx + nz * nz;
                        if (normalizer != 0.f)
                            normalizer = std::sqrt(normalizer);
                        normalizer = 63.f / normalizer;
                        sample.nx = s8(nx * normalizer);
                        sample.ny = s8(ny * normalizer);
                        sample.nz = s8(nz * normalizer);
                        if (sample.height > 0.f)
                            sample.wavecapIntensity =
                                u8(std::min(255, int(info.x38_wavecapIntensityScale * sample.height)));
                        else
                            sample.wavecapIntensity = 0;
                    }
                }
            }
        }
    }
}

static bool UpdatePatch(float time, const CFluidPlaneCPURender::SPatchInfo& info,
                        CFluidPlaneCPURender::SHFieldSample (&heights)[45][45], u8 (&flags)[9][9],
                        const CFluidPlaneCPU& fluidPane, const zeus::CVector3f& areaCenter,
                        const std::experimental::optional<CRippleManager>& rippleManager,
                        int fromX, int toX, int fromY, int toY)
{
    rstl::reserved_vector<CFluidPlaneCPURender::SRippleInfo, 32> rippleInfos;
    if (rippleManager)
    {
        for (const CRipple& ripple : rippleManager->GetRipples())
        {
            if (ripple.GetTime() >= ripple.GetTimeFalloff())
                continue;
            CFluidPlaneCPURender::SRippleInfo rippleInfo(ripple, fromX, toX, fromY, toY);
            if (PrepareRipple(ripple, info, rippleInfo))
                rippleInfos.push_back(rippleInfo);
        }
    }

    if (rippleInfos.empty())
        return true;

    ApplyTurbulence(time, heights, flags, sGlobalSineWave, info, fluidPane, areaCenter);
    ApplyRipples(rippleInfos, heights, flags, sGlobalSineWave, info);
    if (info.x37_normalMode == CFluidPlaneCPURender::NormalMode::NoNormals)
        UpdatePatchNoNormals(heights, flags, info);
    else
        UpdatePatchWithNormals(heights, flags, info);

    return false;
}

static void RenderStripWithRipples(float curY, const CFluidPlaneCPURender::SHFieldSample (&heights)[45][45],
                                   const u8 (&flags)[9][9], int startYDiv,
                                   const CFluidPlaneCPURender::SPatchInfo& info,
                                   std::vector<CFluidPlaneShader::Vertex>& vOut)
{
    int yTile = (startYDiv + CFluidPlaneCPURender::numSubdivisionsInTile - 1) /
        CFluidPlaneCPURender::numSubdivisionsInTile;
    int endXTile = (info.x0_xSubdivs + CFluidPlaneCPURender::numSubdivisionsInTile - 4) /
        CFluidPlaneCPURender::numSubdivisionsInTile;

    int midDiv = CFluidPlaneCPURender::numSubdivisionsInTile / 2;
    float tileMid = info.x18_rippleResolution * midDiv;
    float yMin = curY;
    float yMid = curY + tileMid;
    float xMin = info.x4_localMin.x;

    float curX = info.x4_localMin.x;
    int gridCell = info.x28_tileX + info.x2a_gridDimX * (info.x2e_tileY + yTile - 1);
    int xTile = 1;
    int tileSpan;
    for (int i = 1 ; i < info.x0_xSubdivs - 2 ;
         i += CFluidPlaneCPURender::numSubdivisionsInTile * tileSpan, gridCell += tileSpan,
         xTile += tileSpan, curX += info.x14_tileSize * tileSpan)
    {
        tileSpan = 1;
        if (info.x30_gridFlags && !info.x30_gridFlags[gridCell])
            continue;

        if ((flags[yTile][xTile] & 0x1f) == 0x1f)
        {
            for (; xTile+tileSpan<=endXTile ; ++tileSpan)
            {
                if ((flags[yTile][xTile+tileSpan] & 0x1f) != 0x1f)
                    break;
                if (info.x30_gridFlags && !info.x30_gridFlags[gridCell+tileSpan])
                    break;
            }

            int stripDivCount = tileSpan * CFluidPlaneCPURender::numSubdivisionsInTile + 1;
            int remSubdivs = CFluidPlaneCPURender::numSubdivisionsInTile;
            std::function<void(float x, float y, const CFluidPlaneCPURender::SHFieldSample& samp)> func;

            switch (info.x37_normalMode)
            {
            case CFluidPlaneCPURender::NormalMode::None:
                func = [&](float x, float y, const CFluidPlaneCPURender::SHFieldSample& samp)
                {
                    vOut.emplace_back(zeus::CVector3f(x, y, samp.height));
                };
                break;
            case CFluidPlaneCPURender::NormalMode::NoNormals:
                func = [&](float x, float y, const CFluidPlaneCPURender::SHFieldSample& samp)
                {
                    vOut.emplace_back(zeus::CVector3f(x, y, samp.height), samp.MakeColor(info));
                };
                break;
            case CFluidPlaneCPURender::NormalMode::Normals:
                func = [&](float x, float y, const CFluidPlaneCPURender::SHFieldSample& samp)
                {
                    vOut.emplace_back(zeus::CVector3f(x, y, samp.height), samp.MakeNormal(),
                                      samp.MakeColor(info));
                };
                break;
            case CFluidPlaneCPURender::NormalMode::NBT:
                func = [&](float x, float y, const CFluidPlaneCPURender::SHFieldSample& samp)
                {
                    vOut.emplace_back(zeus::CVector3f(x, y, samp.height), samp.MakeNormal(),
                                      samp.MakeBinormal(), samp.MakeTangent(), samp.MakeColor(info));
                };
                break;
            }

            float curTileY = yMin;
            int curYDiv = startYDiv;
            for (; remSubdivs>0 ; --remSubdivs, ++curYDiv, curTileY+=info.x18_rippleResolution)
            {
                size_t start = vOut.size();
                float curTileX = xMin;
                for (int v=0 ; v<stripDivCount ; ++v)
                {
                    func(curTileX, curTileY, heights[curYDiv][i+v]);
                    func(curTileX, curTileY + info.x18_rippleResolution, heights[curYDiv+1][i+v]);
                    curTileX += info.x18_rippleResolution;
                }
                CGraphics::DrawArray(start, vOut.size() - start);
            }
        }
        else
        {
            bool r19 = (flags[yTile+1][xTile] & 0x2) != 0;
            bool r16 = (flags[yTile][xTile-1] & 0x8) != 0;
            bool r18 = (flags[yTile][xTile+1] & 0x4) != 0;
            bool r17 = (flags[yTile-1][xTile] & 0x1) != 0;

            int r6 = (r19 ? CFluidPlaneCPURender::numSubdivisionsInTile : 1) + 2;
            r6 += r18 ? CFluidPlaneCPURender::numSubdivisionsInTile : 1;
            r6 += r17 ? CFluidPlaneCPURender::numSubdivisionsInTile : 1;
            r6 += r16 ? CFluidPlaneCPURender::numSubdivisionsInTile : 1;

            if (r6 == 6 && (info.x37_normalMode == CFluidPlaneCPURender::NormalMode::Normals ||
                info.x37_normalMode == CFluidPlaneCPURender::NormalMode::NBT))
            {
                for (; xTile+tileSpan<=endXTile ; ++tileSpan)
                {
                    if ((flags[yTile][xTile+tileSpan] & 0x1f) == 0x1f)
                        break;
                    if (info.x30_gridFlags && !info.x30_gridFlags[gridCell+tileSpan])
                        break;
                    if ((flags[yTile+1][xTile+tileSpan] & 0x2) != 0x0)
                        break;
                    if ((flags[yTile][xTile+tileSpan+1] & 0x4) != 0x0)
                        break;
                    if ((flags[yTile-1][xTile+tileSpan] & 0x1) != 0x0)
                        break;
                }

                int stripDivCount = tileSpan + 1;
                size_t start = vOut.size();
                switch (info.x37_normalMode)
                {
                case CFluidPlaneCPURender::NormalMode::Normals:
                {
                    int curYDiv0 = startYDiv;
                    int curYDiv1 = startYDiv + CFluidPlaneCPURender::numSubdivisionsInTile;
                    float curTileX = xMin;
                    for (int v=0 ; v<stripDivCount ; ++v)
                    {
                        int curXDiv = v * CFluidPlaneCPURender::numSubdivisionsInTile + i;
                        const CFluidPlaneCPURender::SHFieldSample& samp0 = heights[curYDiv0][curXDiv];
                        const CFluidPlaneCPURender::SHFieldSample& samp1 = heights[curYDiv1][curXDiv];
                        vOut.emplace_back(zeus::CVector3f(curTileX, yMin, samp0.height),
                                          samp0.MakeNormal(), samp0.MakeColor(info));
                        vOut.emplace_back(zeus::CVector3f(curTileX, yMin + info.x14_tileSize, samp1.height),
                                          samp1.MakeNormal(), samp1.MakeColor(info));
                        curTileX += info.x18_rippleResolution;
                    }
                    break;
                }
                case CFluidPlaneCPURender::NormalMode::NBT:
                {
                    int curYDiv0 = startYDiv;
                    int curYDiv1 = startYDiv + CFluidPlaneCPURender::numSubdivisionsInTile;
                    float curTileX = xMin;
                    for (int v=0 ; v<stripDivCount ; ++v)
                    {
                        int curXDiv = v * CFluidPlaneCPURender::numSubdivisionsInTile + i;
                        const CFluidPlaneCPURender::SHFieldSample& samp0 = heights[curYDiv0][curXDiv];
                        const CFluidPlaneCPURender::SHFieldSample& samp1 = heights[curYDiv1][curXDiv];
                        vOut.emplace_back(zeus::CVector3f(curTileX, yMin, samp0.height),
                                          samp0.MakeNormal(), samp0.MakeBinormal(), samp0.MakeTangent(),
                                          samp0.MakeColor(info));
                        vOut.emplace_back(zeus::CVector3f(curTileX, yMin + info.x14_tileSize, samp1.height),
                                          samp1.MakeNormal(), samp1.MakeBinormal(), samp1.MakeTangent(),
                                          samp1.MakeColor(info));
                        curTileX += info.x18_rippleResolution;
                    }
                    break;
                }
                default:
                    break;
                }
                CGraphics::DrawArray(start, vOut.size() - start);
            }
            else
            {
                TriFanToStrip<CFluidPlaneShader::Vertex> toStrip(vOut);
                std::function<void(float x, float y, const CFluidPlaneCPURender::SHFieldSample& samp)> func;

                switch (info.x37_normalMode)
                {
                case CFluidPlaneCPURender::NormalMode::None:
                    func = [&](float x, float y, const CFluidPlaneCPURender::SHFieldSample& samp)
                    {
                        toStrip.EmplaceVert(zeus::CVector3f(x, y, samp.height));
                    };
                    break;
                case CFluidPlaneCPURender::NormalMode::NoNormals:
                    func = [&](float x, float y, const CFluidPlaneCPURender::SHFieldSample& samp)
                    {
                        toStrip.EmplaceVert(zeus::CVector3f(x, y, samp.height), samp.MakeColor(info));
                    };
                    break;
                case CFluidPlaneCPURender::NormalMode::Normals:
                    func = [&](float x, float y, const CFluidPlaneCPURender::SHFieldSample& samp)
                    {
                        toStrip.EmplaceVert(zeus::CVector3f(x, y, samp.height), samp.MakeNormal(),
                                            samp.MakeColor(info));
                    };
                    break;
                case CFluidPlaneCPURender::NormalMode::NBT:
                    func = [&](float x, float y, const CFluidPlaneCPURender::SHFieldSample& samp)
                    {
                        toStrip.EmplaceVert(zeus::CVector3f(x, y, samp.height), samp.MakeNormal(),
                                            samp.MakeBinormal(), samp.MakeTangent(), samp.MakeColor(info));
                    };
                    break;
                }

                func(tileMid + xMin, yMid, heights[startYDiv+midDiv][i+midDiv]);

                int curXDiv = i;
                int curYDiv = startYDiv + CFluidPlaneCPURender::numSubdivisionsInTile;
                float curTileX = xMin;
                float curTileY = yMin + info.x14_tileSize;
                for (int v=0 ; v<(r19 ? CFluidPlaneCPURender::numSubdivisionsInTile : 1) ; ++v)
                {
                    const CFluidPlaneCPURender::SHFieldSample& samp = heights[curYDiv][curXDiv+v];
                    func(curTileX, curTileY, samp);
                    curTileX += info.x18_rippleResolution;
                }

                curXDiv = i + CFluidPlaneCPURender::numSubdivisionsInTile;
                curYDiv = startYDiv + CFluidPlaneCPURender::numSubdivisionsInTile;
                curTileX = xMin + info.x14_tileSize;
                curTileY = yMin + info.x14_tileSize;
                for (int v=0 ; v<(r18 ? CFluidPlaneCPURender::numSubdivisionsInTile : 1) ; ++v)
                {
                    const CFluidPlaneCPURender::SHFieldSample& samp = heights[curYDiv-v][curXDiv];
                    func(curTileX, curTileY, samp);
                    curTileY -= info.x18_rippleResolution;
                }

                curXDiv = i + CFluidPlaneCPURender::numSubdivisionsInTile;
                curYDiv = startYDiv;
                curTileX = xMin + info.x14_tileSize;
                curTileY = yMin;
                for (int v=0 ; v<(r17 ? CFluidPlaneCPURender::numSubdivisionsInTile : 1) ; ++v)
                {
                    const CFluidPlaneCPURender::SHFieldSample& samp = heights[curYDiv][curXDiv-v];
                    func(curTileX, curTileY, samp);
                    curTileX -= info.x18_rippleResolution;
                }

                curXDiv = i;
                curYDiv = startYDiv;
                curTileX = xMin;
                curTileY = yMin;
                if (r16)
                {
                    for (int v=0 ; v<CFluidPlaneCPURender::numSubdivisionsInTile ; ++v)
                    {
                        const CFluidPlaneCPURender::SHFieldSample& samp = heights[curYDiv+v][curXDiv];
                        func(curTileX, curTileY, samp);
                        curTileY += info.x18_rippleResolution;
                    }
                }
                else
                {
                    {
                        const CFluidPlaneCPURender::SHFieldSample& samp = heights[curYDiv][curXDiv];
                        func(curTileX, curTileY, samp);
                    }
                    curTileY += info.x14_tileSize;
                    {
                        const CFluidPlaneCPURender::SHFieldSample& samp =
                            heights[curYDiv+CFluidPlaneCPURender::numSubdivisionsInTile][curXDiv];
                        func(curTileX, curTileY, samp);
                    }
                }

                toStrip.Draw();
            }
        }
    }
}

void RenderPatch(const CFluidPlaneCPURender::SPatchInfo& info,
                 const CFluidPlaneCPURender::SHFieldSample (&heights)[45][45],
                 const u8 (&flags)[9][9], bool noRipples, bool flagIs1,
                 std::vector<CFluidPlaneShader::Vertex>& vOut)
{
    if (noRipples)
    {
        float xMin = info.x4_localMin.x;
        float yMin = info.x4_localMin.y;
        float xMax = info.x18_rippleResolution * (info.x0_xSubdivs - 2) + xMin;
        float yMax = info.x18_rippleResolution * (info.x1_ySubdivs - 2) + yMin;

        switch (info.x37_normalMode)
        {
        case CFluidPlaneCPURender::NormalMode::None:
        {
            size_t start = vOut.size();
            vOut.emplace_back(zeus::CVector3f(xMin, yMin, 0.f));
            vOut.emplace_back(zeus::CVector3f(xMin, yMax, 0.f));
            vOut.emplace_back(zeus::CVector3f(xMax, yMin, 0.f));
            vOut.emplace_back(zeus::CVector3f(xMax, yMax, 0.f));
            CGraphics::DrawArray(start, 4);
            break;
        }
        case CFluidPlaneCPURender::NormalMode::NoNormals:
        {
            size_t start = vOut.size();
            vOut.emplace_back(zeus::CVector3f(xMin, yMin, 0.f), zeus::CColor::skBlack);
            vOut.emplace_back(zeus::CVector3f(xMin, yMax, 0.f), zeus::CColor::skBlack);
            vOut.emplace_back(zeus::CVector3f(xMax, yMin, 0.f), zeus::CColor::skBlack);
            vOut.emplace_back(zeus::CVector3f(xMax, yMax, 0.f), zeus::CColor::skBlack);
            CGraphics::DrawArray(start, 4);
            break;
        }
        case CFluidPlaneCPURender::NormalMode::Normals:
        {
            int yTiles = (info.x1_ySubdivs - 3) / CFluidPlaneCPURender::numSubdivisionsInTile + 1;
            int xTiles = (info.x0_xSubdivs - 3) / CFluidPlaneCPURender::numSubdivisionsInTile + 1;
            int xTileStart = info.x28_tileX + info.x2e_tileY * info.x2a_gridDimX;
            for (int curYTile=yTiles ; curYTile>0 ; --curYTile,
                 yMax += info.x14_tileSize, xTileStart += info.x2a_gridDimX)
            {
                xMax = xMin;
                int nextXTile;
                for (int curXTile=0 ; curXTile<xTiles ; curXTile=nextXTile)
                {
                    if (!info.x30_gridFlags || info.x30_gridFlags[xTileStart+curXTile])
                    {
                        if (curYTile == yTiles || curYTile == 1 || curXTile == 0 || xTiles - 1 == curXTile)
                        {
                            TriFanToStrip<CFluidPlaneShader::Vertex> toStrip(vOut);

                            toStrip.EmplaceVert(zeus::CVector3f(xMax + 0.5f * info.x14_tileSize,
                                                                yMax + 0.5f * info.x14_tileSize, 0.f),
                                                zeus::CVector3f::skUp, zeus::CColor::skBlack);

                            float tmp = xMax;
                            for (int v=0 ; v<((curYTile == 1) ?
                                              CFluidPlaneCPURender::numSubdivisionsInTile : 1) ; ++v)
                            {
                                toStrip.EmplaceVert(zeus::CVector3f(tmp, yMax + info.x14_tileSize, 0.f),
                                                    zeus::CVector3f::skUp, zeus::CColor::skBlack);
                                tmp += info.x18_rippleResolution;
                            }

                            tmp = yMax + info.x14_tileSize;
                            for (int v=0 ; v<((xTiles - 1 == curXTile) ?
                                              CFluidPlaneCPURender::numSubdivisionsInTile : 1) ; ++v)
                            {
                                toStrip.EmplaceVert(zeus::CVector3f(xMax + info.x14_tileSize, tmp, 0.f),
                                                    zeus::CVector3f::skUp, zeus::CColor::skBlack);
                                tmp -= info.x18_rippleResolution;
                            }

                            tmp = xMax + info.x14_tileSize;
                            for (int v=0 ; v<((curYTile == yTiles) ?
                                              CFluidPlaneCPURender::numSubdivisionsInTile : 1) ; ++v)
                            {
                                toStrip.EmplaceVert(zeus::CVector3f(tmp, yMax, 0.f),
                                                    zeus::CVector3f::skUp, zeus::CColor::skBlack);
                                tmp -= info.x18_rippleResolution;
                            }

                            tmp = yMax;
                            for (int v=0 ; v<((curXTile == 0) ?
                                              CFluidPlaneCPURender::numSubdivisionsInTile : 1) ; ++v)
                            {
                                toStrip.EmplaceVert(zeus::CVector3f(xMax, tmp, 0.f),
                                                    zeus::CVector3f::skUp, zeus::CColor::skBlack);
                                tmp += info.x18_rippleResolution;
                            }

                            toStrip.EmplaceVert(zeus::CVector3f(xMax, yMax + info.x14_tileSize, 0.f),
                                                zeus::CVector3f::skUp, zeus::CColor::skBlack);

                            toStrip.Draw();

                            nextXTile = curXTile + 1;
                            xMax += info.x14_tileSize;
                        }
                        else
                        {
                            nextXTile = curXTile + 1;
                            while (nextXTile < xTiles - 1 &&
                                (!info.x30_gridFlags || info.x30_gridFlags[xTileStart+nextXTile]))
                                ++nextXTile;

                            size_t start = vOut.size();
                            for (int v = 0 ; v < nextXTile - curXTile + 1 ; ++v)
                            {
                                vOut.emplace_back(zeus::CVector3f(xMax, yMax, 0.f), zeus::CColor::skBlack);
                                vOut.emplace_back(zeus::CVector3f(xMax, yMax + info.x14_tileSize, 0.f),
                                                  zeus::CColor::skBlack);
                                xMax += info.x14_tileSize;
                            }
                            CGraphics::DrawArray(start, vOut.size() - start);

                            ++nextXTile;
                            if (nextXTile == xTiles)
                            {
                                --nextXTile;
                                xMax -= info.x14_tileSize;
                            }
                        }
                    }
                    else
                    {
                        nextXTile = curXTile + 1;
                        xMax += info.x14_tileSize;
                        while (nextXTile < xTiles && !info.x30_gridFlags[xTileStart+nextXTile])
                        {
                            xMax += info.x14_tileSize;
                            ++nextXTile;
                        }
                    }
                }
            }
            break;
        }
        case CFluidPlaneCPURender::NormalMode::NBT:
        {
            if (flagIs1 || !info.x30_gridFlags)
            {
                size_t start = vOut.size();
                vOut.emplace_back(zeus::CVector3f(xMin, yMin, 0.f), zeus::CVector3f::skUp,
                                  zeus::CVector3f::skForward, zeus::CVector3f::skRight, zeus::CColor::skBlack);
                vOut.emplace_back(zeus::CVector3f(xMin, yMax, 0.f), zeus::CVector3f::skUp,
                                  zeus::CVector3f::skForward, zeus::CVector3f::skRight, zeus::CColor::skBlack);
                vOut.emplace_back(zeus::CVector3f(xMax, yMin, 0.f), zeus::CVector3f::skUp,
                                  zeus::CVector3f::skForward, zeus::CVector3f::skRight, zeus::CColor::skBlack);
                vOut.emplace_back(zeus::CVector3f(xMax, yMax, 0.f), zeus::CVector3f::skUp,
                                  zeus::CVector3f::skForward, zeus::CVector3f::skRight, zeus::CColor::skBlack);
                CGraphics::DrawArray(start, 4);
            }
            else
            {
                int xTiles = (info.x0_xSubdivs - 3) / CFluidPlaneCPURender::numSubdivisionsInTile + 1;
                int yTiles = (info.x1_ySubdivs - 3) / CFluidPlaneCPURender::numSubdivisionsInTile + 1;
                int xTileStart = info.x28_tileX + info.x2e_tileY * info.x2a_gridDimX;
                for (; yTiles>0 ; --yTiles, yMin += info.x14_tileSize, xTileStart += info.x2a_gridDimX)
                {
                    xMax = xMin;
                    int nextXTile;
                    for (int curXTile=0 ; curXTile<xTiles ; curXTile=nextXTile)
                    {
                        if (info.x30_gridFlags[xTileStart+curXTile])
                        {
                            nextXTile = curXTile + 1;
                            int tile = xTileStart + nextXTile;
                            while (nextXTile < xTiles && info.x30_gridFlags[tile])
                            {
                                ++nextXTile;
                                ++tile;
                            }

                            size_t start = vOut.size();
                            for (int v = 0 ; v < nextXTile - curXTile + 1 ; ++v)
                            {
                                vOut.emplace_back(zeus::CVector3f(xMax, yMin, 0.f), zeus::CVector3f::skUp,
                                                  zeus::CVector3f::skForward, zeus::CVector3f::skRight,
                                                  zeus::CColor::skBlack);
                                vOut.emplace_back(zeus::CVector3f(xMax, yMin + info.x14_tileSize, 0.f),
                                                  zeus::CVector3f::skUp, zeus::CVector3f::skForward,
                                                  zeus::CVector3f::skRight, zeus::CColor::skBlack);
                                xMax += info.x14_tileSize;
                            }
                            CGraphics::DrawArray(start, vOut.size() - start);
                        }
                        else
                        {
                            nextXTile = curXTile + 1;
                            xMax += info.x14_tileSize;
                            int tile = xTileStart + nextXTile;
                            while (nextXTile < xTiles && !info.x30_gridFlags[tile])
                            {
                                xMax += info.x14_tileSize;
                                ++nextXTile;
                                ++tile;
                            }
                        }
                    }
                }
            }
            break;
        }
        }
    }
    else
    {
        float curY = info.x4_localMin.y;
        for (int startYDiv=1 ; startYDiv<info.x1_ySubdivs-2 ;
             startYDiv += CFluidPlaneCPURender::numSubdivisionsInTile, curY += info.x14_tileSize)
            RenderStripWithRipples(curY, heights, flags, startYDiv, info, vOut);
    }
}

void CFluidPlaneCPU::Render(const CStateManager& mgr, float alpha, const zeus::CAABox& aabb, const zeus::CTransform& xf,
                            const zeus::CTransform& areaXf, bool noNormals, const zeus::CFrustum& frustum,
                            const std::experimental::optional<CRippleManager>& rippleManager, TUniqueId waterId,
                            const bool* gridFlags, u32 gridDimX, u32 gridDimY, const zeus::CVector3f& areaCenter) const
{
    TCastToConstPtr<CScriptWater> water = mgr.GetObjectById(waterId);
    CFluidPlaneShader::RenderSetupInfo setupInfo = RenderSetup(mgr, alpha, xf, areaXf, aabb, water.GetPtr());

    CFluidPlaneCPURender::NormalMode normalMode;
    if (xb0_bumpMap && kEnableWaterBumpMaps)
        normalMode = CFluidPlaneCPURender::NormalMode::NBT;
    else if (!noNormals)
        normalMode = CFluidPlaneCPURender::NormalMode::Normals;
    else
        normalMode = CFluidPlaneCPURender::NormalMode::NoNormals;

    // Set Position and color format

    switch (normalMode)
    {
    case CFluidPlaneCPURender::NormalMode::NBT:
        // Set NBT format
        break;
    case CFluidPlaneCPURender::NormalMode::Normals:
        // Set Normal format
        break;
    default:
        break;
    }

    float rippleResolutionRecip = 1.f / x108_rippleResolution;
    CFluidPlaneCPURender::numSubdivisionsInTile = x104_tileSubdivisions;
    CFluidPlaneCPURender::numTilesInHField = std::min(7, 42 / CFluidPlaneCPURender::numSubdivisionsInTile);
    CFluidPlaneCPURender::numSubdivisionsInHField =
        CFluidPlaneCPURender::numTilesInHField * CFluidPlaneCPURender::numSubdivisionsInTile;

    zeus::CVector3f aabbCenter = aabb.center();
    zeus::CVector2f center2D(aabbCenter.x, aabbCenter.y);
    zeus::CVector2f ripplePitch(x108_rippleResolution * CFluidPlaneCPURender::numSubdivisionsInHField);

    // Amount to shift intensity values right (for added wavecap color)
    int redShift = 0;
    int greenShift = 0;
    int blueShift = 0;
    float wavecapIntensityScale = g_tweakGame->GetWavecapIntensityNormal();
    switch (x44_fluidType)
    {
    case EFluidType::PoisonWater:
        wavecapIntensityScale = g_tweakGame->GetWavecapIntensityPoison();
        redShift = 1;
        blueShift = 1;
        break;
    case EFluidType::Lava:
    case EFluidType::Five:
        wavecapIntensityScale = g_tweakGame->GetWavecapIntensityLava();
        blueShift = 8;
        greenShift = 8;
        break;
    default:
        break;
    }

    if (water)
    {
        float cameraPenetration =
            mgr.GetCameraManager()->GetCurrentCamera(mgr)->GetTranslation().dot(zeus::CVector3f::skUp) -
            water->GetTriggerBoundsWR().max.z;
        wavecapIntensityScale *= (cameraPenetration >= 0.5f || cameraPenetration < 0.f) ? 1.f : 2.f * cameraPenetration;
    }

    u32 patchDimX = (water && water->GetPatchDimensionX()) ? water->GetPatchDimensionX() : 128;
    u32 patchDimY = (water && water->GetPatchDimensionY()) ? water->GetPatchDimensionY() : 128;

    m_verts.clear();
    m_shader->prepareDraw(setupInfo);

    u32 tileY = 0;
    float curY = aabb.min.y;
    for (int i=0 ; curY < aabb.max.y && i<patchDimY ; ++i)
    {
        u32 tileX = 0;
        float curX = aabb.min.x;
        float _remDivsY = (aabb.max.y - curY) * rippleResolutionRecip;
        for (int j=0 ; curX < aabb.max.x && j<patchDimX ; ++j)
        {
            if (u8 renderFlags = water->GetPatchRenderFlags(j, i))
            {
                s16 remDivsX = std::min(s16((aabb.max.x - curX) * rippleResolutionRecip),
                                        s16(CFluidPlaneCPURender::numSubdivisionsInHField));
                s16 remDivsY = std::min(s16(_remDivsY), s16(CFluidPlaneCPURender::numSubdivisionsInHField));
                zeus::CVector3f localMax(x108_rippleResolution * remDivsX + curX,
                                         x108_rippleResolution * remDivsY + curY, aabb.max.z);
                zeus::CVector3f localMin(curX, curY, aabb.min.z);
                zeus::CAABox testaabb(localMin + xf.origin, localMax + xf.origin);
                if (frustum.aabbFrustumTest(testaabb))
                {
                    CFluidPlaneCPURender::SPatchInfo info(localMin, localMax, xf.origin, x108_rippleResolution,
                                                          x100_tileSize, wavecapIntensityScale,
                                                          CFluidPlaneCPURender::numSubdivisionsInHField, normalMode,
                                                          redShift, greenShift, blueShift, tileX, gridDimX, gridDimY,
                                                          tileY, gridFlags);

                    int fromX = tileX != 0 ? (2 - CFluidPlaneCPURender::numSubdivisionsInTile) : 0;
                    int toX;
                    if (tileX != gridDimX - 1)
                        toX = info.x0_xSubdivs + (CFluidPlaneCPURender::numSubdivisionsInTile - 2);
                    else
                        toX = info.x0_xSubdivs;

                    int fromY = tileY != 0 ? (2 - CFluidPlaneCPURender::numSubdivisionsInTile) : 0;
                    int toY;
                    if (tileY != gridDimY - 1)
                        toY = info.x1_ySubdivs + (CFluidPlaneCPURender::numSubdivisionsInTile - 2);
                    else
                        toY = info.x1_ySubdivs;

                    CFluidPlaneCPURender::SHFieldSample heights[45][45];
                    u8 flags[9][9] = {};

                    bool noRipples = UpdatePatch(mgr.GetFluidPlaneManager()->GetUVT(), info, heights, flags,
                                                 *this, areaCenter, rippleManager, fromX, toX, fromY, toY);
                    RenderPatch(info, heights, flags, noRipples, renderFlags == 1, m_verts);
                }
            }
            curX += ripplePitch.x;
            tileX += CFluidPlaneCPURender::numTilesInHField;
        }
        curY += ripplePitch.y;
        tileY += CFluidPlaneCPURender::numTilesInHField;
    }

    m_shader->loadVerts(m_verts);
}

}
