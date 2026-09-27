#include "MetroidPrime/CFluidPlaneDoor.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CFluidUVMotion.hpp"
#include "dolphin/gx/GXEnum.h"
#include "dolphin/gx/GXTransform.h"
#include "dolphin/types.h"
#include "rstl/reserved_vector.hpp"

#include <Kyoto/Graphics/CGX.hpp>
#include <dolphin/os/OSCache.h>
#include <rstl/math.hpp>
#include <string.h>

extern void RenderPatch(const CFluidPlaneCPURender::SPatchInfo& info, bool noRipples,
                        bool flaggedRendering);

CFluidPlaneDoor::CFluidPlaneDoor(const CAssetId texPattern1, const CAssetId texPattern2,
                                 const CAssetId texColor, const float tileSize,
                                 const uint tileSubdivisions, const EFluidType fluidType,
                                 const float alpha, const CFluidUVMotion& uvMotion)
: CFluidPlane(texPattern1, texPattern2, texColor, alpha, fluidType, 0.5f, uvMotion)
, mTileSize(tileSize)
, mTileSubdivisions(tileSubdivisions & ~1)
, mRippleResolution(tileSize / mTileSubdivisions) {}

CFluidPlaneDoor::~CFluidPlaneDoor() {}

void CFluidPlaneDoor::RenderSetup(const CStateManager& mgr, float alpha, const CTransform4f& xf,
                                  const CAABox& bounds, const bool noNormals) const {
  static GXColor white = {255, 255, 255, 255};
  float uvT = mgr.GetFluidPlaneManager()->GetUVTime();
  gpRender->SetModelMatrix(xf);
  CGX::SetChanMatColor(CGX::Channel0, white);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::SetNumChans(0);

  if (HasTexturePattern1()) {
    GetTexturePattern1()->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
  }

  if (HasTexturePattern2()) {
    GetTexturePattern2()->Load(GX_TEXMAP1, CTexture::kCM_Repeat);
  }

  if (HasColorTexture()) {
    GetColorTexture()->Load(GX_TEXMAP2, CTexture::kCM_Repeat);
  }

  CGX::SetNumTexGens(3);

  float uvs[3][2];
  GetUVMotion().CalculateFluidTextureOffset(uvT, uvs);

  const rstl::reserved_vector< CFluidUVMotion::SFluidLayerMotion, CFluidUVMotion::kFM_NumLayers >&
      layers = GetUVMotion().GetFluidLayers();
  float scale1[2][4] = {
      {
          layers[1].mUvScale,
          0.f,
          0.f,
          uvs[1][0],
      },
      {
          0.f,
          layers[1].mUvScale,
          0.f,
          uvs[1][1],
      },
  };
  float scale2[2][4] = {
      {
          layers[2].mUvScale,
          0.f,
          0.f,
          uvs[2][0],
      },
      {
          0.f,
          layers[2].mUvScale,
          0.f,
          uvs[2][1],
      },
  };
  float scale0[2][4] = {
      {
          layers[0].mUvScale,
          0.f,
          0.f,
          uvs[0][0],
      },
      {
          0.f,
          layers[0].mUvScale,
          0.f,
          uvs[0][1],
      },
  };
  GXLoadTexMtxImm(scale1, GX_TEXMTX5, GX_MTX2x4);
  GXLoadTexMtxImm(scale2, GX_TEXMTX6, GX_MTX2x4);
  GXLoadTexMtxImm(scale0, GX_TEXMTX7, GX_MTX2x4);

  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX5, false, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX6, false, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX7, false, GX_PTIDENTITY);

  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
  CGX::SetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR_NULL);

  CGX::SetNumTevStages(3);

  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_ZERO);
  CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_ZERO);
  CGX::SetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_CPREV);

  CGX::SetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);

  CGX::SetTevKColor(GX_KCOLOR0, CColor(1.f, 1.f, 1.f, alpha).GetGXColor());
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE2, GX_TEV_KASEL_K0_A);

  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE2);

  CGX::SetBlendMode(alpha == 1.f ? GX_BM_NONE : GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA,
                    GX_LO_CLEAR);
  CGX::SetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
}

void CFluidPlaneDoor::Render(const CStateManager& mgr, float alpha, const CAABox& aabb,
                             const CTransform4f& xf, const CTransform4f& areaXf,
                             const bool noNormals, const CFrustumPlanes& frustum,
                             const rstl::optional_object< CRippleManager >& rippleManager,
                             TUniqueId waterId, const char* gridFlags, int gridDimX, int gridDimY,
                             const CVector3f& areaCenter) const {
  if (!gkWaterEnable) {
    return;
  }

  RenderSetup(mgr, alpha, xf, aabb, noNormals);
  CGX::ResetVtxDescv();
  CGX::SetVtxDesc(GX_VA_POS, GX_DIRECT);

  CFluidPlaneCPURender::numSubdivisionsInTile = mTileSubdivisions;
  float rippleResolution = mRippleResolution;
  CFluidPlaneCPURender::numTilesInHField = 42 / mTileSubdivisions;
  float ooSubdivSize = 1.f / rippleResolution;
  CFluidPlaneCPURender::numSubdivisionsInHField =
      CFluidPlaneCPURender::numTilesInHField * CFluidPlaneCPURender::numSubdivisionsInTile;

  CVector3f center1 = aabb.GetCenterPoint();
  float centerY = center1.GetY();
  CVector3f center2 = aabb.GetCenterPoint();
  CVector2f centerPlane(center2.GetX(), centerY);

  CVector3f aabbMin(aabb.GetMinPoint());
  float maxX = aabb.GetMaxPoint().GetX();
  float maxY = aabb.GetMaxPoint().GetY();
  float maxZ = aabb.GetMaxPoint().GetZ();

  float patchSizeX = rippleResolution * CCast::LtoF(CFluidPlaneCPURender::numSubdivisionsInHField);
  float patchSizeY = rippleResolution * CCast::LtoF(CFluidPlaneCPURender::numSubdivisionsInHField);
  CVector2f ripplePitch(patchSizeX, patchSizeY);

  float curX = aabbMin.GetX();
  for (; curX < maxX; curX += ripplePitch.GetX()) {
    float remSubdivsX = ooSubdivSize * (maxX - curX);
    float curY = aabbMin.GetY();
    for (; curY < maxY; curY += ripplePitch.GetY()) {
      short remXi = CCast::FtoS(remSubdivsX);
      short clampedX =
          rstl::min_val(static_cast< short >(CFluidPlaneCPURender::numSubdivisionsInHField), remXi);

      float remSubdivsY = ooSubdivSize * (maxY - curY);
      short remYi = CCast::FtoS(remSubdivsY);
      short clampedY =
          rstl::min_val(static_cast< short >(CFluidPlaneCPURender::numSubdivisionsInHField), remYi);

      CVector3f localMax(rippleResolution * CCast::StoF(clampedX) + curX,
                         rippleResolution * CCast::StoF(clampedY) + curY, maxZ);

      CAABox patchAABB(aabbMin, localMax);

      if (frustum.BoxInFrustumPlanes(patchAABB.GetTransformedAABox(xf))) {
        CFluidPlaneCPURender::SPatchInfo info(CVector3f(curX, curY, aabbMin.GetZ()), localMax,
                                              xf.GetTranslation(), rippleResolution, mTileSize,
                                              0.f, CFluidPlaneCPURender::numSubdivisionsInHField, 0,
                                              0, 0, 0, 0, 0, 0, 0, NULL);

        CFluidPlaneCPURender::SPatchInfo& lcInfo =
            *reinterpret_cast< CFluidPlaneCPURender::SPatchInfo* >(LCGetBase());
        lcInfo = info;

        RenderPatch(lcInfo, true, true);
      }
    }
  }

  RenderCleanup();
}

void CFluidPlaneDoor::RenderCleanup() const {
  LCQueueWait(0);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_TEX1, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX3x4, GX_TG_TEX2, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
}
