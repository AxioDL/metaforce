#include "Runtime/World/CFluidPlaneCPU.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Camera/CGameCamera.hpp"
#include "Runtime/World/CFluidPlaneManager.hpp"
#include "Runtime/World/CScriptWater.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path
#include "Graphics/CCubeRenderer.hpp"
#include "Graphics/CGX.hpp"

namespace metaforce {
constexpr u32 kTableSize = 2048;

struct STexMtx24 {
  float m[2][4];
};
struct SIndMtx23 {
  float m[2][3];
};
struct SEnvMtx34 {
  float m[3][4];
};
struct SFluidTexMtxTable {
  STexMtx24 pad;
  STexMtx24 color;
  STexMtx24 pattern1;
  STexMtx24 pattern2;
  SIndMtx23 ind;
  SEnvMtx34 env;
};
static const SFluidTexMtxTable kTexMtxTable = {};

static u8 sFluidSetupInitOnce;
static u8 sFluidSetupDone;
static int kMaxTilesInHField = 7;

constexpr bool sRenderFog = true;
constexpr bool sRenderBumpMaps = true;
constexpr int sFluidEnvMapType = 2;

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
  if (x18_amplitudeMax == 0.f && x1c_amplitudeMin == 0.f) {
    return;
  }

  x24_tableCount = kTableSize;
  x28_heightSelPitch = x24_tableCount;
  x20_table.reset(new float[x24_tableCount]);
  const float anglePitch = 2.f * M_PIF / x28_heightSelPitch;
  const float freqConstant = 0.5f * (x8_freqMax + xc_freqMin);
  const float freqLinear = 0.5f * (x8_freqMax - xc_freqMin);
  const float phaseConstant = 0.5f * (x10_phaseMax + x14_phaseMin);
  const float phaseLinear = 0.5f * (x10_phaseMax - x14_phaseMin);
  const float amplitudeConstant = 0.5f * (x18_amplitudeMax + x1c_amplitudeMin);
  const float amplitudeLinear = 0.5f * (x18_amplitudeMax - x1c_amplitudeMin);

  float curAng = 0.f;
  for (size_t i = 0; i < x24_tableCount; ++i, curAng += anglePitch) {
    const float angCos = std::cos(curAng);
    x20_table[i] = (amplitudeLinear * angCos + amplitudeConstant) *
                   std::sin((freqLinear * angCos + freqConstant) * curAng + (phaseLinear * angCos + phaseConstant));
  }

  x34_hasTurbulence = true;
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
, x108_rippleResolution(x100_tileSize / static_cast<float>(x104_tileSubdivisions))
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
                                             const zeus::CAABox& aabb, int idx) const {
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

  zeus::CTransform result =
      zeus::CTransform(zeus::CMatrix3f(zeus::CVector3f(scaleX, scaleY, 0.f)), zeus::CVector3f(offX, offY, 0.f)) *
      toLocal;
  float mtx[2][4];
  result.toCStyleMatrix(mtx);
  CGX::LoadTexMtxImm(mtx, idx, GX_MTX2x4);
}

static bool sSineWaveInitialized = false;
static CFluidPlaneCPU::SineTable sGlobalSineWave{};
static void InitializeSineWave() {
  if (sSineWaveInitialized) {
    return;
  }
  for (size_t i = 0; i < sGlobalSineWave.size(); ++i) {
    sGlobalSineWave[i] = std::sin(2.f * M_PIF * (static_cast<float>(i) / 256.f));
  }
  sSineWaveInitialized = true;
}

void CFluidPlaneCPU::RenderSetup(const CStateManager& mgr, float alpha, const zeus::CTransform& xf,
                                 const zeus::CTransform& areaXf, const zeus::CAABox& aabb, const CScriptWater* water) {
  const SFluidTexMtxTable* tbl = &kTexMtxTable;

  if (!sRenderFog) {
    return;
  }

  bool hasBumpMap = false;
  bool hasDoubleLightmap = false;
  float uvT = mgr.GetFluidPlaneManager()->GetUVT();
  if (HasBumpMap() && sRenderBumpMaps) {
    hasBumpMap = true;
  }
  bool hasLightmap = HasLightMap();
  int envMapType;
  if (mgr.GetCameraManager()->GetFluidCounter() != 0) {
    envMapType = 0;
  } else {
    bool hasEnv = HasEnvMap();
    envMapType = sFluidEnvMapType & ((-hasEnv | hasEnv) >> 31);
  }
  bool hasEnvBumpMap = HasEnvBumpMap();

  InitializeSineWave();

  g_Renderer->SetModelMatrix(xf);

  constexpr GXColor ambColor = {0, 0, 0, 0};
  constexpr GXColor white = {0xff, 0xff, 0xff, 0xff};

  if (hasBumpMap) {
    CColor bumpLightColor(0.5f, 0.5f, 0.5f, 1.f);
    CLight bumpLight = CLight::BuildDirectional(GetBumpLightDir().normalized(), bumpLightColor);
    CGraphics::LoadLight(3, bumpLight);
    CGX::SetNumChans(2);
    CGX::SetChanCtrl(CGX::EChannelId::Channel1, true, GX_SRC_REG, GX_SRC_REG, GX_LIGHT3, GX_DF_CLAMP, GX_AF_SPOT);
    CGX::SetChanMatColor(CGX::EChannelId::Channel1, white);
    CGX::SetChanAmbColor(CGX::EChannelId::Channel1, ambColor);
    CGX::SetChanCtrl(CGX::EChannelId::Channel0, true, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_SPOT);
    CGX::SetChanMatColor(CGX::EChannelId::Channel0, white);
    CGX::SetChanAmbColor(CGX::EChannelId::Channel0, ambColor);
  } else {
    CGX::SetNumChans(2);
    CGX::SetChanCtrl(CGX::EChannelId::Channel1, true, GX_SRC_REG, GX_SRC_REG, CGraphics::GetLightMask(), GX_DF_CLAMP,
                     GX_AF_SPOT);
    CGX::SetChanMatColor(CGX::EChannelId::Channel1, CGraphics::GetLightMask() != 0 ? white : ambColor);
    if (hasLightmap) {
      CGX::SetChanAmbColor(CGX::EChannelId::Channel1, ambColor);
    }
    CGX::SetChanCtrl(CGX::EChannelId::Channel0, true, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_SPOT);
    CGX::SetChanMatColor(CGX::EChannelId::Channel0, white);
    CGX::SetChanAmbColor(CGX::EChannelId::Channel0, ambColor);
  }

  int nextTexMap = 0;
  int nextCoord = 0;

  int texMapIds[8];
  int texCoordIds[8];

  if (HasTexturePattern1()) {
    const_cast<CTexture&>(GetTexturePattern1()).Load(static_cast<GXTexMapID>(nextTexMap), EClampMode::Repeat);
  } else {
    const_cast<CTexture&>(g_Renderer->GetZeroTexture()).Load(static_cast<GXTexMapID>(nextTexMap), EClampMode::Repeat);
  }
  texMapIds[0] = nextTexMap;
  nextTexMap = 1;

  if (HasTexturePattern2()) {
    const_cast<CTexture&>(GetTexturePattern2()).Load(static_cast<GXTexMapID>(nextTexMap), EClampMode::Repeat);
  } else {
    const_cast<CTexture&>(g_Renderer->GetZeroTexture()).Load(static_cast<GXTexMapID>(nextTexMap), EClampMode::Repeat);
  }
  texMapIds[1] = nextTexMap;
  nextTexMap = 2;

  if (HasColorTexture()) {
    const_cast<CTexture&>(GetColorTexture()).Load(static_cast<GXTexMapID>(nextTexMap), EClampMode::Repeat);
  } else {
    const_cast<CTexture&>(g_Renderer->GetZeroTexture()).Load(static_cast<GXTexMapID>(nextTexMap), EClampMode::Repeat);
  }
  texMapIds[2] = nextTexMap;
  nextTexMap = 3;

  if (hasBumpMap) {
    texMapIds[3] = nextTexMap;
    const_cast<CTexture&>(GetBumpMap()).Load(static_cast<GXTexMapID>(nextTexMap), EClampMode::Repeat);
    nextTexMap = 4;
  }

  if (envMapType != 0) {
    texMapIds[4] = nextTexMap;
    const_cast<CTexture&>(GetEnvMap()).Load(static_cast<GXTexMapID>(nextTexMap), EClampMode::Repeat);
    nextTexMap += 1;
  }

  if (hasEnvBumpMap) {
    texMapIds[5] = nextTexMap;
    const_cast<CTexture&>(GetEnvBumpMap()).Load(static_cast<GXTexMapID>(nextTexMap), EClampMode::Repeat);
    nextTexMap += 1;
  }

  const auto uvOffsets = GetUVMotion().CalculateFluidTextureOffset(uvT);

  STexMtx24 colorMtx = tbl->color;
  colorMtx.m[0][0] = x4c_uvMotion.GetFluidLayerMotion(CFluidUVMotion::EFluidUVMotion::Circular).x14_uvScale;
  colorMtx.m[1][1] = x4c_uvMotion.GetFluidLayerMotion(CFluidUVMotion::EFluidUVMotion::Circular).x14_uvScale;
  colorMtx.m[0][3] = uvOffsets[1][0];
  colorMtx.m[1][3] = uvOffsets[1][1];

  STexMtx24 pattern1Mtx = tbl->pattern1;
  pattern1Mtx.m[0][0] = x4c_uvMotion.GetFluidLayerMotion(CFluidUVMotion::EFluidUVMotion::Oscillate).x14_uvScale;
  pattern1Mtx.m[1][1] = x4c_uvMotion.GetFluidLayerMotion(CFluidUVMotion::EFluidUVMotion::Oscillate).x14_uvScale;
  pattern1Mtx.m[0][3] = uvOffsets[2][0];
  pattern1Mtx.m[1][3] = uvOffsets[2][1];

  STexMtx24 pattern2Mtx = tbl->pattern2;
  pattern2Mtx.m[0][0] = x4c_uvMotion.GetFluidLayerMotion(CFluidUVMotion::EFluidUVMotion::Linear).x14_uvScale;
  pattern2Mtx.m[1][1] = x4c_uvMotion.GetFluidLayerMotion(CFluidUVMotion::EFluidUVMotion::Linear).x14_uvScale;
  pattern2Mtx.m[0][3] = uvOffsets[0][0];
  pattern2Mtx.m[1][3] = uvOffsets[0][1];

  GXLoadTexMtxImm(colorMtx.m, GX_TEXMTX0, GX_MTX2x4);
  GXLoadTexMtxImm(pattern1Mtx.m, GX_TEXMTX1, GX_MTX2x4);
  GXLoadTexMtxImm(pattern2Mtx.m, GX_TEXMTX2, GX_MTX2x4);

  int texMtx = GX_TEXMTX3;

  if (hasBumpMap) {
    float bumpScale = GetBumpScale();
    zeus::CTransform nrmMtxSrc(CGraphics::GetViewMatrix().getRotation().quickInverse());
    Mtx nrmMtx;
    nrmMtxSrc.toCStyleMatrix(nrmMtx);
    MTXScaleApply(nrmMtx, nrmMtx, bumpScale, bumpScale, bumpScale);
    GXLoadNrmMtxImm(nrmMtx, GX_PNMTX0);
  }

  texCoordIds[0] = nextCoord;
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX0, false, GX_PTIDENTITY);
  nextCoord = 1;
  texCoordIds[1] = nextCoord;
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX1, false, GX_PTIDENTITY);
  nextCoord = 2;
  texCoordIds[2] = nextCoord;
  CGX::SetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX2, false, GX_PTIDENTITY);
  nextCoord = 3;

  if (hasBumpMap) {
    texCoordIds[3] = nextCoord;
    CGX::SetTexCoordGen(static_cast<GXTexCoordID>(nextCoord), GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX0, false,
                        GX_PTIDENTITY);
    CGX::SetTexCoordGen(static_cast<GXTexCoordID>(nextCoord + 1), GX_TG_BUMP3, GX_TG_TEXCOORD3, GX_IDENTITY, false,
                        GX_PTIDENTITY);
    nextCoord = 5;
  }

  if (hasEnvBumpMap) {
    float envBumpScale;
    if (envMapType != 0) {
      envBumpScale = 0.5f * (1.f - x118_reflectionSize);
    } else {
      envBumpScale = g_tweakGame->GetFluidEnvBumpScale() *
                     GetUVMotion().GetFluidLayerMotion(CFluidUVMotion::EFluidUVMotion::Linear).x14_uvScale;
    }

    Mtx envBumpMtx;
    if (envMapType == 0) {
      MTXIdentity(envBumpMtx);
    } else {
      MTXIdentity(envBumpMtx);
    }
    GXLoadTexMtxImm(envBumpMtx, GX_TEXMTX3, GX_MTX2x4);

    Mtx postMtx;
    MTXScale(postMtx, envBumpScale, -envBumpScale, 1.f);
    postMtx[0][3] = 0.5f;
    postMtx[1][3] = 0.5f;
    GXLoadTexMtxImm(postMtx, GX_PTTEXMTX0, GX_MTX3x4);

    texCoordIds[4] = nextCoord;
    int envBumpCoordIdx = nextCoord;
    CGX::SetTexCoordGen(static_cast<GXTexCoordID>(nextCoord), GX_TG_MTX2x4, GX_TG_NRM, GX_TEXMTX3, true, GX_PTTEXMTX0);
    nextCoord += 1;
    texMtx = GX_TEXMTX4;

    float indScale = 0.5f * (envMapType != 0 ? x118_reflectionSize : 1.f);
    SIndMtx23 indMtx = tbl->ind;
    indMtx.m[0][0] = indScale;
    indMtx.m[1][1] = -indScale;
    GXSetIndTexMtx(GX_ITM_0, indMtx.m, 1);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, static_cast<GXTexCoordID>(envBumpCoordIdx),
                     static_cast<GXTexMapID>(texMapIds[5]));
    CGX::SetNumIndStages(1);
  }

  if (envMapType != 0) {
    float envHeight = aabb.max.z() - aabb.min.z();
    float envWidth = aabb.max.x() - aabb.min.x();
    float maxDim;
    if (envWidth < envHeight) {
      maxDim = envWidth;
    } else {
      maxDim = envHeight;
    }
    float ooMaxDim = 1.f / maxDim;

    SEnvMtx34 envMtx = tbl->env;
    envMtx.m[0][0] = ooMaxDim;
    envMtx.m[1][1] = ooMaxDim;
    envMtx.m[0][3] = 0.5f + -aabb.center().x() / maxDim;
    envMtx.m[1][3] = 0.5f + -aabb.center().y() / maxDim;

    GXLoadTexMtxImm(envMtx.m, texMtx, GX_MTX2x4);
    texCoordIds[5] = nextCoord;
    CGX::SetTexCoordGen(static_cast<GXTexCoordID>(nextCoord), GX_TG_MTX2x4, GX_TG_POS, static_cast<GXTexMtx>(texMtx),
                        false, GX_PTIDENTITY);
    nextCoord += 1;
    texMtx += 3;
  }

  if (hasLightmap) {
    TAreaId areaId = mgr.GetNextAreaId();
    float lightmapAlpha = 1.f;
    float darkLevel = mgr.GetWorld()->GetAreaAlways(areaId)->GetPostConstructed()->x1128_worldLightingLevel;

    const CScriptWater* nextWater = water->GetNextConnectedWater(mgr);

    if (zeus::close_enough(water->GetMorphFactor(), 0.f) || nextWater == nullptr ||
        !nextWater->GetFluidPlane().HasLightMap()) {
      texMapIds[6] = nextTexMap;
      xe0_lightmap->Load(static_cast<GXTexMapID>(nextTexMap), EClampMode::Repeat);
      CalculateLightmapMatrix(areaXf, xf, aabb, texMtx);
      texCoordIds[6] = nextCoord;
      CGX::SetTexCoordGen(static_cast<GXTexCoordID>(nextCoord), GX_TG_MTX2x4, GX_TG_POS, static_cast<GXTexMtx>(texMtx),
                          false, GX_PTIDENTITY);
      nextCoord += 1;
    } else if (nextWater != nullptr && nextWater->GetFluidPlane().HasLightMap()) {
      if (zeus::close_enough(water->GetMorphFactor(), 1.f)) {
        texMapIds[6] = nextTexMap;
        nextWater->GetFluidPlane().xe0_lightmap->Load(static_cast<GXTexMapID>(nextTexMap), EClampMode::Repeat);
        nextWater->GetFluidPlane().CalculateLightmapMatrix(areaXf, xf, aabb, texMtx);
        texCoordIds[6] = nextCoord;
        CGX::SetTexCoordGen(static_cast<GXTexCoordID>(nextCoord), GX_TG_MTX2x4, GX_TG_POS,
                            static_cast<GXTexMtx>(texMtx), false, GX_PTIDENTITY);
        nextCoord += 1;
      } else {
        texMapIds[6] = nextTexMap;
        xe0_lightmap->Load(static_cast<GXTexMapID>(nextTexMap), EClampMode::Repeat);
        CalculateLightmapMatrix(areaXf, xf, aabb, texMtx);

        nextTexMap += 1;
        texMapIds[7] = nextTexMap;
        const_cast<CTexture&>(nextWater->GetFluidPlane().GetLightMap())
            .Load(static_cast<GXTexMapID>(nextTexMap), EClampMode::Repeat);
        nextWater->GetFluidPlane().CalculateLightmapMatrix(areaXf, xf, aabb, texMtx + 3);
        texCoordIds[6] = nextCoord;
        CGX::SetTexCoordGen(static_cast<GXTexCoordID>(nextCoord), GX_TG_MTX2x4, GX_TG_POS,
                            static_cast<GXTexMtx>(texMtx), false, GX_PTIDENTITY);
        texCoordIds[7] = nextCoord + 1;
        CGX::SetTexCoordGen(static_cast<GXTexCoordID>(nextCoord + 1), GX_TG_MTX2x4, GX_TG_POS,
                            static_cast<GXTexMtx>(texMtx + 3), false, GX_PTIDENTITY);
        nextCoord += 2;

        float morphVal = darkLevel * water->GetMorphFactor();
        lightmapAlpha = (1.f - water->GetMorphFactor()) / (1.f - morphVal);
        CColor kColor3(morphVal, morphVal, morphVal, 1.f);
        CGX::SetTevKColor(GX_KCOLOR3, to_gx_color(kColor3));
        hasDoubleLightmap = true;
      }
    }
    float lightmapVal = lightmapAlpha * darkLevel;
    CColor kColor2(lightmapVal, lightmapVal, lightmapVal, 1.f);
    CGX::SetTevKColor(GX_KCOLOR2, to_gx_color(kColor2));
  }

  CVector3f upVec(0.f, 0.f, 1.f);
  CVector3f xfUp(xf.transposeRotate(upVec));
  float xfUpX = xfUp.x();
  float xfUpY = xfUp.y();
  float xfUpZ = xfUp.z();
  CVector3f camUp(0.f, 1.f, 0.f);
  zeus::CTransform invView(CGraphics::GetViewMatrix().quickInverse());
  CVector3f viewUp(invView.transposeRotate(camUp));
  float dot = xfUpX * viewUp.x() + xfUpY * viewUp.y() + xfUpZ * viewUp.z();
  if (dot < 0.f) {
    dot = -dot;
  }
  float specular = (1.f - dot) * (GetSpecularMax() - GetSpecularMin()) + GetSpecularMin();
  float specularAlpha;
  if (envMapType == 2) {
    specularAlpha = 1.f;
  } else {
    specularAlpha = alpha;
  }
  CColor kColor0(specular, specular, specular, specularAlpha);
  CGX::SetTevKColor(GX_KCOLOR0, to_gx_color(kColor0));

  float reflBlend = GetReflectionBlend();
  CColor kColor1(reflBlend, reflBlend, reflBlend, 1.f);
  CGX::SetTevKColor(GX_KCOLOR1, to_gx_color(kColor1));

  CGX::SetNumTexGens(static_cast<u8>(nextCoord));

  EFluidType fluidType = GetFluidType();
  int nextStage = 0;

  switch (fluidType) {
  case EFluidType::NormalWater:
  case EFluidType::PhazonFluid:
  case EFluidType::Four: {
    int curStage = 0;
    if (hasLightmap) {
      GXChannelID lightmapChan = GX_COLOR1A1;
      GXTevColorArg lightmapRasc = GX_CC_RASC;
      if (hasDoubleLightmap) {
        lightmapChan = GX_COLOR_NULL;
        lightmapRasc = GX_CC_ZERO;
      }
      CGX::SetTevOrder(GX_TEVSTAGE0, static_cast<GXTexCoordID>(texCoordIds[6]), static_cast<GXTexMapID>(texMapIds[6]),
                       lightmapChan);
      CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, lightmapRasc);
      CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVREG2);
      CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K2);
      curStage = 1;
      if (hasDoubleLightmap) {
        CGX::SetTevOrder(GX_TEVSTAGE1, static_cast<GXTexCoordID>(texCoordIds[7]), static_cast<GXTexMapID>(texMapIds[7]),
                         GX_COLOR1A1);
        CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_C2, GX_CC_TEXC, GX_CC_KONST, GX_CC_RASC);
        CGX::SetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVREG2);
        CGX::SetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K3);
        curStage = 2;
      }
    }
    CGX::SetTevOrder(static_cast<GXTevStageID>(curStage), static_cast<GXTexCoordID>(texCoordIds[0]),
                     static_cast<GXTexMapID>(texMapIds[0]), GX_COLOR1A1);
    CGX::SetTevColorIn(static_cast<GXTevStageID>(curStage), GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_RASC);
    CGX::SetTevColorOp(static_cast<GXTevStageID>(curStage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
    CGX::SetTevKColorSel(static_cast<GXTevStageID>(curStage), GX_TEV_KCSEL_K0);
    CGX::SetTevOrder(static_cast<GXTevStageID>(curStage + 1), static_cast<GXTexCoordID>(texCoordIds[1]),
                     static_cast<GXTexMapID>(texMapIds[1]), GX_COLOR0A0);
    CGX::SetTevColorIn(static_cast<GXTevStageID>(curStage + 1), GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_RASC);
    CGX::SetTevColorOp(static_cast<GXTevStageID>(curStage + 1), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true,
                       GX_TEVPREV);
    CGX::SetTevOrder(static_cast<GXTevStageID>(curStage + 2), static_cast<GXTexCoordID>(texCoordIds[2]),
                     static_cast<GXTexMapID>(texMapIds[2]), GX_COLOR1A1);
    GXTevColorArg colorRasc = GX_CC_RASC;
    if (hasLightmap) {
      colorRasc = GX_CC_C2;
    }
    CGX::SetTevColorIn(static_cast<GXTevStageID>(curStage + 2), GX_CC_ZERO, GX_CC_TEXC, colorRasc, GX_CC_CPREV);
    CGX::SetTevColorOp(static_cast<GXTevStageID>(curStage + 2), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true,
                       GX_TEVPREV);
    if (envMapType == 0 && hasEnvBumpMap) {
      CGX::SetTevIndirect(static_cast<GXTevStageID>(curStage + 2), GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_STU, GX_ITM_0,
                          GX_ITW_OFF, GX_ITW_OFF, false, false, GX_ITBA_OFF);
    }
    nextStage = curStage + 3;
    if (envMapType > 0) {
      CGX::SetTevOrder(static_cast<GXTevStageID>(nextStage), static_cast<GXTexCoordID>(texCoordIds[5]),
                       static_cast<GXTexMapID>(texMapIds[4]), GX_COLOR_NULL);
      GXTevColorArg envD = GX_CC_TEXC;
      GXTevColorArg envC = GX_CC_ZERO;
      GXTevColorArg envB = GX_CC_ZERO;
      GXTevColorArg envA = GX_CC_ZERO;
      if (envMapType == 1) {
        envD = GX_CC_ZERO;
        envC = GX_CC_KONST;
        envB = GX_CC_TEXC;
        envA = GX_CC_CPREV;
      }
      CGX::SetTevColorIn(static_cast<GXTevStageID>(nextStage), envA, envB, envC, envD);
      CGX::SetTevColorOp(static_cast<GXTevStageID>(nextStage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
      CGX::SetTevKColorSel(static_cast<GXTevStageID>(nextStage), GX_TEV_KCSEL_K1);
      CGX::SetTevIndirect(static_cast<GXTevStageID>(nextStage), GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_STU, GX_ITM_0,
                          GX_ITW_OFF, GX_ITW_OFF, false, false, GX_ITBA_OFF);
      nextStage = curStage + 4;
    }
    break;
  }
  case EFluidType::PoisonWater: {
    if (hasLightmap) {
      GXChannelID lightmapChan = GX_COLOR1A1;
      GXTevColorArg lightmapRasc = GX_CC_RASC;
      if (hasDoubleLightmap) {
        lightmapChan = GX_COLOR_NULL;
        lightmapRasc = GX_CC_ZERO;
      }
      CGX::SetTevOrder(GX_TEVSTAGE0, static_cast<GXTexCoordID>(texCoordIds[6]), static_cast<GXTexMapID>(texMapIds[6]),
                       lightmapChan);
      CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, lightmapRasc);
      CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVREG2);
      CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K2);
      nextStage = 1;
      if (hasDoubleLightmap) {
        CGX::SetTevOrder(GX_TEVSTAGE1, static_cast<GXTexCoordID>(texCoordIds[7]), static_cast<GXTexMapID>(texMapIds[7]),
                         GX_COLOR1A1);
        CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_C2, GX_CC_TEXC, GX_CC_KONST, GX_CC_RASC);
        CGX::SetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVREG2);
        CGX::SetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K3);
        nextStage = 2;
      }
    }
    CGX::SetTevOrder(static_cast<GXTevStageID>(nextStage), static_cast<GXTexCoordID>(texCoordIds[0]),
                     static_cast<GXTexMapID>(texMapIds[0]), GX_COLOR1A1);
    CGX::SetTevColorIn(static_cast<GXTevStageID>(nextStage), GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_RASC);
    CGX::SetTevColorOp(static_cast<GXTevStageID>(nextStage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
    CGX::SetTevKColorSel(static_cast<GXTevStageID>(nextStage), GX_TEV_KCSEL_K0);
    CGX::SetTevOrder(static_cast<GXTevStageID>(nextStage + 1), static_cast<GXTexCoordID>(texCoordIds[1]),
                     static_cast<GXTexMapID>(texMapIds[1]), GX_COLOR0A0);
    CGX::SetTevColorIn(static_cast<GXTevStageID>(nextStage + 1), GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_RASC);
    CGX::SetTevColorOp(static_cast<GXTevStageID>(nextStage + 1), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true,
                       GX_TEVPREV);
    CGX::SetTevOrder(static_cast<GXTevStageID>(nextStage + 2), static_cast<GXTexCoordID>(texCoordIds[2]),
                     static_cast<GXTexMapID>(texMapIds[2]), GX_COLOR1A1);
    GXTevColorArg pColorRasc = GX_CC_RASC;
    if (hasLightmap) {
      pColorRasc = GX_CC_C2;
    }
    CGX::SetTevColorIn(static_cast<GXTevStageID>(nextStage + 2), GX_CC_ZERO, GX_CC_TEXC, pColorRasc, GX_CC_CPREV);
    CGX::SetTevColorOp(static_cast<GXTevStageID>(nextStage + 2), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true,
                       GX_TEVPREV);
    if (hasEnvBumpMap) {
      CGX::SetTevIndirect(static_cast<GXTevStageID>(nextStage + 2), GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_STU, GX_ITM_0,
                          GX_ITW_OFF, GX_ITW_OFF, false, false, GX_ITBA_OFF);
    }
    nextStage += 3;
    break;
  }
  case EFluidType::Lava: {
    CGX::SetTevOrder(GX_TEVSTAGE0, static_cast<GXTexCoordID>(texCoordIds[0]), static_cast<GXTexMapID>(texMapIds[0]),
                     GX_COLOR0A0);
    CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_RASC);
    CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
    CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
    CGX::SetTevOrder(GX_TEVSTAGE1, static_cast<GXTexCoordID>(texCoordIds[1]), static_cast<GXTexMapID>(texMapIds[1]),
                     GX_COLOR0A0);
    CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_RASC);
    CGX::SetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
    CGX::SetTevOrder(GX_TEVSTAGE2, static_cast<GXTexCoordID>(texCoordIds[2]), static_cast<GXTexMapID>(texMapIds[2]),
                     GX_COLOR_NULL);
    CGX::SetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_CPREV);
    CGX::SetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
    nextStage = 3;
    if (hasBumpMap) {
      CGX::SetTevOrder(GX_TEVSTAGE3, static_cast<GXTexCoordID>(texCoordIds[3]), static_cast<GXTexMapID>(texMapIds[3]),
                       GX_COLOR_NULL);
      CGX::SetTevColorIn(GX_TEVSTAGE3, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_HALF);
      CGX::SetTevColorOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, false, GX_TEVREG0);
      CGX::SetTevOrder(GX_TEVSTAGE4, static_cast<GXTexCoordID>(texCoordIds[3] + 1),
                       static_cast<GXTexMapID>(texMapIds[3]), GX_COLOR_NULL);
      CGX::SetTevColorIn(GX_TEVSTAGE4, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_C0);
      CGX::SetTevColorOp(GX_TEVSTAGE4, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVREG0);
      CGX::SetTevOrder(GX_TEVSTAGE5, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
      CGX::SetTevColorIn(GX_TEVSTAGE5, GX_CC_ZERO, GX_CC_CPREV, GX_CC_C0, GX_CC_ZERO);
      CGX::SetTevColorOp(GX_TEVSTAGE5, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_2, true, GX_TEVPREV);
      nextStage = 6;
    }
    break;
  }
  case EFluidType::ThickLava: {
    CGX::SetTevOrder(GX_TEVSTAGE0, static_cast<GXTexCoordID>(texCoordIds[0]), static_cast<GXTexMapID>(texMapIds[0]),
                     GX_COLOR0A0);
    CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_RASC);
    CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
    CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
    CGX::SetTevOrder(GX_TEVSTAGE1, static_cast<GXTexCoordID>(texCoordIds[1]), static_cast<GXTexMapID>(texMapIds[1]),
                     GX_COLOR0A0);
    CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_RASC);
    CGX::SetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
    CGX::SetTevOrder(GX_TEVSTAGE2, static_cast<GXTexCoordID>(texCoordIds[2]), static_cast<GXTexMapID>(texMapIds[2]),
                     GX_COLOR_NULL);
    CGX::SetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_CPREV);
    CGX::SetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
    nextStage = 3;
    if (hasBumpMap) {
      CGX::SetTevOrder(GX_TEVSTAGE3, static_cast<GXTexCoordID>(texCoordIds[3]), static_cast<GXTexMapID>(texMapIds[3]),
                       GX_COLOR_NULL);
      CGX::SetTevColorIn(GX_TEVSTAGE3, GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_ZERO);
      CGX::SetTevColorOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_2, true, GX_TEVPREV);
      nextStage = 4;
    }
    break;
  }
  default: {
    if (!sFluidSetupInitOnce) {
      sFluidSetupDone = 0;
      sFluidSetupInitOnce = 1;
    }
    if (!sFluidSetupDone) {
      sFluidSetupDone = 1;
    }
    break;
  }
  }

  CGX::SetNumTevStages(static_cast<u8>(nextStage));
  int lastStage = nextStage - 1;
  CGX::SetTevAlphaIn(static_cast<GXTevStageID>(lastStage), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
  CGX::SetTevAlphaOp(static_cast<GXTevStageID>(lastStage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevKAlphaSel(static_cast<GXTevStageID>(lastStage), GX_TEV_KASEL_K0_A);

  if (mgr.GetThermalDrawFlag() != EThermalDrawFlag::Hot) {
    GXBlendMode bm = (alpha == 1.f) ? GX_BM_NONE : GX_BM_BLEND;
    CGX::SetBlendMode(bm, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
  } else {
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
  }

  CGX::SetZMode(true, GX_LEQUAL, false);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
  GXSetCullMode(GX_CULL_NONE);
}

int CFluidPlaneRender::numTilesInHField;
int CFluidPlaneRender::numSubdivisionsInTile;
int CFluidPlaneRender::numSubdivisionsInHField;

bool CFluidPlaneCPU::PrepareRipple(const CRipple& ripple, const CFluidPlaneRender::SPatchInfo& info,
                                   CFluidPlaneRender::SRippleInfo& rippleOut) {
  auto lifeIdx =
      static_cast<int>((1.f - (ripple.GetTimeFalloff() - ripple.GetTime()) / ripple.GetTimeFalloff()) * 64.f);
  float dist = CFluidPlaneManager::RippleMaxs[lifeIdx] * (ripple.GetDistanceFalloff() / 256.f);
  dist *= dist;
  if (dist != 0)
    dist = std::sqrt(dist);
  dist = info.x24_ooRippleResolution * dist + 1.f;
  float centerX = info.x24_ooRippleResolution * (ripple.GetCenter().x() - info.xc_globalMin.x());
  float centerY = info.x24_ooRippleResolution * (ripple.GetCenter().y() - info.xc_globalMin.y());
  int fromX = static_cast<int>(centerX - dist) - 1;
  int toX = static_cast<int>(centerX + dist) + 1;
  int fromY = static_cast<int>(centerY - dist) - 1;
  int toY = static_cast<int>(centerY + dist) + 1;
  rippleOut.x4_fromX = std::max(0, fromX);
  rippleOut.x8_toX = std::min(static_cast<int>(info.x0_xSubdivs), toX);
  rippleOut.xc_fromY = std::max(0, fromY);
  rippleOut.x10_toY = std::min(static_cast<int>(info.x1_ySubdivs), toY);
  rippleOut.x14_gfromX = std::max(rippleOut.x14_gfromX, fromX);
  rippleOut.x18_gtoX = std::min(rippleOut.x18_gtoX, toX);
  rippleOut.x1c_gfromY = std::max(rippleOut.x1c_gfromY, fromY);
  rippleOut.x20_gtoY = std::min(rippleOut.x20_gtoY, toY);
  return !(rippleOut.x14_gfromX > rippleOut.x18_gtoX || rippleOut.x1c_gfromY > rippleOut.x20_gtoY);
}

void CFluidPlaneCPU::ApplyTurbulence(float t, Heights& heights, const Flags& flags, const SineTable& sineWave,
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

void CFluidPlaneCPU::ApplyRipple(const CFluidPlaneRender::SRippleInfo& rippleInfo, Heights& heights, Flags& flags,
                                 const SineTable& sineWave, const CFluidPlaneRender::SPatchInfo& info) const {
  float lookupT = 256.f *
                  (1.f - rippleInfo.x0_ripple.GetTime() * rippleInfo.x0_ripple.GetOOTimeFalloff() *
                             rippleInfo.x0_ripple.GetOOTimeFalloff()) *
                  rippleInfo.x0_ripple.GetFrequency();
  auto lifeIdx = static_cast<int>(64.f * rippleInfo.x0_ripple.GetTime() * rippleInfo.x0_ripple.GetOOTimeFalloff());
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
            if (u8 rippleV = CFluidPlaneManager::RippleValues[lifeIdx][static_cast<int>(divDist * distFalloff)]) {
              heights[k][l].height +=
                  rippleV * rippleInfo.x0_ripple.GetLookupAmplitude() *
                  sineWave[static_cast<size_t>(divDist * rippleInfo.x0_ripple.GetLookupPhase() + lookupT) & 0xff];
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

        if (curGridX >= 0 && curGridX < static_cast<int>(info.x2a_gridDimX) &&
            curGridY - static_cast<int>(info.x2a_gridDimX) >= 0 &&
            !info.x30_gridFlags[curGridX + curGridY - static_cast<int>(info.x2a_gridDimX)]) {
          yMax -= 2;
        }
        if (curGridX >= 0 && curGridX < static_cast<int>(info.x2a_gridDimX) &&
            curGridY + static_cast<int>(info.x2a_gridDimX) < gridCells &&
            !info.x30_gridFlags[curGridX + curGridY + static_cast<int>(info.x2a_gridDimX)]) {
          yMin += 2;
        }
        if (curGridY >= 0 && curGridY < static_cast<int>(info.x2c_gridDimY) && curGridX > 0 &&
            !info.x30_gridFlags[curGridY + curGridX - 1]) {
          xMax -= 2;
        }
        if (curGridY >= 0 && curGridY < static_cast<int>(info.x2c_gridDimY) &&
            curGridX + 1 < static_cast<int>(info.x2a_gridDimX) && !info.x30_gridFlags[curGridY + curGridX + 1]) {
          xMin += 2;
        }

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
              if (u8 rippleV = CFluidPlaneManager::RippleValues[lifeIdx][static_cast<int>(divDist * distFalloff)]) {
                heights[k][l].height +=
                    rippleV * rippleInfo.x0_ripple.GetLookupAmplitude() *
                    sineWave[static_cast<size_t>(divDist * rippleInfo.x0_ripple.GetLookupPhase() + lookupT) & 0xff];
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
                                  Heights& heights, Flags& flags, const SineTable& sineWave,
                                  const CFluidPlaneRender::SPatchInfo& info) const {
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

void CFluidPlaneCPU::UpdatePatchNoNormals(Heights& heights, const Flags& flags,
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
              sample.wavecapIntensity =
                  static_cast<u8>(std::min(255, static_cast<int>(info.x38_wavecapIntensityScale * sample.height)));
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
            sample.wavecapIntensity =
                static_cast<u8>(std::min(255, static_cast<int>(info.x38_wavecapIntensityScale * sample.height)));
          else
            sample.wavecapIntensity = 0;
        }

        if (i != 0) {
          for (int l = r11; l < x28; ++l) {
            CFluidPlaneRender::SHFieldSample& sample = heights[r9][l];
            if (sample.height > 0.f)
              sample.wavecapIntensity =
                  static_cast<u8>(std::min(255, static_cast<int>(info.x38_wavecapIntensityScale * sample.height)));
            else
              sample.wavecapIntensity = 0;
          }
        }

        if (j != 0) {
          for (int k = r9 + 1; k < x24; ++k) {
            CFluidPlaneRender::SHFieldSample& sample = heights[k][r11];
            if (sample.height > 0.f)
              sample.wavecapIntensity =
                  static_cast<u8>(std::min(255, static_cast<int>(info.x38_wavecapIntensityScale * sample.height)));
            else
              sample.wavecapIntensity = 0;
          }
        }
      }
    }
  }
}

void CFluidPlaneCPU::UpdatePatchWithNormals(Heights& heights, const Flags& flags,
                                            const CFluidPlaneRender::SPatchInfo& info) {
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
            sample.nx = static_cast<s8>(nx * normalizer);
            sample.ny = static_cast<s8>(ny * normalizer);
            sample.nz = static_cast<s8>(nz * normalizer);
            if (sample.height > 0.f)
              sample.wavecapIntensity =
                  static_cast<u8>(std::min(255, static_cast<int>(info.x38_wavecapIntensityScale * sample.height)));
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
            sample.nx = static_cast<s8>(nx * normalizer);
            sample.ny = static_cast<s8>(ny * normalizer);
            sample.nz = static_cast<s8>(nz * normalizer);
            if (sample.height > 0.f)
              sample.wavecapIntensity =
                  static_cast<u8>(std::min(255, static_cast<int>(info.x38_wavecapIntensityScale * sample.height)));
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
              sample.nx = static_cast<s8>(nx * normalizer);
              sample.ny = static_cast<s8>(ny * normalizer);
              sample.nz = static_cast<s8>(nz * normalizer);
              if (sample.height > 0.f)
                sample.wavecapIntensity =
                    static_cast<u8>(std::min(255, static_cast<int>(info.x38_wavecapIntensityScale * sample.height)));
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
              sample.nx = static_cast<s8>(nx * normalizer);
              sample.ny = static_cast<s8>(ny * normalizer);
              sample.nz = static_cast<s8>(nz * normalizer);
              if (sample.height > 0.f)
                sample.wavecapIntensity =
                    static_cast<u8>(std::min(255, static_cast<int>(info.x38_wavecapIntensityScale * sample.height)));
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
            sample.nx = static_cast<s8>(nx * normalizer);
            sample.ny = static_cast<s8>(ny * normalizer);
            sample.nz = static_cast<s8>(nz * normalizer);
            if (sample.height > 0.f)
              sample.wavecapIntensity =
                  static_cast<u8>(std::min(255, static_cast<int>(info.x38_wavecapIntensityScale * sample.height)));
            else
              sample.wavecapIntensity = 0;
          }
        }
      }
    }
  }
}

bool CFluidPlaneCPU::UpdatePatch(float time, const CFluidPlaneRender::SPatchInfo& info, Heights& heights, Flags& flags,
                                 const zeus::CVector3f& areaCenter, const std::optional<CRippleManager>& rippleManager,
                                 int fromX, int toX, int fromY, int toY) const {
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

// Used to be part of locked cache
// These are too big for stack allocation
static CFluidPlane::Heights lc_heights{};
static CFluidPlane::Flags lc_flags{};

void CFluidPlaneCPU::Render(const CStateManager& mgr, float alpha, const zeus::CAABox& aabb, const zeus::CTransform& xf,
                            const zeus::CTransform& areaXf, bool noNormals, const zeus::CFrustum& frustum,
                            const std::optional<CRippleManager>& rippleManager, TUniqueId waterId,
                            const bool* gridFlags, u32 gridDimX, u32 gridDimY, const zeus::CVector3f& areaCenter) {
  if (!sRenderFog) {
    return;
  }

  SCOPED_GRAPHICS_DEBUG_GROUP("CFluidPlaneCPU::Render", zeus::skCyan);
  TCastToConstPtr<CScriptWater> water = mgr.GetObjectById(waterId);
  RenderSetup(mgr, alpha, xf, areaXf, aabb, water);

  CGX::ResetVtxDescv();

  CFluidPlaneRender::NormalMode normalMode;
  if (HasBumpMap() && sRenderBumpMaps) {
    normalMode = CFluidPlaneRender::NormalMode::NBT;
  } else if (!noNormals) {
    normalMode = CFluidPlaneRender::NormalMode::Normals;
  } else {
    normalMode = CFluidPlaneRender::NormalMode::NoNormals;
  }

  CGX::SetVtxDesc(GX_VA_POS, GX_DIRECT);
  CGX::SetVtxDesc(GX_VA_CLR0, GX_DIRECT);

  switch (normalMode) {
  case CFluidPlaneRender::NormalMode::NBT:
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NBT, GX_CLR_RGBA, GX_RGB8, 6);
    GXSetVtxDesc(GX_VA_NBT, GX_DIRECT);
    break;
  case CFluidPlaneRender::NormalMode::Normals:
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_CLR_RGB, GX_RGB8, 6);
    CGX::SetVtxDesc(GX_VA_NRM, GX_DIRECT);
    break;
  default:
    break;
  }

  float rippleResolutionRecip = 1.f / x108_rippleResolution;
  CFluidPlaneRender::numSubdivisionsInTile = x104_tileSubdivisions;
  CFluidPlaneRender::numTilesInHField = std::min(kMaxTilesInHField, 42 / CFluidPlaneRender::numSubdivisionsInTile);
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
    float cameraPenetration = mgr.GetCameraManager()->GetCurrentCamera(mgr)->GetTranslation().dot(zeus::skUp) -
                              water->GetTriggerBoundsWR().max.z();
    wavecapIntensityScale *= (cameraPenetration >= 0.5f || cameraPenetration < 0.f) ? 1.f : 2.f * cameraPenetration;
  }

  u32 patchDimX = (water && water->GetPatchDimensionX()) ? water->GetPatchDimensionX() : 128;
  u32 patchDimY = (water && water->GetPatchDimensionY()) ? water->GetPatchDimensionY() : 128;

  u32 tileY = 0;
  float curY = aabb.min.y();
  for (int i = 0; curY < aabb.max.y() && i < patchDimY; ++i) {
    u32 tileX = 0;
    float curX = aabb.min.x();
    float _remDivsY = (aabb.max.y() - curY) * rippleResolutionRecip;
    for (int j = 0; curX < aabb.max.x() && j < patchDimX; ++j) {
      if (u8 renderFlags = water->GetPatchRenderFlags(j, i)) {
        s16 remDivsX = std::min(static_cast<s16>((aabb.max.x() - curX) * rippleResolutionRecip),
                                static_cast<s16>(CFluidPlaneRender::numSubdivisionsInHField));
        s16 remDivsY =
            std::min(static_cast<s16>(_remDivsY), static_cast<s16>(CFluidPlaneRender::numSubdivisionsInHField));
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
                                       *rippleManager, fromX, toX, fromY, toY);
          RenderPatch(info, lc_heights, lc_flags, noRipples, renderFlags == 1);
        }
      }
      curX += ripplePitch.x();
      tileX += CFluidPlaneRender::numTilesInHField;
    }
    curY += ripplePitch.y();
    tileY += CFluidPlaneRender::numTilesInHField;
  }

  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NBT, GX_CLR_RGBA, GX_F32, 6);
  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_CLR_RGB, GX_F32, 6);
  GXSetVtxDesc(GX_VA_NBT, GX_NONE);
  GXSetCullMode(GX_CULL_FRONT);
  RenderCleanup();
}

void CFluidPlaneCPU::RenderCleanup() const {
  if (!sRenderFog) {
    return;
  }

  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_TEX1, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX3x4, GX_TG_TEX2, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD3, GX_TG_MTX3x4, GX_TG_TEX3, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD4, GX_TG_MTX3x4, GX_TG_TEX4, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD5, GX_TG_MTX3x4, GX_TG_TEX5, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD6, GX_TG_MTX3x4, GX_TG_TEX6, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);

  CGX::SetTevDirect(GX_TEVSTAGE3);
  CGX::SetTevDirect(GX_TEVSTAGE6);

  CGX::SetNumIndStages(0);

  CGX::ResetVtxDescv();

  float mtx[3][4];
  CGraphics::GetViewMatrix().getRotation().quickInverse().toCStyleMatrix(mtx);
  GXLoadNrmMtxImm(mtx, GX_PNMTX0);

  CGX::SetChanCtrl(CGX::EChannelId::Channel1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_SPOT);
  CGX::SetNumChans(1);

  CGraphics::SetLightState(CGraphics::GetLightMask());
}

} // namespace metaforce
