#include "GuiSys/CAuiImagePane.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"
#include "dolphin/gx/GXVert.h"
#include "rstl/math.hpp"

CGuiWidget* CAuiImagePane::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp) {
  CGuiWidgetParms parms = ReadWidgetHeader(frame, in);
  in.ReadLong();
  in.ReadLong();
  in.ReadLong();
  rstl::reserved_vector< CVector3f, 4 > coords(in);
  rstl::reserved_vector< CVector2f, 4 > uvs(in);
  CAuiImagePane* widget =
      rs_new CAuiImagePane(parms, sp, kInvalidAssetId, kInvalidAssetId, coords, uvs, true);
  widget->ParseBaseInfo(frame, in, parms);
  return widget;
}

void CAuiImagePane::WriteData(COutputStream& out, bool) const {
  out.Put(mTex0);
  out.Put(mTex1);
  out.Put(static_cast< int >(GetDrawFlags()));
  mCoords.PutTo(out);
  mUvs.PutTo(out);
}

CAuiImagePane::CAuiImagePane(const CGuiWidgetParms& parms, CSimplePool* sp, CAssetId tex0,
                             CAssetId tex1, const rstl::reserved_vector< CVector3f, 4 >& coords,
                             const rstl::reserved_vector< CVector2f, 4 >& uvs, bool initTex)
: CGuiWidget(parms)
, mTex0(tex0)
, mTex1(tex1)
, mUvBias0(0.f, 0.f)
, mUvBias1(0.f, 0.f)
, mCoords(coords)
, mUvs(uvs)
, mTileSize(CVector2f::Zero())
, mInterval(0.f)
, mFrameTimer(0.f)
, mFadeDuration(0.f)
, mDeResFactor(0.f)
, mFlashFactor(0.f) {
  if (initTex) {
    SetTextureID0(tex0, sp);
  }
}

void CAuiImagePane::SetAnimationParms(const CVector2f& tileSize, float interval,
                                      float fadeDuration) {
  mTileSize = tileSize;
  mInterval = interval;
  mFrameTimer = 0.f;
  mFadeDuration = fadeDuration;
}

void CAuiImagePane::Update(float dt) {
  mUvBias0[0] = CMath::ModF(mUvBias0.GetX(), 1.f);
  mUvBias0[1] = CMath::ModF(mUvBias0.GetY(), 1.f);
  if (!(mTileSize == CVector2f::Zero()) && mTex0Tok && mTex0Tok->GetObject()) {
    const CTexture& texture = *mTex0Tok->GetObject();
    const int columns = static_cast< int >(texture.GetWidth() / mTileSize.GetX());
    const int rows = static_cast< int >(texture.GetHeight() / mTileSize.GetY());
    mFrameTimer = CMath::ModF(mFrameTimer + dt * mInterval, columns * rows);
  }
  CGuiWidget::Update(dt);
}

void CAuiImagePane::Draw(const CGuiWidgetDrawParms& parms) const {
  CGraphics::SetModelMatrix(GetWorldTransform());
  if (!GetIsVisible() || !mTex0Tok) {
    return;
  }
  GetIsFinishedLoadingWidgetSpecific();
  const CTexture* texture = mTex0Tok->GetObject();
  if (!texture) {
    return;
  }

  const CColor color = GetModifiedColor().WithAlphaModulatedBy(parms.GetAlpha());
  CGraphics::SetDepthWriteMode(true, kE_LEqual,
                               GetDrawFlags() == kGMDF_Shadeless || GetDrawFlags() == kGMDF_Opaque);
  float alpha0 = 1.f;
  float alpha1 = 0.f;
  int frame0 = 0;
  int frame1 = 0;
  if (mInterval < 1.f && mInterval > 0.f) {
    frame0 = CCast::ToInt(mFrameTimer);
    const float columns = texture->GetWidth() / mTileSize.GetX();
    const float rows = texture->GetHeight() / mTileSize.GetY();
    frame1 = (frame0 + 1) % CCast::ToInt(columns * rows);
    const float fraction = mFrameTimer - CCast::ToReal32(frame0);
    float blend;
    if (mFadeDuration == 0.f) {
      blend = 1.f;
    } else {
      blend = rstl::min_val(1.f, fraction / mFadeDuration);
    }
    alpha1 = blend;
    alpha0 = 1.f - blend;
  }

  CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
  DoDrawImagePane(CColor::Modulate(color, CColor::Black().WithAlphaOf(0.5f)), *texture, frame0, 1.f,
                  true);
  if (mFlashFactor > 0.f) {
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
    CColor flashColor = GetModifiedColor().WithAlphaOf(mFlashFactor);
    DoDrawImagePane(flashColor, *texture, frame0, alpha0, false);
    if (alpha1 > 0.f) {
      DoDrawImagePane(flashColor, *texture, frame1, alpha1, false);
    }
  }

  switch (GetDrawFlags()) {
  case kGMDF_Shadeless:
  case kGMDF_Opaque:
    CGraphics::SetBlendMode(kBM_Blend, kBF_One, kBF_Zero, kLO_Clear);
    DoDrawImagePane(color, *texture, frame0, alpha0, false);
    if (alpha1 > 0.f) {
      DoDrawImagePane(color, *texture, frame1, alpha1, false);
    }
    break;
  case kGMDF_Alpha:
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
    DoDrawImagePane(color, *texture, frame0, alpha0, false);
    if (alpha1 > 0.f) {
      DoDrawImagePane(color, *texture, frame1, alpha1, false);
    }
    break;
  case kGMDF_Additive:
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
    DoDrawImagePane(color, *texture, frame0, alpha0, false);
    if (alpha1 > 0.f) {
      DoDrawImagePane(color, *texture, frame1, alpha1, false);
    }
    break;
  case kGMDF_AlphaAdditiveOverdraw:
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
    DoDrawImagePane(color, *texture, frame0, alpha0, false);
    if (alpha1 > 0.f) {
      DoDrawImagePane(color, *texture, frame1, alpha1, false);
    }
    CGraphics::SetBlendMode(kBM_Blend, kBF_One, kBF_One, kLO_Clear);
    DoDrawImagePane(color, *texture, frame0, alpha0, false);
    if (alpha1 > 0.f) {
      DoDrawImagePane(color, *texture, frame1, alpha1, false);
    }
    break;
  }
}

void CAuiImagePane::DoDrawImagePane(CColor color, const CTexture& texture, int frame, float alpha,
                                    bool noBlur) const {
  const CColor useColor = color.WithAlphaModulatedBy(alpha);
  rstl::reserved_vector< CVector2f, 4 > frameUVs;
  const CVector2f* uvs;
  if (!(mTileSize == CVector2f::Zero())) {
    const CTexture& tileTexture = *mTex0Tok->GetObject();
    const int width = tileTexture.GetWidth();
    const int height = tileTexture.GetHeight();
    const int columns = CCast::ToInt(width / mTileSize.GetX());
    const int rows = CCast::ToInt(height / mTileSize.GetY());
    const float tileWidth = mTileSize.GetX() / width;
    const float tileHeight = mTileSize.GetY() / height;
    const float x0 = tileWidth * (frame % columns);
    const float y0 = tileHeight * (rows - frame / rows);
    const float x1 = x0 + tileWidth;
    const float y1 = y0 - tileHeight;
    frameUVs.push_back(CVector2f(x0, y0));
    frameUVs.push_back(CVector2f(x0, y1));
    frameUVs.push_back(CVector2f(x1, y0));
    frameUVs.push_back(CVector2f(x1, y1));
    uvs = frameUVs.data();
  } else {
    uvs = mUvs.data();
  }

  if (noBlur) {
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulateAlpha);
    CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
    texture.Load(GX_TEXMAP0, CTexture::kCM_Repeat);
    CGraphics::StreamBegin(kP_TriangleStrip);
    CGraphics::StreamColor(useColor);
    for (int i = 0; i < 4; ++i) {
      CGraphics::StreamTexcoord(uvs[i] + mUvBias0);
      CGraphics::StreamVertex(mCoords[i]);
    }
    CGraphics::StreamEnd();
    return;
  }
  const int numMips = texture.GetNumberOfMipMaps();
  if ((mDeResFactor == 0.f && alpha == 1.f) || numMips == 1) {
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
    CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
    texture.LoadMipLevel(0, GX_TEXMAP0, CTexture::kCM_Repeat);
    CGraphics::StreamBegin(kP_TriangleStrip);
    CGraphics::StreamColor(useColor);
    for (int i = 0; i < 4; ++i) {
      CGraphics::StreamTexcoord(uvs[i] + mUvBias0);
      CGraphics::StreamVertex(mCoords[i]);
    }
    CGraphics::StreamEnd();
  } else {
    const int mipCount = numMips - 1;
    float fadeFactor = (1.f - mDeResFactor) * alpha;
    const float fadeQ = -(fadeFactor * fadeFactor * fadeFactor - 1.f);
    fadeFactor = fadeQ * mipCount;
    const int mip1 = static_cast< int >(fadeFactor);
    const int mip2 = fadeQ == static_cast< float >(mip1 / mipCount) ? mip1 : mip1 + 1;
    const float weight1 = fadeFactor - mip1;
    const float weight0 = 1.f - weight1;
    texture.LoadMipLevel(mip1, GX_TEXMAP0, CTexture::kCM_Repeat);
    texture.LoadMipLevel(mip2, GX_TEXMAP1, CTexture::kCM_Repeat);
    GXVtxDescList desc[] = {{GX_VA_POS, GX_DIRECT}, {GX_VA_TEX0, GX_DIRECT}, {GX_VA_NULL, GX_NONE}};
    CGX::SetVtxDescv(desc);
    CGX::SetNumChans(0);
    CGX::SetNumTexGens(2);
    CGX::SetNumTevStages(2);
    for (GXTevStageID stage = GX_TEVSTAGE0; stage < GX_TEVSTAGE2;
         stage = static_cast< GXTevStageID >(stage + 1)) {
      GXTevColorArg prevColor = GX_CC_CPREV;
      if (stage == GX_TEVSTAGE0) {
        prevColor = GX_CC_ZERO;
      }
      CGX::SetTevColorIn(stage, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, prevColor);
      CGX::SetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST,
                         stage == GX_TEVSTAGE0 ? GX_CA_ZERO : GX_CA_APREV);
      CGX::SetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
      CGX::SetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
    }
    CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
    CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
    CGX::SetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_K1_A);
    CGX::SetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K1);
    const CColor color0 = CColor::Modulate(useColor, CColor(weight0, weight0, weight0, weight0));
    const CColor color1 = CColor::Modulate(useColor, CColor(weight1, weight1, weight1, weight1));
    CGX::SetTevKColor(GX_KCOLOR0, color0.GetGXColor());
    CGX::SetTevKColor(GX_KCOLOR1, color1.GetGXColor());
    CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
    CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
    CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
    CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
    for (int i = 0; i < 4; ++i) {
      CVector2f uv = uvs[i] + mUvBias0;
      const CVector3f& pos = mCoords[i];
      GXPosition3f32(pos.GetX(), pos.GetY(), pos.GetZ());
      GXTexCoord2f32(uv[0], uv[1]);
    }
    CGX::End();
  }
}

void CAuiImagePane::SetTextureID0(CAssetId tex, CSimplePool* sp) {
  mTex0 = tex;
  if (sp) {
    if (mTex0 != kInvalidAssetId) {
      mTex0Tok = TCachedToken< CTexture >(sp->GetObj(SObjectTag('TXTR', mTex0)));
      mTex0Tok->Lock();
    } else {
      mTex0Tok = rstl::optional_object_null();
    }
  }
}

bool CAuiImagePane::GetIsFinishedLoadingWidgetSpecific() const {
  if (!mTex0Tok) {
    return true;
  }
  return mTex0Tok->TryCache();
}
