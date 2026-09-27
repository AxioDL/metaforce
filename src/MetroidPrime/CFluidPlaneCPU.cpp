#include "MetroidPrime/CFluidPlaneCPU.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CRipple.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/Tweaks/CTweakGame.hpp"

#include "rstl/math.hpp"
#include "rstl/reserved_vector.hpp"

#include "MetaRender/CCubeRenderer.hpp"

#include "dolphin/gx/GXBump.h"
#include "dolphin/gx/GXCull.h"
#include "dolphin/gx/GXGeometry.h"
#include "dolphin/gx/GXTransform.h"
#include "dolphin/mtx.h"
#include "dolphin/os/OSCache.h"

#include <math.h>

float* InitializeSineWave();

static int kMaxTilesInHField = 7;

int CFluidPlaneCPU::CTurbulence::kTableSize = 2048;

// TODO: these are in CFluidPlaneRender
const float (&GetGlobalSineWave())[256];
bool PrepareRipple(const CRipple& ripple, const CFluidPlaneCPURender::SPatchInfo& info,
                   CFluidPlaneCPURender::SRippleInfo& rippleOut);
void ApplyRipples(const rstl::reserved_vector< CFluidPlaneCPURender::SRippleInfo, 32 >& rippleInfos,
                  CFluidPlaneCPURender::SHFieldSample (&heights)[45][45],
                  unsigned char (&flags)[9][9], const float (&sineTable)[256],
                  CFluidPlaneCPURender::SPatchInfo& info);

#ifdef __MWERKS__
static inline float fast_sqrt(register float x) {
  if (0.f == x)
    return x;
  register float rsq;
  asm {
    ps_rsqrte rsq, x
    ps_mul rsq, rsq, x
  }
  return rsq;
}
#else
static inline float fast_sqrt(float x) { return sqrtf(x); }
#endif

CFluidPlaneCPU::CFluidPlaneCPU(uint patternMap1, uint patternMap2, uint colorMap, uint bumpMap,
                               uint envMap, uint envBumpMap, float unitsPerLightmapTexel,
                               uint lightmap, uint tileSubdivisions, EFluidType fluidType,
                               float tileSize, const CVector3f& bumpLightDir, float alpha,
                               const CFluidUVMotion& uvMotion, float bumpScale, float turbSpeed,
                               float turbDistance, float turbFreqMax, float turbFreqMin,
                               float turbPhaseMax, float turbPhaseMin, float turbAmplitudeMax,
                               float turbAmplitudeMin, float specularMin, float specularMax,
                               float reflectionBlend, float reflectionSize, float rippleIntensity)
: CFluidPlane(patternMap1, patternMap2, colorMap, alpha, fluidType, rippleIntensity, uvMotion)
, mTexIdBumpMap(bumpMap)
, mTexIdEnvMap(envMap)
, mTexIdEnvBumpMap(envBumpMap)
, mTexIdLightmap(lightmap)
, mBumpMap()
, mEnvMap()
, mEnvBumpMap()
, mLightmap()
, mBumpLightDir(bumpLightDir.AsNormalized())
, mBumpScale(bumpScale)
, mTileSize(tileSize)
, mTileSubdivisions(tileSubdivisions & ~1u)
, mRippleResolution(tileSize / static_cast< float >(mTileSubdivisions))
, mSpecularMin(specularMin)
, mSpecularMax(specularMax)
, mReflectionBlend(reflectionBlend)
, mReflectionSize(reflectionSize)
, mUnitsPerLightmapTexel(unitsPerLightmapTexel)
, mTurbulence(turbSpeed, turbDistance, turbFreqMax, turbFreqMin, turbPhaseMax, turbPhaseMin,
                  turbAmplitudeMax, turbAmplitudeMin) {
  if (gpResourceFactory->GetResourceTypeById(mTexIdBumpMap) == 'TXTR' &&
      (fluidType == kFT_Lava || fluidType == kFT_ThickLava)) {
    mBumpMap = gpSimplePool->GetObj(SObjectTag('TXTR', mTexIdBumpMap));
  }
  if (gpResourceFactory->GetResourceTypeById(mTexIdEnvMap) == 'TXTR' &&
      fluidType == kFT_NormalWater) {
    mEnvMap = gpSimplePool->GetObj(SObjectTag('TXTR', mTexIdEnvMap));
  }
  if (gpResourceFactory->GetResourceTypeById(mTexIdEnvBumpMap) == 'TXTR') {
    mEnvBumpMap = gpSimplePool->GetObj(SObjectTag('TXTR', mTexIdEnvBumpMap));
  }
  if (gpResourceFactory->GetResourceTypeById(mTexIdLightmap) == 'TXTR' &&
      fluidType != kFT_Lava && fluidType != kFT_ThickLava) {
    mLightmap = gpSimplePool->GetObj(SObjectTag('TXTR', mTexIdLightmap));
  }
}

CFluidPlaneCPU::CTurbulence::CTurbulence(float speed, float distance, float freqMax, float freqMin,
                                         float phaseMax, float phaseMin, float amplitudeMax,
                                         float amplitudeMin)
: mSpeed(speed)
, mDistance(distance)
, mFreqMax(freqMax)
, mFreqMin(freqMin)
, mPhaseMax(phaseMax)
, mPhaseMin(phaseMin)
, mAmplitudeMax(amplitudeMax)
, mAmplitudeMin(amplitudeMin)
, mTable(nullptr)
, mTableCount(0)
, mHeightSelPitch(0.f)
, mOoTurbSpeed(1.f / mSpeed)
, mOoTurbDistance(1.f / mDistance)
, mHasTurbulence(false) {
  if (mAmplitudeMax == 0.f && mAmplitudeMin == 0.f) {
    return;
  }

  mTableCount = kTableSize;
  mHeightSelPitch = static_cast< float >(mTableCount);
  mTable = rs_new float[mTableCount];

  float freqConstant = 0.5f * (mFreqMax + mFreqMin);
  float freqLinear = 0.5f * (mFreqMax - mFreqMin);
  float phaseConstant = 0.5f * (mPhaseMax + mPhaseMin);
  float phaseLinear = 0.5f * (mPhaseMax - mPhaseMin);
  float amplitudeConstant = 0.5f * (mAmplitudeMax + mAmplitudeMin);
  float amplitudeLinear = 0.5f * (mAmplitudeMax - mAmplitudeMin);

  float curAng = 0.f;
  const float anglePitch = M_2PIF / mHeightSelPitch;
  for (int i = 0; i < mTableCount; ++i, curAng += anglePitch) {
    float angCos = CMath::FastCosR(curAng);
    float freq = freqLinear * angCos + freqConstant;
    mTable[i] = (amplitudeLinear * angCos + amplitudeConstant) *
                   CMath::FastSinR(freq * curAng + (phaseLinear * angCos + phaseConstant));
  }
  mHasTurbulence = true;
}

CFluidPlaneCPU::CTurbulence::~CTurbulence() { delete[] mTable; }

static inline uchar CalculateWavecapIntensity(float scale, float height) {
  int intensity = 0;
  if (height > 0.f) {
    intensity = static_cast< int >(scale * height);
    if (intensity > 255)
      intensity = 255;
  }
  return static_cast< uchar >(intensity);
}

static inline void Normal(CFluidPlaneCPURender::SHFieldSample* sample, float resolution,
                          const CFluidPlaneCPURender::SPatchInfo& info) {
  float scale = -resolution;
  float nz = .25f * resolution;
  float dx = scale * (sample[1].height - sample[-1].height);
  float dy = scale * (sample[45].height - sample[-45].height);
  float n = 63.f / fast_sqrt(dx * dx + dy * dy + nz * nz);
  sample->nx = static_cast< signed char >(CCast::FtoL(n * dx));
  sample->ny = static_cast< signed char >(CCast::FtoL(n * dy));
  sample->nz = static_cast< signed char >(CCast::FtoL(n * nz));
  int intensity = 0;
  float wavecapScale = info.mWavecapIntensityScale;
  if (sample->height > 0.f) {
    intensity = CCast::FtoL(sample->height * wavecapScale);
    if (intensity > 255)
      intensity = 255;
  }
  sample->wavecapIntensity = static_cast< uchar >(intensity);
}

void UpdatePatchWithNormals(CFluidPlaneCPURender::SHFieldSample (&heights)[45][45],
                            const unsigned char (&flags)[9][9],
                            const CFluidPlaneCPURender::SPatchInfo& info) {
  typedef CFluidPlaneCPURender::SHFieldSample SHFieldSample;
  float resolution = 2.f * info.mRippleResolution;
  int curGridY = info.mTileY * info.mGridDimX - 1;
  curGridY += info.mTileX;
  SHFieldSample* rowBase = &heights[1][1];

  for (int i = 1; i <= (info.mYSubdivs - 1 + CFluidPlaneCPURender::numSubdivisionsInTile - 1) /
                           CFluidPlaneCPURender::numSubdivisionsInTile;
       ++i) {
    int yEnd = i * CFluidPlaneCPURender::numSubdivisionsInTile + 1;
    int yStart = rstl::max_val(0, yEnd - CFluidPlaneCPURender::numSubdivisionsInTile);
    yEnd = rstl::min_val(info.mYSubdivs + 1, yEnd);
    SHFieldSample* tileBase = rowBase;
    for (int j = 1; j <= (info.mXSubdivs - 1 + CFluidPlaneCPURender::numSubdivisionsInTile - 1) /
                             CFluidPlaneCPURender::numSubdivisionsInTile;
         ++j, tileBase += CFluidPlaneCPURender::numSubdivisionsInTile) {
      int xEnd = j * CFluidPlaneCPURender::numSubdivisionsInTile + 1;
      int xStart = xEnd - CFluidPlaneCPURender::numSubdivisionsInTile;
      xEnd = rstl::min_val(info.mXSubdivs + 1, xEnd);
      if ((flags[i][j] & 0x1f) == 0x1f) {
        SHFieldSample* row = tileBase;
        for (int k = yStart; k < yEnd; ++k, row += 45) {
          SHFieldSample* sample = row;
          for (int l = xStart; l < xEnd; ++l, ++sample) {
            Normal(sample, resolution, info);
          }
        }
      } else {
#if NONMATCHING
        // Border samples and partial patches can extend beyond the shoreline grid.
        const int gridX = info.mTileX + j - 1;
        const int gridY = info.mTileY + i - 1;
        if (!info.mGridFlags ||
            (gridX >= 0 && gridX < info.mGridDimX && gridY >= 0 && gridY < info.mGridDimY &&
             info.mGridFlags[curGridY + j])) {
#else
        if (!info.mGridFlags || (info.mGridFlags && info.mGridFlags[curGridY + j])) {
#endif
          if (i > 0 && i < CFluidPlaneCPURender::numTilesInHField + 1 && j > 0 &&
              j < CFluidPlaneCPURender::numTilesInHField + 1) {
            int halfRow = (CFluidPlaneCPURender::numSubdivisionsInTile * 45) / 2;
            int halfCol = CFluidPlaneCPURender::numSubdivisionsInTile / 2;
            SHFieldSample* center = tileBase + halfRow + halfCol;
            Normal(center, resolution, info);
          }
        }
        if (j != 0 && i != 0) {
          if ((flags[i][j] & 2) != 0 || (flags[i - 1][j] & 1) != 0 || (flags[i][j] & 4) != 0 ||
              (flags[i][j - 1] & 8) != 0) {
            SHFieldSample* sample = tileBase;
            for (int l = xStart; l < xEnd; ++l, ++sample) {
              Normal(sample, resolution, info);
            }
            SHFieldSample* column = tileBase + 45;
            for (int k = yStart + 1; k < yEnd; ++k, column += 45) {
              Normal(column, resolution, info);
            }
          } else {
            Normal(tileBase, resolution, info);
          }
        }
      }
    }
    curGridY += info.mGridDimX;
    rowBase += CFluidPlaneCPURender::numSubdivisionsInTile * 45;
  }
}

void UpdatePatchNoNormals(CFluidPlaneCPURender::SHFieldSample (&heights)[45][45],
                          const unsigned char (&flags)[9][9],
                          const CFluidPlaneCPURender::SPatchInfo& info) {
  typedef CFluidPlaneCPURender::SHFieldSample SHFieldSample;
  for (int i = 1; i <= (info.mYSubdivs - 1 + CFluidPlaneCPURender::numSubdivisionsInTile - 1) /
                           CFluidPlaneCPURender::numSubdivisionsInTile;
       ++i) {
    int yEnd = i * CFluidPlaneCPURender::numSubdivisionsInTile + 1;
    int yStart = rstl::max_val(0, yEnd - CFluidPlaneCPURender::numSubdivisionsInTile);
    yEnd = rstl::min_val(info.mYSubdivs + 1, yEnd);
    for (int j = 1; j <= (info.mXSubdivs - 1 + CFluidPlaneCPURender::numSubdivisionsInTile - 1) /
                             CFluidPlaneCPURender::numSubdivisionsInTile;
         ++j) {
      int xEnd = j * CFluidPlaneCPURender::numSubdivisionsInTile + 1;
      int xStart = rstl::max_val(0, xEnd - CFluidPlaneCPURender::numSubdivisionsInTile);
      xEnd = rstl::min_val(info.mXSubdivs + 1, xEnd);
      if ((flags[i][j] & 0x1f) == 0x1f) {
        for (int k = yStart; k < yEnd; ++k) {
          for (int l = xStart; l < xEnd; ++l) {
            SHFieldSample& sample = heights[k][l];
            sample.wavecapIntensity =
                CalculateWavecapIntensity(info.mWavecapIntensityScale, sample.height);
          }
        }
      } else {
        if (i > 0 && i < CFluidPlaneCPURender::numTilesInHField + 1 && j > 0 &&
            j < CFluidPlaneCPURender::numTilesInHField + 1) {
          SHFieldSample& sample = heights[yStart + CFluidPlaneCPURender::numSubdivisionsInTile / 2]
                                         [xStart + CFluidPlaneCPURender::numSubdivisionsInTile / 2];
          sample.wavecapIntensity =
              CalculateWavecapIntensity(info.mWavecapIntensityScale, sample.height);
        }
        if (i != 0) {
          for (int l = xStart; l < xEnd; ++l) {
            SHFieldSample& sample = heights[yStart][l];
            sample.wavecapIntensity =
                CalculateWavecapIntensity(info.mWavecapIntensityScale, sample.height);
          }
        }
        if (j != 0) {
          SHFieldSample* col = &heights[0][xStart];
          for (int k = yStart + 1; k < yEnd; ++k) {
            SHFieldSample& sample = col[k * 45];
            sample.wavecapIntensity =
                CalculateWavecapIntensity(info.mWavecapIntensityScale, sample.height);
          }
        }
      }
    }
  }
}

void ApplyTurbulence(float time, CFluidPlaneCPURender::SHFieldSample (&heights)[45][45],
                     const unsigned char* flags, const float (&sineTable)[256],
                     const CFluidPlaneCPURender::SPatchInfo& info, const CFluidPlaneCPU& fluidPlane,
                     const CVector3f& areaCenter) {
  bool hasTurb = gkWaterTurbulence ? fluidPlane.HasTurbulence() : false;
  if (!hasTurb) {
    DCZeroRange(&heights, sizeof(heights));
    return;
  }

  float rippleRes = info.mRippleResolution * CCast::LtoF(1);
  float curY = (info.mLocalMinY - rippleRes) - areaCenter.GetY();
  time *= fluidPlane.GetOOTurbulenceSpeed();
  const float ooDist = fluidPlane.GetOOTurbulenceDistance();
  int xDivs = (info.mXSubdivs - 2 + CFluidPlaneCPURender::numSubdivisionsInTile - 2) /
                  CFluidPlaneCPURender::numSubdivisionsInTile *
                  CFluidPlaneCPURender::numSubdivisionsInTile +
              2;
  int yDivs = (info.mYSubdivs - 2 + CFluidPlaneCPURender::numSubdivisionsInTile - 2) /
                  CFluidPlaneCPURender::numSubdivisionsInTile *
                  CFluidPlaneCPURender::numSubdivisionsInTile +
              2;
  for (int i = 0; i <= yDivs; ++i) {
    float curYSq = curY * curY;
    float curX = (info.mLocalMinX - rippleRes) - areaCenter.GetX();
    for (int j = 0; j <= xDivs; ++j) {
      float dist = fast_sqrt(curX * curX + curYSq);
      float sel = ooDist * dist;
      sel += time;
      heights[i][j].height = fluidPlane.GetTurbulenceHeight(sel);
      curX += info.mRippleResolution;
    }
    curY += info.mRippleResolution;
  }
}

const bool UpdatePatch(float time, CFluidPlaneCPURender::SPatchInfo& info,
                       const CFluidPlaneCPU& fluidPlane, const CVector3f& areaCenter,
                       const rstl::optional_object< CRippleManager >& rippleManager, int fromX,
                       int toX, int fromY, int toY) {
  typedef CFluidPlaneCPURender::SRippleInfo SRippleInfo;
  typedef CFluidPlaneCPURender::SHFieldSample SHFieldSample;

  const float (&sineTable)[256] = GetGlobalSineWave();

  DCZeroRange(static_cast< uchar* >(LCGetBase()) + 0x40, 0x51);

  rstl::reserved_vector< SRippleInfo, 32 > rippleInfos;
  bool noRipples = true;

  if (rippleManager) {
    const rstl::vector< CRipple >& ripples = rippleManager->GetRipples();
    for (AUTO(ripple, ripples.begin()); ripple != ripples.end(); ++ripple) {
      if (ripple->GetTime() >= ripple->GetTimeFalloff())
        continue;
      SRippleInfo rippleInfo(*ripple, fromX, toX, fromY, toY);
      if (PrepareRipple(*ripple, info, rippleInfo)) {
        rippleInfos.push_back(rippleInfo);
      }
    }
    if (!rippleInfos.empty()) {
      noRipples = false;
    }
  }

  if (noRipples) {
    return noRipples;
  }

  SHFieldSample(&heights)[45][45] = *reinterpret_cast< SHFieldSample(*)[45][45] >(static_cast< uchar* >(LCGetBase()) + 0xa0);

  ApplyTurbulence(time, heights, reinterpret_cast< unsigned char* >(static_cast< uchar* >(LCGetBase()) + 0x40), sineTable, info,
                  fluidPlane, areaCenter);
  ApplyRipples(rippleInfos, heights, *reinterpret_cast< unsigned char (*)[9][9] >(static_cast< uchar* >(LCGetBase()) + 0x40),
               sineTable, info);

  if (static_cast< int >(info.mNormalMode) ==
      static_cast< int >(CFluidPlaneCPURender::kNM_NoNormals)) {
    UpdatePatchNoNormals(heights, *reinterpret_cast< const unsigned char (*)[9][9] >(static_cast< uchar* >(LCGetBase()) + 0x40),
                         info);
  } else {
    UpdatePatchWithNormals(heights, *reinterpret_cast< const unsigned char (*)[9][9] >(static_cast< uchar* >(LCGetBase()) + 0x40),
                           info);
  }

  return noRipples;
}

void CFluidPlaneCPU::CalculateLightmapMtx(const CTransform4f& areaXf, const CTransform4f& xf,
                                          const CAABox& aabb, int idx) const {
  int width = GetLightMap()->GetWidth();
  int height = GetLightMap()->GetHeight();

  CVector3f norm = areaXf.GetRow(kDZ).AsNormalized();
  CTransform4f toLocal = areaXf.GetRotation().GetQuickInverse();
  CAABox areaLocalAABB = aabb.GetTransformedAABox(toLocal);

  float scaleU = areaLocalAABB.GetWidth() / (width * mUnitsPerLightmapTexel);
  float scaleV = areaLocalAABB.GetHeight() / (height * mUnitsPerLightmapTexel);

  float leftBorder = (1.f + fmod(areaLocalAABB.GetMinPoint().GetX() + xf.GetTranslation().GetX(),
                                 mUnitsPerLightmapTexel)) /
                     width;
  float rightBorder = (2.f - fmod(areaLocalAABB.GetMaxPoint().GetX() + xf.GetTranslation().GetX(),
                                  mUnitsPerLightmapTexel)) /
                      width;
  float bottomBorder = (1.f + fmod(areaLocalAABB.GetMinPoint().GetY() + xf.GetTranslation().GetY(),
                                   mUnitsPerLightmapTexel)) /
                       height;
  float topBorder = (2.f - fmod(areaLocalAABB.GetMaxPoint().GetY() + xf.GetTranslation().GetY(),
                                mUnitsPerLightmapTexel)) /
                    height;

  CTransform4f texMtx(
      (scaleU - leftBorder - rightBorder) / areaLocalAABB.GetWidth(), 0.f, 0.f,
      leftBorder + scaleU * -areaLocalAABB.GetMinPoint().GetX() / areaLocalAABB.GetWidth(), 0.f,
      (-(scaleV - bottomBorder - topBorder)) / areaLocalAABB.GetHeight(), 0.f,
      scaleV * areaLocalAABB.GetMinPoint().GetY() / areaLocalAABB.GetHeight() - topBorder, 0.f, 0.f,
      0.f, 0.f);

  CTransform4f result(texMtx * toLocal);
  float mtx[2][4] = {{result.Get00(), result.Get01(), result.Get02(), result.Get03()},
                     {result.Get10(), result.Get11(), result.Get12(), result.Get13()}};
  CGX::LoadTexMtxImm(mtx, idx, GX_MTX2x4);
}

void CFluidPlaneCPU::RenderSetup(const CStateManager& mgr, float alpha, const CTransform4f& xf,
                                 const CTransform4f& areaXf, const CAABox& aabb,
                                 CScriptWater* water) const {
  if (!gkWaterEnable) {
    return;
  }

  float uvT = mgr.GetFluidPlaneManager()->GetTime();
  bool hasBumpMap = HasBumpMap() && gkWaterBumpMapping;
  bool hasDoubleLightmap = false;
  const bool hasLightmap = HasLightMap();
  const int envMapType =
      mgr.GetCameraManager()->GetFluidCounter() != 0 ? 0 : (HasEnvMap() ? gkWaterEnvMap : 0);
  const bool hasEnvBumpMap = HasEnvBumpMap();

  InitializeSineWave();

  gpRender->SetModelMatrix(xf);

  const GXColor ambColor = {0, 0, 0, 0};
  const GXColor white = {0xff, 0xff, 0xff, 0xff};

  if (hasBumpMap) {
    CLight bumpLight(
        CLight::BuildDirectional(GetBumpLightDir().AsNormalized(), CColor(0.5f, 0.5f, 0.5f, 1.f)));
    CGraphics::LoadLight(kLight3, bumpLight);
    CGX::SetNumChans(2);
    CGX::SetChanCtrl(CGX::Channel1, true, GX_SRC_REG, GX_SRC_REG, GX_LIGHT3, GX_DF_CLAMP,
                     GX_AF_SPOT);
    CGX::SetChanMatColor(CGX::Channel1, white);
    CGX::SetChanAmbColor(CGX::Channel1, ambColor);
    CGX::SetChanCtrl(CGX::Channel0, true, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP,
                     GX_AF_SPOT);
    CGX::SetChanMatColor(CGX::Channel0, white);
    CGX::SetChanAmbColor(CGX::Channel0, ambColor);
  } else {
    CGX::SetNumChans(2);
    CGX::SetChanCtrl(CGX::Channel1, true, GX_SRC_REG, GX_SRC_REG,
                     static_cast< GXLightID >(CGraphics::GetLightMask()), GX_DF_CLAMP, GX_AF_SPOT);
    CGX::SetChanMatColor(CGX::Channel1, CGraphics::GetLightMask() != 0 ? white : ambColor);
    if (hasLightmap) {
      CGX::SetChanAmbColor(CGX::Channel1, ambColor);
    }
    CGX::SetChanCtrl(CGX::Channel0, true, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP,
                     GX_AF_SPOT);
    CGX::SetChanMatColor(CGX::Channel0, white);
    CGX::SetChanAmbColor(CGX::Channel0, ambColor);
  }

  int nextTexMap = 0;
  int nextCoord = 0;

  GXTexMapID texMapIds[8];
  GXTexCoordID texCoordIds[8];

  const CTexture& zeroTexture = CCubeRenderer::That()->GetZeroTexture();
  if (HasTexturePattern1()) {
    GetTexturePattern1()->Load(static_cast< GXTexMapID >(nextTexMap), CTexture::kCM_Repeat);
  } else {
    zeroTexture.Load(static_cast< GXTexMapID >(nextTexMap), CTexture::kCM_Repeat);
  }
  texMapIds[0] = static_cast< GXTexMapID >(nextTexMap++);

  if (HasTexturePattern2()) {
    GetTexturePattern2()->Load(static_cast< GXTexMapID >(nextTexMap), CTexture::kCM_Repeat);
  } else {
    zeroTexture.Load(static_cast< GXTexMapID >(nextTexMap), CTexture::kCM_Repeat);
  }
  texMapIds[1] = static_cast< GXTexMapID >(nextTexMap++);

  if (HasColorTexture()) {
    GetColorTexture()->Load(static_cast< GXTexMapID >(nextTexMap), CTexture::kCM_Repeat);
  } else {
    zeroTexture.Load(static_cast< GXTexMapID >(nextTexMap), CTexture::kCM_Repeat);
  }
  texMapIds[2] = static_cast< GXTexMapID >(nextTexMap++);

  if (hasBumpMap) {
    texMapIds[3] = static_cast< GXTexMapID >(nextTexMap);
    GetBumpMap()->Load(static_cast< GXTexMapID >(nextTexMap++), CTexture::kCM_Repeat);
  }

  if (envMapType != 0) {
    texMapIds[4] = static_cast< GXTexMapID >(nextTexMap);
    GetEnvMap()->Load(static_cast< GXTexMapID >(nextTexMap++), CTexture::kCM_Repeat);
  }

  if (hasEnvBumpMap) {
    texMapIds[5] = static_cast< GXTexMapID >(nextTexMap);
    GetEnvBumpMap()->Load(static_cast< GXTexMapID >(nextTexMap++), CTexture::kCM_Repeat);
  }

  float uvOffsets[3][2];
  GetUVMotion().CalculateFluidTextureOffset(uvT, uvOffsets);

  const float pattern1Scale =
      mUvMotion.GetFluidLayerMotion(CFluidUVMotion::kFL_Pattern1).mUvScale;
  float pattern1Mtx[2][4] = {};
  pattern1Mtx[0][0] = pattern1Scale;
  pattern1Mtx[0][3] = uvOffsets[1][0];
  pattern1Mtx[1][1] = pattern1Scale;
  pattern1Mtx[1][3] = uvOffsets[1][1];

  const float pattern2Scale =
      mUvMotion.GetFluidLayerMotion(CFluidUVMotion::kFL_Pattern2).mUvScale;
  float pattern2Mtx[2][4] = {};
  pattern2Mtx[0][0] = pattern2Scale;
  pattern2Mtx[0][3] = uvOffsets[2][0];
  pattern2Mtx[1][1] = pattern2Scale;
  pattern2Mtx[1][3] = uvOffsets[2][1];

  const float colorScale = mUvMotion.GetFluidLayerMotion(CFluidUVMotion::kFL_Color).mUvScale;
  float colorMtx[2][4] = {};
  colorMtx[0][0] = colorScale;
  colorMtx[0][3] = uvOffsets[0][0];
  colorMtx[1][1] = colorScale;
  colorMtx[1][3] = uvOffsets[0][1];

  GXLoadTexMtxImm(pattern1Mtx, GX_TEXMTX0, GX_MTX2x4);
  GXLoadTexMtxImm(pattern2Mtx, GX_TEXMTX1, GX_MTX2x4);
  GXLoadTexMtxImm(colorMtx, GX_TEXMTX2, GX_MTX2x4);

  GXTexMtx texMtx = GX_TEXMTX3;

  if (hasBumpMap) {
    float bumpScale = GetBumpScale();
    Mtx nrmMtx;
    CTransform4f nrmMtxSrc(CGraphics::GetViewMatrix().GetRotation().GetQuickInverse());
    nrmMtx[0][0] = nrmMtxSrc.Get00();
    nrmMtx[0][1] = nrmMtxSrc.Get01();
    nrmMtx[0][2] = nrmMtxSrc.Get02();
    nrmMtx[0][3] = nrmMtxSrc.Get03();
    nrmMtx[1][0] = nrmMtxSrc.Get10();
    nrmMtx[1][1] = nrmMtxSrc.Get11();
    nrmMtx[1][2] = nrmMtxSrc.Get12();
    nrmMtx[1][3] = nrmMtxSrc.Get13();
    nrmMtx[2][0] = nrmMtxSrc.Get20();
    nrmMtx[2][1] = nrmMtxSrc.Get21();
    nrmMtx[2][2] = nrmMtxSrc.Get22();
    nrmMtx[2][3] = nrmMtxSrc.Get23();
    MTXScaleApply(nrmMtx, nrmMtx, bumpScale, bumpScale, bumpScale);
    GXLoadNrmMtxImm(nrmMtx, GX_PNMTX0);
  }

  texCoordIds[0] = static_cast< GXTexCoordID >(nextCoord++);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX0, false, GX_PTIDENTITY);
  texCoordIds[1] = static_cast< GXTexCoordID >(nextCoord++);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX1, false, GX_PTIDENTITY);
  texCoordIds[2] = static_cast< GXTexCoordID >(nextCoord++);
  CGX::SetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX2, false, GX_PTIDENTITY);

  if (hasBumpMap) {
    texCoordIds[3] = static_cast< GXTexCoordID >(nextCoord);
    CGX::SetTexCoordGen(static_cast< GXTexCoordID >(nextCoord), GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX0,
                        false, GX_PTIDENTITY);
    CGX::SetTexCoordGen(static_cast< GXTexCoordID >(nextCoord + 1), GX_TG_BUMP3, GX_TG_TEXCOORD3,
                        GX_IDENTITY, false, GX_PTIDENTITY);
    nextCoord += 2;
  }

  if (hasEnvBumpMap) {
    const float envBumpScale =
        envMapType != 0
            ? 0.5f * (1.f - mReflectionSize)
            : gpTweakGame->GetFluidEnvBumpScale() *
                  GetUVMotion().GetFluidLayerMotion(CFluidUVMotion::kFL_Color).mUvScale;

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

    texCoordIds[5] = static_cast< GXTexCoordID >(nextCoord);
    int envBumpCoordIdx = nextCoord;
    CGX::SetTexCoordGen(static_cast< GXTexCoordID >(nextCoord++), GX_TG_MTX2x4, GX_TG_NRM,
                        GX_TEXMTX3, true, GX_PTTEXMTX0);
    texMtx = GX_TEXMTX4;

    float indScale = 0.5f * (envMapType != 0 ? mReflectionSize : 1.f);
    float indMtx[2][3] = {};
    indMtx[0][0] = indScale;
    indMtx[1][1] = -indScale;
    GXSetIndTexMtx(GX_ITM_0, indMtx, 1);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, static_cast< GXTexCoordID >(envBumpCoordIdx), texMapIds[5]);
    CGX::SetNumIndStages(1);
  }

  if (envMapType != 0) {
    float ooMaxDim;
    const float maxDim = rstl::max_val(aabb.GetWidth(), aabb.GetHeight());
    ooMaxDim = 1.f / maxDim;

    float envMtx[3][4] = {};
    envMtx[0][0] = ooMaxDim;
    envMtx[0][3] = 0.5f + (-aabb.GetCenterPoint().GetX()) / maxDim;
    envMtx[1][1] = ooMaxDim;
    envMtx[1][3] = 0.5f + (-aabb.GetCenterPoint().GetY()) / maxDim;

    GXLoadTexMtxImm(envMtx, texMtx, GX_MTX2x4);
    texCoordIds[4] = static_cast< GXTexCoordID >(nextCoord);
    CGX::SetTexCoordGen(static_cast< GXTexCoordID >(nextCoord++), GX_TG_MTX2x4, GX_TG_POS, texMtx,
                        false, GX_PTIDENTITY);
    texMtx = static_cast< GXTexMtx >(texMtx + 3);
  }

  if (hasLightmap) {
    float lightmapAlpha = 1.f;
    float darkLevel = mgr.GetWorld()
                          ->GetArea(mgr.GetNextAreaId())
                          ->GetPostConstructed()
                          ->mWorldLightingLevel;

    const CScriptWater* nextWater = water->GetNextConnectedWater(mgr);

    if (close_enough(water->GetMorphFactor(), 0.f) || nextWater == nullptr ||
        (nextWater != nullptr && !nextWater->GetFluidPlane().HasLightMap())) {
      texMapIds[6] = static_cast< GXTexMapID >(nextTexMap);
      GetLightMap()->Load(static_cast< GXTexMapID >(nextTexMap), CTexture::kCM_Repeat);
      CalculateLightmapMtx(areaXf, xf, aabb, texMtx);
      texCoordIds[6] = static_cast< GXTexCoordID >(nextCoord);
      CGX::SetTexCoordGen(static_cast< GXTexCoordID >(nextCoord++), GX_TG_MTX2x4, GX_TG_POS, texMtx,
                          false, GX_PTIDENTITY);
    } else if (nextWater != nullptr && nextWater->GetFluidPlane().HasLightMap()) {
      if (close_enough(water->GetMorphFactor(), 1.f)) {
        texMapIds[6] = static_cast< GXTexMapID >(nextTexMap);
        nextWater->GetFluidPlane().GetLightMap()->Load(static_cast< GXTexMapID >(nextTexMap),
                                                       CTexture::kCM_Repeat);
        nextWater->GetFluidPlane().CalculateLightmapMtx(areaXf, xf, aabb, texMtx);
        texCoordIds[6] = static_cast< GXTexCoordID >(nextCoord);
        CGX::SetTexCoordGen(static_cast< GXTexCoordID >(nextCoord++), GX_TG_MTX2x4, GX_TG_POS,
                            texMtx, false, GX_PTIDENTITY);
      } else {
        texMapIds[6] = static_cast< GXTexMapID >(nextTexMap);
        GetLightMap()->Load(static_cast< GXTexMapID >(nextTexMap++), CTexture::kCM_Repeat);
        CalculateLightmapMtx(areaXf, xf, aabb, texMtx);

        texCoordIds[6] = static_cast< GXTexCoordID >(nextCoord);
        CGX::SetTexCoordGen(static_cast< GXTexCoordID >(nextCoord++), GX_TG_MTX2x4, GX_TG_POS,
                            texMtx, false, GX_PTIDENTITY);
        texMapIds[7] = static_cast< GXTexMapID >(nextTexMap);
        nextWater->GetFluidPlane().GetLightMap()->Load(static_cast< GXTexMapID >(nextTexMap),
                                                       CTexture::kCM_Repeat);
        nextWater->GetFluidPlane().CalculateLightmapMtx(areaXf, xf, aabb, texMtx + 3);
        texCoordIds[7] = static_cast< GXTexCoordID >(nextCoord);
        CGX::SetTexCoordGen(static_cast< GXTexCoordID >(nextCoord++), GX_TG_MTX2x4, GX_TG_POS,
                            static_cast< GXTexMtx >(texMtx + 3), false, GX_PTIDENTITY);

        float morphVal = darkLevel * water->GetMorphFactor();
        lightmapAlpha = (1.f - water->GetMorphFactor()) / (1.f - morphVal);
        CGX::SetTevKColor(GX_KCOLOR3, CColor(morphVal, morphVal, morphVal, 1.f).GetGXColor());
        hasDoubleLightmap = true;
      }
    }
    lightmapAlpha *= darkLevel;
    CGX::SetTevKColor(GX_KCOLOR2,
                      CColor(lightmapAlpha, lightmapAlpha, lightmapAlpha, 1.f).GetGXColor());
  }

  const CVector3f xfUp = xf.TransposeRotate(CVector3f(0.f, 0.f, 1.f));
  const CVector3f viewUp =
      CGraphics::GetViewMatrix().GetQuickInverse().TransposeRotate(CVector3f(0.f, 1.f, 0.f));
  float dot = CVector3f::Dot(xfUp, viewUp);
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
  CGX::SetTevKColor(GX_KCOLOR0, CColor(specular, specular, specular, specularAlpha).GetGXColor());

  float reflBlend = GetReflectionBlend();
  CGX::SetTevKColor(GX_KCOLOR1, CColor(reflBlend, reflBlend, reflBlend, 1.f).GetGXColor());

  CGX::SetNumTexGens(static_cast< u8 >(nextCoord));

  int fluidType = GetFluidType();
  GXTevStageID nextStage = GX_TEVSTAGE0;

  switch (fluidType) {
  case kFT_NormalWater:
  case kFT_PhazonFluid:
  case kFT_Four: {
    if (hasLightmap) {
      CGX::SetTevOrder(GX_TEVSTAGE0, texCoordIds[6], texMapIds[6],
                       hasDoubleLightmap ? GX_COLOR_NULL : GX_COLOR1A1);
      CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST,
                         hasDoubleLightmap ? GX_CC_ZERO : GX_CC_RASC);
      CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVREG2);
      CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K2);
      nextStage = GX_TEVSTAGE1;
      if (hasDoubleLightmap) {
        CGX::SetTevOrder(GX_TEVSTAGE1, texCoordIds[7], texMapIds[7], GX_COLOR1A1);
        CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_C2, GX_CC_TEXC, GX_CC_KONST, GX_CC_RASC);
        CGX::SetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVREG2);
        CGX::SetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K3);
        nextStage = GX_TEVSTAGE2;
      }
    }
    CGX::SetTevOrder(nextStage, texCoordIds[0], texMapIds[0], GX_COLOR1A1);
    CGX::SetTevColorIn(nextStage, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_RASC);
    CGX::SetTevColorOp(nextStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
    CGX::SetTevKColorSel(nextStage, GX_TEV_KCSEL_K0);
    nextStage = static_cast< GXTevStageID >(nextStage + 1);
    CGX::SetTevOrder(nextStage, texCoordIds[1], texMapIds[1], GX_COLOR0A0);
    CGX::SetTevColorIn(nextStage, GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_RASC);
    CGX::SetTevColorOp(nextStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
    nextStage = static_cast< GXTevStageID >(nextStage + 1);
    CGX::SetTevOrder(nextStage, texCoordIds[2], texMapIds[2], GX_COLOR1A1);
    CGX::SetTevColorIn(nextStage, GX_CC_ZERO, GX_CC_TEXC, hasLightmap ? GX_CC_C2 : GX_CC_RASC,
                       GX_CC_CPREV);
    CGX::SetTevColorOp(nextStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
    if (envMapType == 0 && hasEnvBumpMap) {
      CGX::SetTevIndirect(nextStage, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_STU, GX_ITM_0, GX_ITW_OFF,
                          GX_ITW_OFF, false, false, GX_ITBA_OFF);
    }
    nextStage = static_cast< GXTevStageID >(nextStage + 1);
    if (envMapType > 0) {
      CGX::SetTevOrder(nextStage, texCoordIds[4], texMapIds[4], GX_COLOR_NULL);
      CGX::SetTevColorIn(nextStage, envMapType == 1 ? GX_CC_CPREV : GX_CC_ZERO,
                         envMapType == 1 ? GX_CC_TEXC : GX_CC_ZERO,
                         envMapType == 1 ? GX_CC_KONST : GX_CC_ZERO,
                         envMapType == 1 ? GX_CC_ZERO : GX_CC_TEXC);
      CGX::SetTevColorOp(nextStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
      CGX::SetTevKColorSel(nextStage, GX_TEV_KCSEL_K1);
      CGX::SetTevIndirect(nextStage, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_STU, GX_ITM_0, GX_ITW_OFF,
                          GX_ITW_OFF, false, false, GX_ITBA_OFF);
      nextStage = static_cast< GXTevStageID >(nextStage + 1);
    }
    break;
  }
  case kFT_PoisonWater: {
    if (hasLightmap) {
      CGX::SetTevOrder(GX_TEVSTAGE0, texCoordIds[6], texMapIds[6],
                       hasDoubleLightmap ? GX_COLOR_NULL : GX_COLOR1A1);
      CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST,
                         hasDoubleLightmap ? GX_CC_ZERO : GX_CC_RASC);
      CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVREG2);
      CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K2);
      nextStage = GX_TEVSTAGE1;
      if (hasDoubleLightmap) {
        CGX::SetTevOrder(GX_TEVSTAGE1, texCoordIds[7], texMapIds[7], GX_COLOR1A1);
        CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_C2, GX_CC_TEXC, GX_CC_KONST, GX_CC_RASC);
        CGX::SetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVREG2);
        CGX::SetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K3);
        nextStage = GX_TEVSTAGE2;
      }
    }
    CGX::SetTevOrder(nextStage, texCoordIds[0], texMapIds[0], GX_COLOR1A1);
    CGX::SetTevColorIn(nextStage, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_RASC);
    CGX::SetTevColorOp(nextStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
    CGX::SetTevKColorSel(nextStage, GX_TEV_KCSEL_K0);
    nextStage = static_cast< GXTevStageID >(nextStage + 1);
    CGX::SetTevOrder(nextStage, texCoordIds[1], texMapIds[1], GX_COLOR0A0);
    CGX::SetTevColorIn(nextStage, GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_RASC);
    CGX::SetTevColorOp(nextStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
    nextStage = static_cast< GXTevStageID >(nextStage + 1);
    CGX::SetTevOrder(nextStage, texCoordIds[2], texMapIds[2], GX_COLOR1A1);
    CGX::SetTevColorIn(nextStage, GX_CC_ZERO, GX_CC_TEXC, hasLightmap ? GX_CC_C2 : GX_CC_RASC,
                       GX_CC_CPREV);
    CGX::SetTevColorOp(nextStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
    if (hasEnvBumpMap) {
      CGX::SetTevIndirect(nextStage, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_STU, GX_ITM_0, GX_ITW_OFF,
                          GX_ITW_OFF, false, false, GX_ITBA_OFF);
    }
    nextStage = static_cast< GXTevStageID >(nextStage + 1);
    break;
  }
  case kFT_Lava: {
    CGX::SetTevOrder(GX_TEVSTAGE0, texCoordIds[0], texMapIds[0], GX_COLOR0A0);
    CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_RASC);
    CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
    CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
    CGX::SetTevOrder(GX_TEVSTAGE1, texCoordIds[1], texMapIds[1], GX_COLOR0A0);
    CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_RASC);
    CGX::SetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
    CGX::SetTevOrder(GX_TEVSTAGE2, texCoordIds[2], texMapIds[2], GX_COLOR_NULL);
    CGX::SetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_CPREV);
    CGX::SetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
    nextStage = GX_TEVSTAGE3;
    if (hasBumpMap) {
      CGX::SetTevOrder(GX_TEVSTAGE3, texCoordIds[3], texMapIds[3], GX_COLOR_NULL);
      CGX::SetTevColorIn(GX_TEVSTAGE3, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_HALF);
      CGX::SetTevColorOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, false, GX_TEVREG0);
      CGX::SetTevOrder(GX_TEVSTAGE4, static_cast< GXTexCoordID >(texCoordIds[3] + 1), texMapIds[3],
                       GX_COLOR_NULL);
      CGX::SetTevColorIn(GX_TEVSTAGE4, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_C0);
      CGX::SetTevColorOp(GX_TEVSTAGE4, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVREG0);
      CGX::SetTevOrder(GX_TEVSTAGE5, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
      CGX::SetTevColorIn(GX_TEVSTAGE5, GX_CC_ZERO, GX_CC_CPREV, GX_CC_C0, GX_CC_ZERO);
      CGX::SetTevColorOp(GX_TEVSTAGE5, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_2, true, GX_TEVPREV);
      nextStage = GX_TEVSTAGE6;
    }
    break;
  }
  case kFT_ThickLava: {
    CGX::SetTevOrder(GX_TEVSTAGE0, texCoordIds[0], texMapIds[0], GX_COLOR0A0);
    CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_RASC);
    CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
    CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
    CGX::SetTevOrder(GX_TEVSTAGE1, texCoordIds[1], texMapIds[1], GX_COLOR0A0);
    CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_RASC);
    CGX::SetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
    CGX::SetTevOrder(GX_TEVSTAGE2, texCoordIds[2], texMapIds[2], GX_COLOR_NULL);
    CGX::SetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_CPREV);
    CGX::SetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
    nextStage = GX_TEVSTAGE3;
    if (hasBumpMap) {
      CGX::SetTevOrder(GX_TEVSTAGE3, texCoordIds[3], texMapIds[3], GX_COLOR_NULL);
      CGX::SetTevColorIn(GX_TEVSTAGE3, GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_ZERO);
      CGX::SetTevColorOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_2, true, GX_TEVPREV);
      nextStage = GX_TEVSTAGE4;
    }
    break;
  }
  default: {
    static bool setupDone = false;
    if (!setupDone) {
      setupDone = true;
    }
    break;
  }
  }

  CGX::SetNumTevStages(static_cast< u8 >(nextStage));
  int lastStage = nextStage - 1;
  CGX::SetTevAlphaIn(static_cast< GXTevStageID >(lastStage), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
                     GX_CA_KONST);
  CGX::SetTevAlphaOp(static_cast< GXTevStageID >(lastStage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                     true, GX_TEVPREV);
  CGX::SetTevKAlphaSel(static_cast< GXTevStageID >(lastStage), GX_TEV_KASEL_K0_A);

  if (mgr.GetThermalDrawFlag() == kTD_Hot) {
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
  } else {
    CGX::SetBlendMode(alpha == 1.f ? GX_BM_NONE : GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA,
                      GX_LO_CLEAR);
  }

  CGX::SetZMode(true, GX_LEQUAL, false);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
  GXSetCullMode(GX_CULL_NONE);
}

CFluidPlaneCPURender::SPatchInfo::SPatchInfo(const CVector3f& localMin, const CVector3f& localMax,
                                             const CVector3f& pos, float rippleResolution,
                                             float tileSize, float wavecapIntensityScale,
                                             int numSubdivisionsInHField, int normalMode,
                                             int redShift, int greenShift, uchar blueShift,
                                             int tileX, int gridDimX, int gridDimY, int tileY,
                                             const char* gridFlags) {
  short maxSubsX;
  short xSubs =
      CCast::FtoS(1.f + (localMax.GetX() - localMin.GetX()) / rippleResolution - FLT_EPSILON) + 2;
  maxSubsX = (short)(numSubdivisionsInHField + 2);
  mXSubdivs = rstl::min_val< short >(maxSubsX, xSubs);
  short maxSubsY;
  short ySubs =
      CCast::FtoS(1.f + (localMax.GetY() - localMin.GetY()) / rippleResolution - FLT_EPSILON) + 2;
  maxSubsY = (short)(numSubdivisionsInHField + 2);
  mYSubdivs = rstl::min_val< short >(maxSubsY, ySubs);
  mLocalMinX = localMin.GetX();
  mLocalMinY = localMin.GetY();
  mGlobalMinX = mLocalMinX + pos.GetX();
  mGlobalMinY = mLocalMinY + pos.GetY();
  mTileSize = tileSize;
  mRippleResolution = rippleResolution;
  mTileHypRadius = fast_sqrt(0.25f * (tileSize * tileSize + tileSize * tileSize));
  mOoTileSize = 1.f / mTileSize;
  mOoRippleResolution = 1.f / mRippleResolution;
  mTileX = (short)tileX;
  mGridDimX = (short)gridDimX;
  mGridDimY = (short)gridDimY;
  mTileY = (short)tileY;
  mGridFlags = gridFlags;
  mRedShift = static_cast< uchar >(redShift);
  mGreenShift = static_cast< uchar >(greenShift);
  mBlueShift = static_cast< uchar >(blueShift);
  mNormalMode = static_cast< uchar >(normalMode);
  mWavecapIntensityScale = wavecapIntensityScale;
}

extern void RenderPatch(const CFluidPlaneCPURender::SPatchInfo& info, bool noRipples,
                        bool flaggedGridGen);
const bool UpdatePatch(float time, CFluidPlaneCPURender::SPatchInfo& info,
                       const CFluidPlaneCPU& fluidPlane, const CVector3f& areaCenter,
                       const rstl::optional_object< CRippleManager >& rippleManager, int fromX,
                       int toX, int fromY, int toY);

void CFluidPlaneCPU::Render(const CStateManager& mgr, float alpha, const CAABox& aabb,
                            const CTransform4f& xf, const CTransform4f& areaXf, bool noNormals,
                            const CFrustumPlanes& frustum,
                            const rstl::optional_object< CRippleManager >& rippleManager,
                            TUniqueId waterId, const char* gridFlags, int gridDimX, int gridDimY,
                            const CVector3f& areaCenter) const {
  if (!gkWaterEnable) {
    return;
  }

  CScriptWater* waterPtr =
      TCastToPtr< CScriptWater >(const_cast< CEntity* >(mgr.GetObjectById(waterId)));
  RenderSetup(mgr, alpha, xf, areaXf, aabb, waterPtr);

  float time = mgr.GetFluidPlaneManager()->GetTime();
  CGX::ResetVtxDescv();

  int normalMode;
  if (HasBumpMap() && gkWaterBumpMapping) {
    normalMode = CFluidPlaneCPURender::kNM_NBT;
  } else if (!noNormals) {
    normalMode = CFluidPlaneCPURender::kNM_Normals;
  } else {
    normalMode = CFluidPlaneCPURender::kNM_NoNormals;
  }

  CGX::SetVtxDesc(GX_VA_POS, GX_DIRECT);
  CGX::SetVtxDesc(GX_VA_CLR0, GX_DIRECT);

  switch (normalMode) {
  case CFluidPlaneCPURender::kNM_NBT:
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NBT, GX_CLR_RGBA, GX_RGB8, 6);
    GXSetVtxDesc(GX_VA_NBT, GX_DIRECT);
    break;
  case CFluidPlaneCPURender::kNM_Normals:
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_CLR_RGB, GX_RGB8, 6);
    CGX::SetVtxDesc(GX_VA_NRM, GX_DIRECT);
    break;
  default:
    break;
  }

  CFluidPlaneCPURender::numSubdivisionsInTile = mTileSubdivisions;
  float rippleResolution = mRippleResolution;
  float rippleResolutionRecip = 1.f / rippleResolution;
  int numTiles = 42 / CFluidPlaneCPURender::numSubdivisionsInTile;
  CFluidPlaneCPURender::numTilesInHField = rstl::min_val(numTiles, kMaxTilesInHField);
  CFluidPlaneCPURender::numSubdivisionsInHField =
      CFluidPlaneCPURender::numTilesInHField * CFluidPlaneCPURender::numSubdivisionsInTile;

  CVector2f aabbCenter(aabb.GetCenterPoint().GetX(), aabb.GetCenterPoint().GetY());

  float aabbMinX = aabb.GetMinPoint().GetX();
  float aabbMinY = aabb.GetMinPoint().GetY();
  float aabbMinZ = aabb.GetMinPoint().GetZ();
  float aabbMaxX = aabb.GetMaxPoint().GetX();
  float aabbMaxY = aabb.GetMaxPoint().GetY();
  float aabbMaxZ = aabb.GetMaxPoint().GetZ();

  CVector2f ripplePitch(
      rippleResolution * CCast::ToReal32(CFluidPlaneCPURender::numSubdivisionsInHField),
      rippleResolution * CCast::ToReal32(CFluidPlaneCPURender::numSubdivisionsInHField));

  int redShift = 0;
  int greenShift = 0;
  int blueShift = 0;
  float wavecapIntensityScale = gpTweakGame->GetWavecapIntensityNormal();

  switch (mFluidType) {
  case kFT_PoisonWater:
    wavecapIntensityScale = gpTweakGame->GetWavecapIntensityPoison();
    redShift = 1;
    blueShift = 1;
    break;
  case kFT_Lava:
  case kFT_ThickLava:
    wavecapIntensityScale = gpTweakGame->GetWavecapIntensityLava();
    blueShift = 8;
    greenShift = 8;
    break;
  default:
    break;
  }

  if (waterPtr != nullptr) {
    const CGameCamera& camera = mgr.GetCameraManager()->GetCurrentCamera(mgr);
    CVector3f camPos(camera.GetTranslation());
    const CPlane& waterPlane = CPlane(waterPtr->GetTriggerBoundsWR().GetMaxPoint().GetZ(),
                                      CUnitVector3f(0.f, 0.f, 1.f, CUnitVector3f::kN_Yes));
    float cameraPenetration = waterPlane.GetHeight(camPos);
    bool inRange = false;
    if (cameraPenetration >= 0.5f || cameraPenetration < 0.f) {
      inRange = true;
    }
    float scale;
    if (inRange) {
      scale = 1.f;
    } else {
      scale = 2.f * cameraPenetration;
    }
    wavecapIntensityScale *= scale;
  }

  int patchDimX = waterPtr != nullptr && waterPtr->GetPatchDimensionX() != 0
                      ? waterPtr->GetPatchDimensionX()
                      : 128;
  int patchDimY = waterPtr != nullptr && waterPtr->GetPatchDimensionY() != 0
                      ? waterPtr->GetPatchDimensionY()
                      : 128;

  uchar blueShiftU = static_cast< uchar >(blueShift);
  uchar greenShiftU = static_cast< uchar >(greenShift);
  uchar redShiftU = static_cast< uchar >(redShift);

  int tileY = 0;
  float curY = aabbMinY;
  for (int i = 0; curY < aabbMaxY && i < patchDimY; ++i) {
    float remDivsY = rippleResolutionRecip * (aabbMaxY - curY);
    int tileX = 0;
    float curX = aabbMinX;
    for (int j = 0; curX < aabbMaxX && j < patchDimX; ++j) {
      int renderFlags = waterPtr->GetPatchRenderFlags(j, i);
      if (renderFlags != 0) {
        const short xSubdivs = rstl::min_val< short >(
            static_cast< short >(CFluidPlaneCPURender::numSubdivisionsInHField),
            CCast::FtoS(rippleResolutionRecip * (aabbMaxX - curX)));
        const short ySubdivs = rstl::min_val< short >(
            static_cast< short >(CFluidPlaneCPURender::numSubdivisionsInHField),
            CCast::FtoS(remDivsY));

        CVector3f patchMax = CVector3f(rippleResolution * CCast::StoF(xSubdivs) + curX,
                                       rippleResolution * CCast::StoF(ySubdivs) + curY, aabbMaxZ);
        CVector3f localMax = patchMax + xf.GetTranslation();
        const CVector3f& patchMinBounds = CVector3f(curX, curY, aabbMinZ);
        CVector3f localMin = patchMinBounds + xf.GetTranslation();

        CAABox testAABB(localMin, localMax);
        if (frustum.BoxInFrustumPlanes(testAABB)) {
          CVector3f pos = xf.GetTranslation();
          CVector3f patchMin(curX, curY, aabbMinZ);

          CFluidPlaneCPURender::SPatchInfo info(
              patchMin, patchMax, pos, rippleResolution, mTileSize, wavecapIntensityScale,
              CFluidPlaneCPURender::numSubdivisionsInHField, normalMode, redShiftU, greenShiftU,
              blueShiftU, tileX, gridDimX, gridDimY, tileY, gridFlags);

          CFluidPlaneCPURender::SPatchInfo& lcInfo =
              *reinterpret_cast< CFluidPlaneCPURender::SPatchInfo* >(LCGetBase());
          lcInfo = info;

          int fromX = tileX != 0 ? 2 - CFluidPlaneCPURender::numSubdivisionsInTile : 0;
          int toX;
          if (tileX != gridDimX - 1) {
            toX = lcInfo.mXSubdivs + CFluidPlaneCPURender::numSubdivisionsInTile - 2;
          } else {
            toX = lcInfo.mXSubdivs;
          }

          int fromY = tileY != 0 ? 2 - CFluidPlaneCPURender::numSubdivisionsInTile : 0;
          int toY;
          if (tileY != gridDimY - 1) {
            toY = lcInfo.mYSubdivs + CFluidPlaneCPURender::numSubdivisionsInTile - 2;
          } else {
            toY = lcInfo.mYSubdivs;
          }

          bool noRipples =
              UpdatePatch(time, lcInfo, *this, areaCenter, rippleManager, fromX, toX, fromY, toY);
          RenderPatch(lcInfo, noRipples, renderFlags == 1);
        }
      }
      curX += ripplePitch.GetX();
      tileX += CFluidPlaneCPURender::numTilesInHField;
    }
    curY += ripplePitch.GetY();
    tileY += CFluidPlaneCPURender::numTilesInHField;
  }

  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NBT, GX_CLR_RGBA, GX_F32, 6);
  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_CLR_RGB, GX_F32, 6);
  GXSetVtxDesc(GX_VA_NBT, GX_NONE);
  GXSetCullMode(GX_CULL_FRONT);
  RenderCleanup();
}

void CFluidPlaneCPU::RenderCleanup() const {
  if (!gkWaterEnable) {
    return;
  }

  LCQueueWait(0);

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
  CTransform4f nrmMtx(CGraphics::GetViewMatrix().GetRotation().GetQuickInverse());
  mtx[0][0] = nrmMtx.Get00();
  mtx[0][1] = nrmMtx.Get01();
  mtx[0][2] = nrmMtx.Get02();
  mtx[0][3] = nrmMtx.Get03();
  mtx[1][0] = nrmMtx.Get10();
  mtx[1][1] = nrmMtx.Get11();
  mtx[1][2] = nrmMtx.Get12();
  mtx[1][3] = nrmMtx.Get13();
  mtx[2][0] = nrmMtx.Get20();
  mtx[2][1] = nrmMtx.Get21();
  mtx[2][2] = nrmMtx.Get22();
  mtx[2][3] = nrmMtx.Get23();
  GXLoadNrmMtxImm(mtx, GX_PNMTX0);

  CGX::SetChanCtrl(CGX::Channel1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_CLAMP,
                   GX_AF_SPOT);
  CGX::SetNumChans(1);

  CGraphics::SetLightState(CGraphics::GetLightMask());
}
