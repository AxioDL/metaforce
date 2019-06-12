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

namespace urde {
CFluidPlaneCPU::CTurbulence::CTurbulence(float speed, float distance, float freqMax, float freqMin, float phaseMax,
                                         float phaseMin, float amplitudeMax, float amplitudeMin)
: x0_speed(speed)
, x4_distance(distance)
, x8_freqMax(freqMax)
, xc_freqMin(freqMin)
, x10_phaseMax(phaseMax)
, x14_phaseMin(phaseMin)
, x18_amplitudeMax(amplitudeMax)
, x1c_amplitudeMin(amplitudeMin)
, x2c_ooTurbSpeed(1.f / x0_speed)
, x30_ooTurbDistance(1.f / x4_distance) {
  if (x18_amplitudeMax != 0.f || x1c_amplitudeMin != 0.f) {
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
    for (int i = 0; i < x24_tableCount; ++i, curAng += anglePitch) {
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
: CFluidPlane(texPattern1, texPattern2, texColor, alpha, fluidType, rippleIntensity, mot)
, xa0_texIdBumpMap(bumpMap)
, xa4_texIdEnvMap(envMap)
, xa8_texIdEnvBumpMap(envBumpMap)
, xac_texId4(lightMap)
, xf0_bumpLightDir(bumpLightDir)
, xfc_bumpScale(bumpScale)
, x100_tileSize(tileSize)
, x104_tileSubdivisions(tileSubdivisions & ~0x1)
, x108_rippleResolution(x100_tileSize / float(x104_tileSubdivisions))
, x10c_specularMin(specularMin)
, x110_specularMax(specularMax)
, x114_reflectionBlend(reflectionBlend)
, x118_reflectionSize(reflectionSize)
, x11c_unitsPerLightmapTexel(unitsPerLightmapTexel)
, x120_turbulence(turbSpeed, turbDistance, turbFreqMax, turbFreqMin, turbPhaseMax, turbPhaseMin, turbAmplitudeMax,
                  turbAmplitudeMin)
, m_maxVertCount(maxVertCount) {
  if (g_ResFactory->GetResourceTypeById(xa0_texIdBumpMap) == FOURCC('TXTR'))
    xb0_bumpMap = g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), xa0_texIdBumpMap});
  if (g_ResFactory->GetResourceTypeById(xa4_texIdEnvMap) == FOURCC('TXTR'))
    xc0_envMap = g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), xa4_texIdEnvMap});
  if (g_ResFactory->GetResourceTypeById(xa8_texIdEnvBumpMap) == FOURCC('TXTR'))
    xd0_envBumpMap = g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), xa8_texIdEnvBumpMap});
  if (g_ResFactory->GetResourceTypeById(xac_texId4) == FOURCC('TXTR'))
    xe0_lightmap = g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), xac_texId4});
}

void CFluidPlaneCPU::CreateRipple(const CRipple& ripple, CStateManager& mgr) {}

void CFluidPlaneCPU::CalculateLightmapMatrix(const zeus::CTransform& areaXf, const zeus::CTransform& xf,
                                             const zeus::CAABox& aabb, zeus::CMatrix4f& mtxOut) const {
  int width = GetLightMap().GetWidth();
  int height = GetLightMap().GetHeight();

  zeus::CTransform toLocal = areaXf.getRotation().inverse();
  zeus::CAABox areaLocalAABB = aabb.getTransformedAABox(toLocal);
  float f26 = (areaLocalAABB.max.x() - areaLocalAABB.min.x()) / (width * x11c_unitsPerLightmapTexel);
  float f25 = (areaLocalAABB.max.y() - areaLocalAABB.min.y()) / (height * x11c_unitsPerLightmapTexel);
  float f24 = (1.f + std::fmod(areaLocalAABB.min.x() + xf.origin.x(), x11c_unitsPerLightmapTexel)) / width;
  float f23 = (2.f - std::fmod(areaLocalAABB.max.x() + xf.origin.x(), x11c_unitsPerLightmapTexel)) / width;
  float f29 = (1.f + std::fmod(areaLocalAABB.min.y() + xf.origin.y(), x11c_unitsPerLightmapTexel)) / height;
  float f6 = (2.f - std::fmod(areaLocalAABB.max.y() + xf.origin.y(), x11c_unitsPerLightmapTexel)) / height;

  float scaleX = (f26 - f24 - f23) / (areaLocalAABB.max.x() - areaLocalAABB.min.x());
  float scaleY = -(f25 - f29 - f6) / (areaLocalAABB.max.y() - areaLocalAABB.min.y());
  float offX = f24 + f26 * -areaLocalAABB.min.x() / (areaLocalAABB.max.x() - areaLocalAABB.min.x());
  float offY = f25 * areaLocalAABB.min.y() / (areaLocalAABB.max.y() - areaLocalAABB.min.y()) - f6;
  mtxOut = (zeus::CTransform(zeus::CMatrix3f(zeus::CVector3f(scaleX, scaleY, 0.f)), zeus::CVector3f(offX, offY, 0.f)) *
            toLocal)
               .toMatrix4f();
}

static bool sSineWaveInitialized = false;
static float sGlobalSineWave[256] = {};
static const float* InitializeSineWave() {
  if (sSineWaveInitialized)
    return sGlobalSineWave;
  for (int i = 0; i < 256; ++i)
    sGlobalSineWave[i] = std::sin(2.f * M_PIF * (i / 256.f));
  sSineWaveInitialized = true;
  return sGlobalSineWave;
}

#define kEnableWaterBumpMaps true

CFluidPlaneShader::RenderSetupInfo CFluidPlaneCPU::RenderSetup(const CStateManager& mgr, float alpha,
                                                               const zeus::CTransform& xf,
                                                               const zeus::CTransform& areaXf, const zeus::CAABox& aabb,
                                                               const CScriptWater* water) const {
  CFluidPlaneShader::RenderSetupInfo out;

  float uvT = mgr.GetFluidPlaneManager()->GetUVT();
  bool hasBumpMap = HasBumpMap() && kEnableWaterBumpMaps;
  bool doubleLightmapBlend = false;
  bool hasEnvMap = mgr.GetCameraManager()->GetFluidCounter() == 0 && HasEnvMap();
  bool hasEnvBumpMap = HasEnvBumpMap();
  InitializeSineWave();
  CGraphics::SetModelMatrix(xf);

  if (hasBumpMap) {
    // Build 50% grey directional light with xf0_bumpLightDir and load into LIGHT_3
    // Light 3 in channel 1
    // Vertex colors in channel 0
    out.lights.resize(4);
    out.lights[3] = CLight::BuildDirectional(xf0_bumpLightDir, zeus::skGrey);
  } else {
    // Normal light mask in channel 1
    // Vertex colors in channel 0
    out.lights = water->GetActorLights()->BuildLightVector();
  }

  int curTex = 3;

  if (hasBumpMap) {
    // Load into next
    curTex++;
  }

  if (hasEnvMap) {
    // Load into next
    curTex++;
  }

  if (hasEnvBumpMap) {
    // Load into next
    curTex++;
  }

  float fluidUVs[3][2];
  x4c_uvMotion.CalculateFluidTextureOffset(uvT, fluidUVs);

  out.texMtxs[0][0][0] = x4c_uvMotion.GetFluidLayers()[1].GetUVScale();
  out.texMtxs[0][1][1] = x4c_uvMotion.GetFluidLayers()[1].GetUVScale();
  out.texMtxs[0][3][0] = fluidUVs[1][0];
  out.texMtxs[0][3][1] = fluidUVs[1][1];

  out.texMtxs[1][0][0] = x4c_uvMotion.GetFluidLayers()[2].GetUVScale();
  out.texMtxs[1][1][1] = x4c_uvMotion.GetFluidLayers()[2].GetUVScale();
  out.texMtxs[1][3][0] = fluidUVs[2][0];
  out.texMtxs[1][3][1] = fluidUVs[2][1];

  out.texMtxs[2][0][0] = x4c_uvMotion.GetFluidLayers()[0].GetUVScale();
  out.texMtxs[2][1][1] = x4c_uvMotion.GetFluidLayers()[0].GetUVScale();
  out.texMtxs[2][3][0] = fluidUVs[0][0];
  out.texMtxs[2][3][1] = fluidUVs[0][1];

  // Load normal mtx 0 with
  out.normMtx = (zeus::CTransform::Scale(xfc_bumpScale) * CGraphics::g_ViewMatrix.getRotation().inverse()).toMatrix4f();

  // Setup TCGs
  int nextTexMtx = 3;

  if (hasEnvBumpMap) {
    float pttScale;
    if (hasEnvMap)
      pttScale = 0.5f * (1.f - x118_reflectionSize);
    else
      pttScale = g_tweakGame->GetFluidEnvBumpScale() * x4c_uvMotion.GetFluidLayers()[0].GetUVScale();

    // Load GX_TEXMTX3 with identity
    zeus::CMatrix4f& texMtx = out.texMtxs[nextTexMtx++];
    texMtx[0][0] = pttScale;
    texMtx[1][1] = pttScale;
    texMtx[3][0] = 0.5f;
    texMtx[3][1] = 0.5f;
    // Load GX_PTTEXMTX0 with scale of pttScale
    // Next: GX_TG_MTX2x4 GX_TG_NRM, GX_TEXMTX3, true, GX_PTTEXMTX0

    out.indScale = 0.5f * (hasEnvMap ? x118_reflectionSize : 1.f);
    // Load ind mtx with scale of (indScale, -indScale)
    // Load envBumpMap into ind stage 0 with previous TCG
  }

  if (hasEnvMap) {
    float scale = std::max(aabb.max.x() - aabb.min.x(), aabb.max.y() - aabb.min.y());
    zeus::CMatrix4f& texMtx = out.texMtxs[nextTexMtx++];
    texMtx[0][0] = 1.f / scale;
    texMtx[1][1] = 1.f / scale;
    zeus::CVector3f center = aabb.center();
    texMtx[3][0] = 0.5f + -center.x() / scale;
    texMtx[3][1] = 0.5f + -center.y() / scale;
    // Next: GX_TG_MTX2x4 GX_TG_POS, mtxNext, false, GX_PTIDENTITY
  }

  if (HasLightMap()) {
    float lowLightBlend = 1.f;
    const CGameArea* area = mgr.GetWorld()->GetAreaAlways(mgr.GetNextAreaId());
    float lightLevel = area->GetPostConstructed()->x1128_worldLightingLevel;
    const CScriptWater* nextWater = water->GetNextConnectedWater(mgr);
    if (std::fabs(water->GetMorphFactor()) < 0.00001f || !nextWater || !nextWater->GetFluidPlane().HasLightMap()) {
      // Load lightmap
      CalculateLightmapMatrix(areaXf, xf, aabb, out.texMtxs[nextTexMtx++]);
      // Next: GX_TG_MTX2x4 GX_TG_POS, mtxNext, false, GX_PTIDENTITY
    } else if (nextWater && nextWater->GetFluidPlane().HasLightMap()) {
      if (std::fabs(water->GetMorphFactor() - 1.f) < 0.00001f) {
        // Load lightmap
        CalculateLightmapMatrix(areaXf, xf, aabb, out.texMtxs[nextTexMtx++]);
        // Next: GX_TG_MTX2x4 GX_TG_POS, mtxNext, false, GX_PTIDENTITY
      } else {
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

  float waterPlaneOrthoDot = xf.transposeRotate(zeus::skUp)
                                 .dot(CGraphics::g_ViewMatrix.inverse().transposeRotate(zeus::skForward));
  if (waterPlaneOrthoDot < 0.f)
    waterPlaneOrthoDot = -waterPlaneOrthoDot;

  out.kColors[0] =
      zeus::CColor((1.f - waterPlaneOrthoDot) * (x110_specularMax - x10c_specularMin) + x10c_specularMin, alpha);
  out.kColors[1] = zeus::CColor(x114_reflectionBlend, 1.f);

  if (!m_shader || m_cachedDoubleLightmapBlend != doubleLightmapBlend ||
      m_cachedAdditive != (mgr.GetThermalDrawFlag() == EThermalDrawFlag::Hot)) {
    m_cachedDoubleLightmapBlend = doubleLightmapBlend;
    m_cachedAdditive = mgr.GetThermalDrawFlag() == EThermalDrawFlag::Hot;
    m_shader.emplace(x44_fluidType, x10_texPattern1, x20_texPattern2, x30_texColor, xb0_bumpMap, xc0_envMap,
                     xd0_envBumpMap, xe0_lightmap,
                     m_tessellation ? CFluidPlaneManager::RippleMapTex : boo::ObjToken<boo::ITextureS>{},
                     m_cachedDoubleLightmapBlend, m_cachedAdditive, m_maxVertCount);
  }

  return out;
}

int CFluidPlaneRender::numTilesInHField;
int CFluidPlaneRender::numSubdivisionsInTile;
int CFluidPlaneRender::numSubdivisionsInHField;

bool CFluidPlaneCPU::PrepareRipple(const CRipple& ripple, const CFluidPlaneRender::SPatchInfo& info,
                                   CFluidPlaneRender::SRippleInfo& rippleOut) {
  auto lifeIdx = int((1.f - (ripple.GetTimeFalloff() - ripple.GetTime()) / ripple.GetTimeFalloff()) * 64.f);
  float dist = CFluidPlaneManager::RippleMaxs[lifeIdx] * (ripple.GetDistanceFalloff() / 256.f);
  dist *= dist;
  if (dist != 0)
    dist = std::sqrt(dist);
  dist = info.x24_ooRippleResolution * dist + 1.f;
  float centerX = info.x24_ooRippleResolution * (ripple.GetCenter().x() - info.xc_globalMin.x());
  float centerY = info.x24_ooRippleResolution * (ripple.GetCenter().y() - info.xc_globalMin.y());
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

void CFluidPlaneCPU::ApplyTurbulence(float t, CFluidPlaneRender::SHFieldSample (&heights)[46][46],
                                     const u8 (&flags)[9][9], const float sineWave[256],
                                     const CFluidPlaneRender::SPatchInfo& info,
                                     const zeus::CVector3f& areaCenter) const {
  if (!HasTurbulence()) {
    memset(&heights, 0, sizeof(heights));
    return;
  }

  float scaledT = t * GetOOTurbulenceSpeed();
  float curY = info.x4_localMin.y() - info.x18_rippleResolution - areaCenter.y();
  int xDivs = (info.x0_xSubdivs + CFluidPlaneRender::numSubdivisionsInTile - 4) /
                  CFluidPlaneRender::numSubdivisionsInTile * CFluidPlaneRender::numSubdivisionsInTile +
              2;
  int yDivs = (info.x1_ySubdivs + CFluidPlaneRender::numSubdivisionsInTile - 4) /
                  CFluidPlaneRender::numSubdivisionsInTile * CFluidPlaneRender::numSubdivisionsInTile +
              2;
  for (int i = 0; i <= yDivs; ++i) {
    float curYSq = curY * curY;
    float curX = info.x4_localMin.x() - info.x18_rippleResolution - areaCenter.x();
    for (int j = 0; j <= xDivs; ++j) {
      float distFac = curX * curX + curYSq;
      if (distFac != 0.f)
        distFac = std::sqrt(distFac);
      heights[i][j].height = GetTurbulenceHeight(GetOOTurbulenceDistance() * distFac + scaledT);
      curX += info.x18_rippleResolution;
    }
    curY += info.x18_rippleResolution;
  }
}

void CFluidPlaneCPU::ApplyRipple(const CFluidPlaneRender::SRippleInfo& rippleInfo,
                                 CFluidPlaneRender::SHFieldSample (&heights)[46][46], u8 (&flags)[9][9],
                                 const float sineWave[256], const CFluidPlaneRender::SPatchInfo& info) const {
  float lookupT = 256.f *
                  (1.f - rippleInfo.x0_ripple.GetTime() * rippleInfo.x0_ripple.GetOOTimeFalloff() *
                             rippleInfo.x0_ripple.GetOOTimeFalloff()) *
                  rippleInfo.x0_ripple.GetFrequency();
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
  int fromY =
      (rippleInfo.x1c_gfromY + CFluidPlaneRender::numSubdivisionsInTile - 1) / CFluidPlaneRender::numSubdivisionsInTile;
  int fromX =
      (rippleInfo.x14_gfromX + CFluidPlaneRender::numSubdivisionsInTile - 1) / CFluidPlaneRender::numSubdivisionsInTile;
  int toY =
      (rippleInfo.x20_gtoY + CFluidPlaneRender::numSubdivisionsInTile - 1) / CFluidPlaneRender::numSubdivisionsInTile;
  int toX =
      (rippleInfo.x18_gtoX + CFluidPlaneRender::numSubdivisionsInTile - 1) / CFluidPlaneRender::numSubdivisionsInTile;

  float curY = rippleInfo.x0_ripple.GetCenter().y() - info.xc_globalMin.y() -
               (0.5f * info.x14_tileSize + (fromY - 1) * info.x14_tileSize);
  int curGridY = info.x2a_gridDimX * (info.x2e_tileY + fromY - 1);
  int startGridX = (info.x28_tileX + fromX - 1);
  int gridCells = info.x2a_gridDimX * info.x2c_gridDimY;
  float distFalloff = 64.f * rippleInfo.x0_ripple.GetOODistanceFalloff();
  int curYDiv = rippleInfo.xc_fromY;

  for (int i = fromY; i <= toY; ++i, curY -= info.x14_tileSize) {
    int nextYDiv = (i + 1) * CFluidPlaneRender::numSubdivisionsInTile;
    float curYSq = curY * curY;
    int curGridX = startGridX;
    int curXDiv = rippleInfo.x4_fromX;
    float curX = rippleInfo.x0_ripple.GetCenter().x() - info.xc_globalMin.x() -
                 (0.5f * info.x14_tileSize + (fromX - 1) * info.x14_tileSize);
    for (int j = fromX; j <= toX; ++j, curX -= info.x14_tileSize, ++curGridX) {
      float dist = curX * curX + curYSq;
      if (dist != 0.f)
        dist = std::sqrt(dist);
      if (maxDist < dist - info.x1c_tileHypRadius || minDist > dist + info.x1c_tileHypRadius)
        continue;

      bool addedRipple = false;
      int nextXDiv = (j + 1) * CFluidPlaneRender::numSubdivisionsInTile;
      float curXMod =
          (rippleInfo.x0_ripple.GetCenter().x() - info.xc_globalMin.x()) - info.x18_rippleResolution * curXDiv;
      float curYMod =
          (rippleInfo.x0_ripple.GetCenter().y() - info.xc_globalMin.y()) - info.x18_rippleResolution * curYDiv;

      if (!info.x30_gridFlags || (info.x30_gridFlags && curGridY >= 0 && curGridY < gridCells && curGridX >= 0 &&
                                  curGridX < info.x2a_gridDimX && info.x30_gridFlags[curGridX + curGridY])) {
        for (int k = curYDiv; k <= std::min(rippleInfo.x10_toY, nextYDiv - 1);
             ++k, curYMod -= info.x18_rippleResolution) {
          float tmpXMod = curXMod;
          float curYModSq = curYMod * curYMod;
          for (int l = curXDiv; l <= std::min(rippleInfo.x8_toX, nextXDiv - 1);
               ++l, tmpXMod -= info.x18_rippleResolution) {
            float divDistSq = tmpXMod * tmpXMod + curYModSq;
            if (divDistSq < minDistSq || divDistSq > maxDistSq)
              continue;

            if (m_tessellation) {
              /* This will be evaluated in tessellation shader instead */
              addedRipple = true;
              break;
            }

            float divDist = (divDistSq != 0.f) ? std::sqrt(divDistSq) : 0.f;
            if (u8 rippleV = CFluidPlaneManager::RippleValues[lifeIdx][int(divDist * distFalloff)]) {
              heights[k][l].height += rippleV * rippleInfo.x0_ripple.GetLookupAmplitude() *
                                      sineWave[int(divDist * rippleInfo.x0_ripple.GetLookupPhase() + lookupT) & 0xff];
            } else {
              heights[k][l].height += 0.f;
            }
            addedRipple = true;
          }
        }

        if (addedRipple)
          flags[i][j] = 0x1f;
      } else {
        int yMin = nextYDiv - 1;
        int yMax = nextYDiv - CFluidPlaneRender::numSubdivisionsInTile + 1;
        int xMin = nextXDiv - 1;
        int xMax = nextXDiv - CFluidPlaneRender::numSubdivisionsInTile + 1;

        if (curGridX >= 0.f && curGridX < info.x2a_gridDimX && curGridY - info.x2a_gridDimX >= 0 &&
            !info.x30_gridFlags[curGridX + curGridY - info.x2a_gridDimX])
          yMax -= 2;

        if (curGridX >= 0.f && curGridX < info.x2a_gridDimX && curGridY + info.x2a_gridDimX < gridCells &&
            !info.x30_gridFlags[curGridX + info.x2a_gridDimX])
          yMin += 2;

        if (curGridY >= 0 && curGridY < info.x2c_gridDimY && curGridX > 0 && !info.x30_gridFlags[curGridX - 1])
          xMax -= 2;

        if (curGridY >= 0 && curGridY < info.x2c_gridDimY && curGridX + 1 < info.x2a_gridDimX &&
            !info.x30_gridFlags[curGridX + 1])
          xMin += 2;

        for (int k = curYDiv; k <= std::min(rippleInfo.x10_toY, nextYDiv - 1);
             ++k, curYMod -= info.x18_rippleResolution) {
          float tmpXMod = curXMod;
          float curYModSq = curYMod * curYMod;
          for (int l = curXDiv; l <= std::min(rippleInfo.x8_toX, nextXDiv - 1);
               ++l, tmpXMod -= info.x18_rippleResolution) {
            if (k <= yMax || k >= yMin || l <= xMax || l >= xMin) {
              float divDistSq = tmpXMod * tmpXMod + curYModSq;
              if (divDistSq < minDistSq || divDistSq > maxDistSq)
                continue;

              if (m_tessellation) {
                /* This will be evaluated in tessellation shader instead */
                addedRipple = true;
                break;
              }

              float divDist = (divDistSq != 0.f) ? std::sqrt(divDistSq) : 0.f;
              if (u8 rippleV = CFluidPlaneManager::RippleValues[lifeIdx][int(divDist * distFalloff)]) {
                heights[k][l].height += rippleV * rippleInfo.x0_ripple.GetLookupAmplitude() *
                                        sineWave[int(divDist * rippleInfo.x0_ripple.GetLookupPhase() + lookupT) & 0xff];
              } else {
                heights[k][l].height += 0.f;
              }
              addedRipple = true;
            }
          }

          if (m_tessellation && addedRipple)
            break;
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

void CFluidPlaneCPU::ApplyRipples(const rstl::reserved_vector<CFluidPlaneRender::SRippleInfo, 32>& rippleInfos,
                                  CFluidPlaneRender::SHFieldSample (&heights)[46][46], u8 (&flags)[9][9],
                                  const float sineWave[256], const CFluidPlaneRender::SPatchInfo& info) const {
  for (const CFluidPlaneRender::SRippleInfo& rippleInfo : rippleInfos)
    ApplyRipple(rippleInfo, heights, flags, sineWave, info);
  for (int i = 0; i < CFluidPlaneRender::numTilesInHField; ++i)
    flags[0][i + 1] |= 1;
  for (int i = 0; i < CFluidPlaneRender::numTilesInHField; ++i)
    flags[i + 1][0] |= 8;
  for (int i = 0; i < CFluidPlaneRender::numTilesInHField; ++i)
    flags[i + 1][CFluidPlaneRender::numTilesInHField + 1] |= 4;
  for (int i = 0; i < CFluidPlaneRender::numTilesInHField; ++i)
    flags[CFluidPlaneRender::numTilesInHField + 1][i + 1] |= 2;
}

void CFluidPlaneCPU::UpdatePatchNoNormals(CFluidPlaneRender::SHFieldSample (&heights)[46][46], const u8 (&flags)[9][9],
                                          const CFluidPlaneRender::SPatchInfo& info) {
  for (int i = 1; i <= (info.x1_ySubdivs + CFluidPlaneRender::numSubdivisionsInTile - 2) /
                           CFluidPlaneRender::numSubdivisionsInTile;
       ++i) {
    int r10 = i * CFluidPlaneRender::numSubdivisionsInTile + 1;
    int r9 = std::max(0, r10 - CFluidPlaneRender::numSubdivisionsInTile);
    int x24 = std::min(r10, info.x1_ySubdivs + 1);
    for (int j = 1; j <= (info.x0_xSubdivs + CFluidPlaneRender::numSubdivisionsInTile - 2) /
                             CFluidPlaneRender::numSubdivisionsInTile;
         ++j) {
      int r29 = j * CFluidPlaneRender::numSubdivisionsInTile + 1;
      int r11 = std::max(0, r29 - CFluidPlaneRender::numSubdivisionsInTile);
      int x28 = std::min(r29, info.x0_xSubdivs + 1);
      if ((flags[i][j] & 0x1f) == 0x1f) {
        for (int k = r9; k < x24; ++k) {
          for (int l = r11; l < x28; ++l) {
            CFluidPlaneRender::SHFieldSample& sample = heights[k][l];
            if (sample.height > 0.f)
              sample.wavecapIntensity = u8(std::min(255, int(info.x38_wavecapIntensityScale * sample.height)));
            else
              sample.wavecapIntensity = 0;
          }
        }
      } else {
        if (i > 0 && i < CFluidPlaneRender::numTilesInHField + 1 && j > 0 &&
            j < CFluidPlaneRender::numTilesInHField + 1) {
          int halfSubdivs = CFluidPlaneRender::numSubdivisionsInTile / 2;
          CFluidPlaneRender::SHFieldSample& sample = heights[halfSubdivs + r9][halfSubdivs + r11];
          if (sample.height > 0.f)
            sample.wavecapIntensity = u8(std::min(255, int(info.x38_wavecapIntensityScale * sample.height)));
          else
            sample.wavecapIntensity = 0;
        }

        if (i != 0) {
          for (int l = r11; l < x28; ++l) {
            CFluidPlaneRender::SHFieldSample& sample = heights[r9][l];
            if (sample.height > 0.f)
              sample.wavecapIntensity = u8(std::min(255, int(info.x38_wavecapIntensityScale * sample.height)));
            else
              sample.wavecapIntensity = 0;
          }
        }

        if (j != 0) {
          for (int k = r9 + 1; k < x24; ++k) {
            CFluidPlaneRender::SHFieldSample& sample = heights[k][r11];
            if (sample.height > 0.f)
              sample.wavecapIntensity = u8(std::min(255, int(info.x38_wavecapIntensityScale * sample.height)));
            else
              sample.wavecapIntensity = 0;
          }
        }
      }
    }
  }
}

void CFluidPlaneCPU::UpdatePatchWithNormals(CFluidPlaneRender::SHFieldSample (&heights)[46][46],
                                            const u8 (&flags)[9][9], const CFluidPlaneRender::SPatchInfo& info) {
  float normalScale = -(2.f * info.x18_rippleResolution);
  float nz = 0.25f * 2.f * info.x18_rippleResolution;
  int curGridY = info.x2e_tileY * info.x2a_gridDimX - 1 + info.x28_tileX;
  for (int i = 1; i <= (info.x1_ySubdivs + CFluidPlaneRender::numSubdivisionsInTile - 2) /
                           CFluidPlaneRender::numSubdivisionsInTile;
       ++i, curGridY += info.x2a_gridDimX) {
    int r11 = i * CFluidPlaneRender::numSubdivisionsInTile + 1;
    int r9 = std::max(0, r11 - CFluidPlaneRender::numSubdivisionsInTile);
    int x38 = std::min(r11, info.x1_ySubdivs + 1);
    for (int j = 1; j <= (info.x0_xSubdivs + CFluidPlaneRender::numSubdivisionsInTile - 2) /
                             CFluidPlaneRender::numSubdivisionsInTile;
         ++j) {
      int r12 = j * CFluidPlaneRender::numSubdivisionsInTile + 1;
      int x3c = std::min(r12, info.x0_xSubdivs + 1);
      r12 -= CFluidPlaneRender::numSubdivisionsInTile;
      if ((flags[i][j] & 0x1f) == 0x1f) {
        for (int k = r9; k < x38; ++k) {
          for (int l = r12; l < x3c; ++l) {
            CFluidPlaneRender::SHFieldSample& sample = heights[k][l];
            CFluidPlaneRender::SHFieldSample& up = heights[k + 1][l];
            CFluidPlaneRender::SHFieldSample& down = heights[k - 1][l];
            CFluidPlaneRender::SHFieldSample& right = heights[k][l + 1];
            CFluidPlaneRender::SHFieldSample& left = heights[k][l - 1];
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
              sample.wavecapIntensity = u8(std::min(255, int(info.x38_wavecapIntensityScale * sample.height)));
            else
              sample.wavecapIntensity = 0;
          }
        }
      } else {
        if (!info.x30_gridFlags || info.x30_gridFlags[curGridY + j]) {
          if (i > 0 && i < CFluidPlaneRender::numTilesInHField + 1 && j > 0 &&
              j < CFluidPlaneRender::numTilesInHField + 1) {
            int halfSubdivs = CFluidPlaneRender::numSubdivisionsInTile / 2;
            int k = halfSubdivs + r9;
            int l = halfSubdivs + r12;
            CFluidPlaneRender::SHFieldSample& sample = heights[k][l];
            CFluidPlaneRender::SHFieldSample& up = heights[k + 1][l];
            CFluidPlaneRender::SHFieldSample& down = heights[k - 1][l];
            CFluidPlaneRender::SHFieldSample& right = heights[k][l + 1];
            CFluidPlaneRender::SHFieldSample& left = heights[k][l - 1];
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
              sample.wavecapIntensity = u8(std::min(255, int(info.x38_wavecapIntensityScale * sample.height)));
            else
              sample.wavecapIntensity = 0;
          }
        }

        if (j != 0 && i != 0) {
          if ((flags[i][j] & 2) != 0 || (flags[i - 1][j] & 1) != 0 || (flags[i][j] & 4) != 0 ||
              (flags[i][j - 1] & 8) != 0) {
            for (int l = r12; l < x3c; ++l) {
              CFluidPlaneRender::SHFieldSample& sample = heights[r9][l];
              CFluidPlaneRender::SHFieldSample& up = heights[r9 + 1][l];
              CFluidPlaneRender::SHFieldSample& down = heights[r9 - 1][l];
              CFluidPlaneRender::SHFieldSample& right = heights[r9][l + 1];
              CFluidPlaneRender::SHFieldSample& left = heights[r9][l - 1];
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
                sample.wavecapIntensity = u8(std::min(255, int(info.x38_wavecapIntensityScale * sample.height)));
              else
                sample.wavecapIntensity = 0;
            }

            for (int k = r9; k < x38; ++k) {
              CFluidPlaneRender::SHFieldSample& sample = heights[k][r12];
              CFluidPlaneRender::SHFieldSample& up = heights[k + 1][r12];
              CFluidPlaneRender::SHFieldSample& down = heights[k - 1][r12];
              CFluidPlaneRender::SHFieldSample& right = heights[k][r12 + 1];
              CFluidPlaneRender::SHFieldSample& left = heights[k][r12 - 1];
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
                sample.wavecapIntensity = u8(std::min(255, int(info.x38_wavecapIntensityScale * sample.height)));
              else
                sample.wavecapIntensity = 0;
            }
          } else {
            CFluidPlaneRender::SHFieldSample& sample = heights[r9][r12];
            CFluidPlaneRender::SHFieldSample& up = heights[r9 + 1][r12];
            CFluidPlaneRender::SHFieldSample& down = heights[r9 - 1][r12];
            CFluidPlaneRender::SHFieldSample& right = heights[r9][r12 + 1];
            CFluidPlaneRender::SHFieldSample& left = heights[r9][r12 - 1];
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
              sample.wavecapIntensity = u8(std::min(255, int(info.x38_wavecapIntensityScale * sample.height)));
            else
              sample.wavecapIntensity = 0;
          }
        }
      }
    }
  }
}

bool CFluidPlaneCPU::UpdatePatch(float time, const CFluidPlaneRender::SPatchInfo& info,
                                 CFluidPlaneRender::SHFieldSample (&heights)[46][46], u8 (&flags)[9][9],
                                 const zeus::CVector3f& areaCenter,
                                 const std::optional<CRippleManager>& rippleManager, int fromX, int toX,
                                 int fromY, int toY) const {
  rstl::reserved_vector<CFluidPlaneRender::SRippleInfo, 32> rippleInfos;
  if (rippleManager) {
    for (const CRipple& ripple : rippleManager->GetRipples()) {
      if (ripple.GetTime() >= ripple.GetTimeFalloff())
        continue;
      CFluidPlaneRender::SRippleInfo rippleInfo(ripple, fromX, toX, fromY, toY);
      if (PrepareRipple(ripple, info, rippleInfo))
        rippleInfos.push_back(rippleInfo);
    }
  }

  if (rippleInfos.empty())
    return true;

  ApplyTurbulence(time, heights, flags, sGlobalSineWave, info, areaCenter);
  ApplyRipples(rippleInfos, heights, flags, sGlobalSineWave, info);

  /* No further action necessary if using tessellation shaders */
  if (m_tessellation)
    return false;

  if (info.x37_normalMode == CFluidPlaneRender::NormalMode::NoNormals)
    UpdatePatchNoNormals(heights, flags, info);
  else
    UpdatePatchWithNormals(heights, flags, info);

  return false;
}

/* Used to be part of locked cache
 * These are too big for stack allocation */
static CFluidPlaneRender::SHFieldSample lc_heights[46][46] = {};
static u8 lc_flags[9][9] = {};

void CFluidPlaneCPU::Render(const CStateManager& mgr, float alpha, const zeus::CAABox& aabb, const zeus::CTransform& xf,
                            const zeus::CTransform& areaXf, bool noNormals, const zeus::CFrustum& frustum,
                            const std::optional<CRippleManager>& rippleManager, TUniqueId waterId,
                            const bool* gridFlags, u32 gridDimX, u32 gridDimY,
                            const zeus::CVector3f& areaCenter) const {
  TCastToConstPtr<CScriptWater> water = mgr.GetObjectById(waterId);
  CFluidPlaneShader::RenderSetupInfo setupInfo = RenderSetup(mgr, alpha, xf, areaXf, aabb, water.GetPtr());

  CFluidPlaneRender::NormalMode normalMode;
  if (xb0_bumpMap && kEnableWaterBumpMaps)
    normalMode = CFluidPlaneRender::NormalMode::NBT;
  else if (!noNormals)
    normalMode = CFluidPlaneRender::NormalMode::Normals;
  else
    normalMode = CFluidPlaneRender::NormalMode::NoNormals;

  // Set Position and color format

  switch (normalMode) {
  case CFluidPlaneRender::NormalMode::NBT:
    // Set NBT format
    break;
  case CFluidPlaneRender::NormalMode::Normals:
    // Set Normal format
    break;
  default:
    break;
  }

  float rippleResolutionRecip = 1.f / x108_rippleResolution;
  CFluidPlaneRender::numSubdivisionsInTile = x104_tileSubdivisions;
  CFluidPlaneRender::numTilesInHField = std::min(7, 42 / CFluidPlaneRender::numSubdivisionsInTile);
  CFluidPlaneRender::numSubdivisionsInHField =
      CFluidPlaneRender::numTilesInHField * CFluidPlaneRender::numSubdivisionsInTile;

  zeus::CVector2f ripplePitch(x108_rippleResolution * CFluidPlaneRender::numSubdivisionsInHField);

  // Amount to shift intensity values right (for added wavecap color)
  int redShift = 0;
  int greenShift = 0;
  int blueShift = 0;
  float wavecapIntensityScale = g_tweakGame->GetWavecapIntensityNormal();
  switch (x44_fluidType) {
  case EFluidType::PoisonWater:
    wavecapIntensityScale = g_tweakGame->GetWavecapIntensityPoison();
    redShift = 1;
    blueShift = 1;
    break;
  case EFluidType::Lava:
  case EFluidType::ThickLava:
    wavecapIntensityScale = g_tweakGame->GetWavecapIntensityLava();
    blueShift = 8;
    greenShift = 8;
    break;
  default:
    break;
  }

  if (water) {
    float cameraPenetration =
        mgr.GetCameraManager()->GetCurrentCamera(mgr)->GetTranslation().dot(zeus::skUp) -
        water->GetTriggerBoundsWR().max.z();
    wavecapIntensityScale *= (cameraPenetration >= 0.5f || cameraPenetration < 0.f) ? 1.f : 2.f * cameraPenetration;
  }

  u32 patchDimX = (water && water->GetPatchDimensionX()) ? water->GetPatchDimensionX() : 128;
  u32 patchDimY = (water && water->GetPatchDimensionY()) ? water->GetPatchDimensionY() : 128;

  m_verts.clear();
  m_pVerts.clear();
  if (m_tessellation) {
    /* Additional uniform data for tessellation evaluation shader */
    zeus::CColor colorMul;
    colorMul.r() = wavecapIntensityScale / 255.f / float(1 << redShift);
    colorMul.g() = wavecapIntensityScale / 255.f / float(1 << greenShift);
    colorMul.b() = wavecapIntensityScale / 255.f / float(1 << blueShift);
    m_shader->prepareDraw(setupInfo, xf.origin, *rippleManager, colorMul, x108_rippleResolution / 4.f);
  } else {
    m_shader->prepareDraw(setupInfo);
  }

  u32 tileY = 0;
  float curY = aabb.min.y();
  for (int i = 0; curY < aabb.max.y() && i < patchDimY; ++i) {
    u32 tileX = 0;
    float curX = aabb.min.x();
    float _remDivsY = (aabb.max.y() - curY) * rippleResolutionRecip;
    for (int j = 0; curX < aabb.max.x() && j < patchDimX; ++j) {
      if (u8 renderFlags = water->GetPatchRenderFlags(j, i)) {
        s16 remDivsX = std::min(s16((aabb.max.x() - curX) * rippleResolutionRecip),
                                s16(CFluidPlaneRender::numSubdivisionsInHField));
        s16 remDivsY = std::min(s16(_remDivsY), s16(CFluidPlaneRender::numSubdivisionsInHField));
        zeus::CVector3f localMax(x108_rippleResolution * remDivsX + curX, x108_rippleResolution * remDivsY + curY,
                                 aabb.max.z());
        zeus::CVector3f localMin(curX, curY, aabb.min.z());
        zeus::CAABox testaabb(localMin + xf.origin, localMax + xf.origin);
        if (frustum.aabbFrustumTest(testaabb)) {
          CFluidPlaneRender::SPatchInfo info(localMin, localMax, xf.origin, x108_rippleResolution, x100_tileSize,
                                             wavecapIntensityScale, CFluidPlaneRender::numSubdivisionsInHField,
                                             normalMode, redShift, greenShift, blueShift, tileX, gridDimX, gridDimY,
                                             tileY, gridFlags);

          int fromX = tileX != 0 ? (2 - CFluidPlaneRender::numSubdivisionsInTile) : 0;
          int toX;
          if (tileX != gridDimX - 1)
            toX = info.x0_xSubdivs + (CFluidPlaneRender::numSubdivisionsInTile - 2);
          else
            toX = info.x0_xSubdivs;

          int fromY = tileY != 0 ? (2 - CFluidPlaneRender::numSubdivisionsInTile) : 0;
          int toY;
          if (tileY != gridDimY - 1)
            toY = info.x1_ySubdivs + (CFluidPlaneRender::numSubdivisionsInTile - 2);
          else
            toY = info.x1_ySubdivs;

          bool noRipples = UpdatePatch(mgr.GetFluidPlaneManager()->GetUVT(), info, lc_heights, lc_flags, areaCenter,
                                       rippleManager, fromX, toX, fromY, toY);
          RenderPatch(info, lc_heights, lc_flags, noRipples, renderFlags == 1, m_verts, m_pVerts);
        }
      }
      curX += ripplePitch.x();
      tileX += CFluidPlaneRender::numTilesInHField;
    }
    curY += ripplePitch.y();
    tileY += CFluidPlaneRender::numTilesInHField;
  }

  m_shader->loadVerts(m_verts, m_pVerts);
  m_shader->doneDrawing();
}

} // namespace urde
