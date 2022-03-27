#include "Runtime/GuiSys/CAuiImagePane.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/Camera/CCameraFilter.hpp"
#include "Runtime/Graphics/CTexture.hpp"
#include "Runtime/Graphics/CGX.hpp"
#include "Runtime/GuiSys/CGuiWidgetDrawParms.hpp"

namespace metaforce {

CAuiImagePane::CAuiImagePane(const CGuiWidgetParms& parms, CSimplePool* sp, CAssetId tex0, CAssetId tex1,
                             rstl::reserved_vector<zeus::CVector3f, 4>&& coords,
                             rstl::reserved_vector<zeus::CVector2f, 4>&& uvs, bool initTex)
: CGuiWidget(parms), xc8_tex0(tex0), xcc_tex1(tex1), xe0_coords(std::move(coords)), x114_uvs(std::move(uvs)) {
  if (initTex)
    SetTextureID0(tex0, sp);
}

std::shared_ptr<CGuiWidget> CAuiImagePane::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp) {
  CGuiWidgetParms parms = ReadWidgetHeader(frame, in);
  in.ReadLong();
  in.ReadLong();
  in.ReadLong();
  u32 coordCount = in.ReadLong();
  rstl::reserved_vector<zeus::CVector3f, 4> coords;
  for (u32 i = 0; i < coordCount; ++i)
    coords.push_back(in.Get<zeus::CVector3f>());
  u32 uvCount = in.ReadLong();
  rstl::reserved_vector<zeus::CVector2f, 4> uvs;
  for (u32 i = 0; i < uvCount; ++i)
    uvs.push_back(in.Get<zeus::CVector2f>());
  std::shared_ptr<CGuiWidget> ret =
      std::make_shared<CAuiImagePane>(parms, sp, CAssetId(), CAssetId(), std::move(coords), std::move(uvs), true);
  ret->ParseBaseInfo(frame, in, parms);
  return ret;
}

void CAuiImagePane::Reset(ETraversalMode mode) {
  xc8_tex0 = CAssetId();
  xb8_tex0Tok = TLockedToken<CTexture>();
  CGuiWidget::Reset(mode);
}

void CAuiImagePane::Update(float dt) {
  xd0_uvBias0.x() = std::fmod(xd0_uvBias0.x(), 1.f);
  xd0_uvBias0.y() = std::fmod(xd0_uvBias0.y(), 1.f);
  if (x138_tileSize != zeus::skZero2f && xb8_tex0Tok.IsLoaded()) {
    zeus::CVector2f tmp = zeus::CVector2f(xb8_tex0Tok->GetWidth(), xb8_tex0Tok->GetHeight()) / x138_tileSize;
    x144_frameTimer = std::fmod(x144_frameTimer + dt * x140_interval, std::floor(tmp.x()) * std::floor(tmp.y()));
  }

  CGuiWidget::Update(dt);
}

void CAuiImagePane::DoDrawImagePane(const zeus::CColor& color, CTexture& tex, int frame, float alpha,
                                    bool noBlur) const {
  zeus::CColor useColor = color;
  useColor.a() *= alpha;

  rstl::reserved_vector<zeus::CVector2f, 4> vec;
  const rstl::reserved_vector<zeus::CVector2f, 4>* useUVs;
  if (x138_tileSize != zeus::skZero2f) {
    const zeus::CVector2f res(xb8_tex0Tok->GetWidth(), xb8_tex0Tok->GetHeight());
    const zeus::CVector2f tmp = res / x138_tileSize;
    const zeus::CVector2f tmpRecip = x138_tileSize / res;
    const float x0 = tmpRecip.x() * static_cast<float>(frame % static_cast<int>(tmp.x()));
    const float x1 = x0 + tmpRecip.x();
    const float y0 = tmpRecip.y() * static_cast<float>(frame % static_cast<int>(tmp.y()));
    const float y1 = y0 - tmpRecip.y();
    vec.push_back(zeus::CVector2f(x0, y0));
    vec.push_back(zeus::CVector2f(x0, y1));
    vec.push_back(zeus::CVector2f(x1, y0));
    vec.push_back(zeus::CVector2f(x1, y1));
    useUVs = &vec;
  } else {
    useUVs = &x114_uvs;
  }

  if (!noBlur) {
    if ((x14c_deResFactor == 0.f && alpha == 1.f) || tex.GetNumberOfMipMaps() == 1) {
      CGraphics::SetTevOp(ERglTevStage::Stage0, CTevCombiners::sTevPass805a5ebc);
      CGraphics::SetTevOp(ERglTevStage::Stage1, CTevCombiners::skPassThru);
      tex.LoadMipLevel(0.f, GX::TEXMAP0, EClampMode::Repeat);
      CGraphics::StreamBegin(GX::TRIANGLESTRIP);
      CGraphics::StreamColor(useColor);
      for (u32 i = 0; i < useUVs->size(); ++i) {
        CGraphics::StreamTexcoord((*useUVs)[i] + xd0_uvBias0);
        CGraphics::StreamVertex(xe0_coords[i]);
      }
      CGraphics::StreamEnd();
    } else {
      u32 mipCount = tex.GetNumberOfMipMaps() - 1;
      float fadeFactor = (1.f - x14c_deResFactor) * alpha;
      float fadeQ = -(fadeFactor * fadeFactor * fadeFactor - 1.f);
      fadeFactor = fadeQ * static_cast<float>(mipCount);
      u32 mip1 = fadeFactor;
      u32 mip2 = mip1;
      if (fadeQ != static_cast<float>(mip1 / mipCount)) {
        mip2 = mip1 + 1;
      }

      float rgba1 = (fadeFactor - static_cast<float>(mip1));
      float rgba2 = 1.f - rgba1;
      tex.LoadMipLevel(mip1, GX::TexMapID::TEXMAP0, EClampMode::Repeat);
      tex.LoadMipLevel(mip2, GX::TexMapID::TEXMAP1, EClampMode::Repeat);
      std::array<GX::VtxDescList, 3> list{{
          {GX::VA_POS, GX::DIRECT},
          {GX::VA_TEX0, GX::DIRECT},
          GX::VtxDescList{},
      }};

      CGX::SetVtxDescv(list.data());
      CGX::SetNumChans(0);
      CGX::SetNumTexGens(2);
      CGX::SetNumTevStages(2);
      GX::TevStageID stage = GX::TEVSTAGE0;
      while (stage < GX::TEVSTAGE2) {
        GX::TevColorArg colorD = stage == GX::TEVSTAGE0 ? GX::CC_ZERO : GX::CC_CPREV;
        CGX::SetTevColorIn(stage, GX::CC_ZERO, GX::CC_TEXC, GX::CC_KONST, colorD);
        GX::TevAlphaArg alphaD = stage == GX::TEVSTAGE0 ? GX::CA_ZERO : GX::CA_APREV;
        CGX::SetTevAlphaIn(stage, GX::CA_ZERO, GX::CA_TEXA, GX::CA_KONST, alphaD);
        CGX::SetTevColorOp(stage, GX::TEV_ADD, GX::TB_ZERO, GX::CS_SCALE_1, true, GX::TEVPREV);
        CGX::SetTevAlphaOp(stage, GX::TEV_ADD, GX::TB_ZERO, GX::CS_SCALE_1, true, GX::TEVPREV);
        stage = static_cast<GX::TevStageID>(stage + GX::TEVSTAGE1);
      }
      CGX::SetTevKAlphaSel(GX::TEVSTAGE0, GX::TEV_KASEL_K0_A);
      CGX::SetTevKColorSel(GX::TEVSTAGE0, GX::TEV_KCSEL_K0);
      CGX::SetTevKAlphaSel(GX::TEVSTAGE1, GX::TEV_KASEL_K1_A);
      CGX::SetTevKColorSel(GX::TEVSTAGE1, GX::TEV_KCSEL_K1);
      zeus::CColor col1 = useColor * zeus::CColor(rgba2, rgba2, rgba2, rgba2);
      zeus::CColor col2 = useColor * zeus::CColor(rgba1, rgba1, rgba1, rgba1);
      CGX::SetTevKColor(GX::KCOLOR0, col1);
      CGX::SetTevKColor(GX::KCOLOR1, col2);
      CGX::SetTevOrder(GX::TEVSTAGE0, GX::TEXCOORD0, GX::TEXMAP0, GX::COLOR_NULL);
      CGX::SetTevOrder(GX::TEVSTAGE1, GX::TEXCOORD1, GX::TEXMAP1, GX::COLOR_NULL);
      CGX::SetTexCoordGen(GX::TEXCOORD0, GX::TG_MTX2x4, GX::TG_TEX0, GX::IDENTITY, false, GX::PTIDENTITY);
      CGX::SetTexCoordGen(GX::TEXCOORD1, GX::TG_MTX2x4, GX::TG_TEX0, GX::IDENTITY, false, GX::PTIDENTITY);
      CGX::Begin(GX::Primitive::TRIANGLESTRIP, GX::VTXFMT0, 4);
      for (u32 idx = 0; const auto& coord : xe0_coords) {
        GXPosition3f32(coord);
        GXTexCoord2f32((*useUVs)[idx] + xd0_uvBias0);
        ++idx;
      }
      CGX::End();
    }
  } else {
    CGraphics::SetTevOp(ERglTevStage::Stage0, CTevCombiners::sTevPass805a5fec);
    CGraphics::SetTevOp(ERglTevStage::Stage1, CTevCombiners::skPassThru);
    tex.Load(GX::TEXMAP0, EClampMode::Repeat);
    CGraphics::StreamBegin(GX::TRIANGLESTRIP);
    CGraphics::StreamColor(useColor);
    for (u32 i = 0; i < useUVs->size(); ++i) {
      CGraphics::StreamTexcoord((*useUVs)[i]);
      CGraphics::StreamVertex(xe0_coords[i] + xd0_uvBias0);
    }
    CGraphics::StreamEnd();
  }
}

void CAuiImagePane::Draw(const CGuiWidgetDrawParms& params) {
  CGraphics::SetModelMatrix(x34_worldXF);
  if (!GetIsVisible() || !xb8_tex0Tok.IsLoaded()) {
    return;
  }
  SCOPED_GRAPHICS_DEBUG_GROUP(fmt::format(FMT_STRING("CAuiImagePane::Draw {}"), m_name).c_str(), zeus::skCyan);
  GetIsFinishedLoadingWidgetSpecific();
  zeus::CColor color = xa8_color2;
  color.a() *= params.x0_alphaMod;
  CGraphics::SetDepthWriteMode(true, ERglEnum::LEqual,
                               xac_drawFlags == EGuiModelDrawFlags::Shadeless ||
                                   xac_drawFlags == EGuiModelDrawFlags::Opaque);
  float blur0 = 1.f;
  float blur1 = 0.f;
  const int frame0 = static_cast<int>(x144_frameTimer);
  int frame1 = 0;
  if (x140_interval < 1.f && x140_interval > 0.f) {
    zeus::CVector2f tmp = zeus::CVector2f(xb8_tex0Tok->GetWidth(), xb8_tex0Tok->GetHeight()) / x138_tileSize;
    frame1 = (frame0 + 1) % static_cast<int>(tmp.x() * tmp.y());
    if (x148_fadeDuration == 0.f)
      blur1 = 1.f;
    else
      blur1 = std::min(std::fmod(x144_frameTimer, 1.f) / x148_fadeDuration, 1.f);
    blur0 = 1.f - blur1;
  }

  // Alpha blend
  CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::InvSrcAlpha,
                          ERglLogicOp::Clear);
  DoDrawImagePane(color * zeus::CColor(0.f, 0.5f), *xb8_tex0Tok, frame0, 1.f, true);

  if (x150_flashFactor > 0.f) {
    // Additive blend
    zeus::CColor color2 = xa8_color2;
    color2.a() = x150_flashFactor;
    CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::One, ERglLogicOp::Clear);
    DoDrawImagePane(color2, *xb8_tex0Tok, frame0, blur0, false);
    if (blur1 > 0.f)
      DoDrawImagePane(color2, *xb8_tex0Tok, frame1, blur1, false);
  }

  switch (xac_drawFlags) {
  case EGuiModelDrawFlags::Shadeless:
  case EGuiModelDrawFlags::Opaque:
    // Opaque blend
    CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::InvSrcAlpha,
                            ERglLogicOp::Clear);
    DoDrawImagePane(color, *xb8_tex0Tok, frame0, blur0, false);
    if (blur1 > 0.f) {
      DoDrawImagePane(color, *xb8_tex0Tok, frame1, blur1, false);
    }
    break;
  case EGuiModelDrawFlags::Alpha:
    // Alpha blend
    CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::InvSrcAlpha,
                            ERglLogicOp::Clear);
    DoDrawImagePane(color, *xb8_tex0Tok, frame0, blur0, false);
    if (blur1 > 0.f) {
      DoDrawImagePane(color, *xb8_tex0Tok, frame1, blur1, false);
    }
    break;
  case EGuiModelDrawFlags::Additive:
    // Additive blend
    CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::One, ERglLogicOp::Clear);
    DoDrawImagePane(color, *xb8_tex0Tok, frame0, blur0, false);
    if (blur1 > 0.f) {
      DoDrawImagePane(color, *xb8_tex0Tok, frame1, blur1, false);
    }
    break;
  case EGuiModelDrawFlags::AlphaAdditiveOverdraw:
    // Alpha blend
    CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::InvSrcAlpha,
                            ERglLogicOp::Clear);
    DoDrawImagePane(color, *xb8_tex0Tok, frame0, blur0, false);
    if (blur1 > 0.f) {
      DoDrawImagePane(color, *xb8_tex0Tok, frame1, blur1, false);
    }
    // Full additive blend
    CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::One, ERglBlendFactor::One, ERglLogicOp::Clear);
    DoDrawImagePane(color, *xb8_tex0Tok, frame0, blur0, false);
    if (blur1 > 0.f) {
      DoDrawImagePane(color, *xb8_tex0Tok, frame1, blur1, false);
    }
    break;
  default:
    break;
  }
}

bool CAuiImagePane::GetIsFinishedLoadingWidgetSpecific() { return !xb8_tex0Tok || xb8_tex0Tok.IsLoaded(); }

void CAuiImagePane::SetTextureID0(CAssetId tex, CSimplePool* sp) {
  xc8_tex0 = tex;
  if (!sp)
    return;
  if (xc8_tex0.IsValid())
    xb8_tex0Tok = sp->GetObj({FOURCC('TXTR'), xc8_tex0});
  else
    xb8_tex0Tok = TLockedToken<CTexture>();
}

void CAuiImagePane::SetAnimationParms(const zeus::CVector2f& tileSize, float interval, float fadeDuration) {
  x138_tileSize = tileSize;
  x140_interval = interval;
  x144_frameTimer = 0.f;
  x148_fadeDuration = fadeDuration;
}

} // namespace metaforce
