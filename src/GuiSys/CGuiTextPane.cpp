#include "GuiSys/CGuiTextPane.hpp"

#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"

#if VERSION >= VERSION_GM8P_00
bool CGuiTextPane::sDrawPaneRects = false;
#endif

#if VERSION >= VERSION_GM8P_00
CGuiWidget* CGuiTextPane::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp, uint version) {
#else
CGuiWidget* CGuiTextPane::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp) {
#endif
  const CGuiWidgetParms parms = ReadWidgetHeader(frame, in);
  float dimX = in.Get< float >();
  float dimY = in.Get< float >();
  CVector3f vec(in);
  const CAssetId fontId = in.ReadLong();
  bool wordWrap = in.Get< bool >();
  bool horizontal = in.Get< bool >();
#if VERSION >= VERSION_GM8P_00
  const EJustification justification = static_cast< EJustification >(in.ReadLong());
  const uint verticalJustification = in.ReadLong();
  const EVerticalJustification vJustification =
      static_cast< EVerticalJustification >(verticalJustification);
#else
  EJustification justification = static_cast< EJustification >(in.Get< int >());
  EVerticalJustification vJustification = static_cast< EVerticalJustification >(in.Get< int >());
#endif
  CColor fontCol(in);
  CColor outlineColor(in);
  const int extentX = CCast::ToInt32(in.Get< float >());
  const int extentY = CCast::ToInt32(in.Get< float >());
#if VERSION >= VERSION_GM8P_00
  CAssetId jpFontId = version >= 1 ? in.Get< CAssetId >() : fontId;
  int jpExtentX = version >= 1 ? in.Get< int >() : extentX;
  int jpExtentY = version >= 1 ? in.Get< int >() : extentY;
#endif
  const CGuiTextProperties props(wordWrap, horizontal, justification, vJustification, nullptr);
#if VERSION >= VERSION_GM8P_00
  CGuiTextPane* ret = rs_new CGuiTextPane(parms, sp, dimX, dimY, vec, fontId, props, fontCol,
                                        outlineColor, extentX, extentY, jpFontId, jpExtentX, jpExtentY);
#else
  CGuiTextPane* ret = rs_new CGuiTextPane(parms, sp, dimX, dimY, vec, fontId, props, fontCol,
                                        outlineColor, extentX, extentY);
#endif
  ret->ParseBaseInfo(frame, in, parms);
  ret->InitializeBuffers();
  ret->TextSupport().SetText(rstl::string_l(""));
  return ret;
}

#if VERSION >= VERSION_GM8P_00
CGuiTextPane::CGuiTextPane(const CGuiWidgetParms& parms, CSimplePool* sp, const float dimX,
                         const float dimY, const CVector3f& vec, const CAssetId fontId,
                         const CGuiTextProperties& props, const CColor& col1, const CColor& col2,
                         const int padX, const int padY, CAssetId jpFontId, int jpExtentX, int jpExtentY)
: CGuiPane(parms, dimX, dimY, vec)
, mTextSupport(fontId, padX, padY, props, col1, col2, CColor::White(), sp)
, mDrawShadow(false) {}
#else
CGuiTextPane::CGuiTextPane(const CGuiWidgetParms& parms, CSimplePool* sp, const float dimX,
                         const float dimY, const CVector3f& vec, const CAssetId fontId,
                         const CGuiTextProperties& props, const CColor& col1, const CColor& col2,
                         const int padX, const int padY)
: CGuiPane(parms, dimX, dimY, vec)
, mTextSupport(fontId, props, col1, col2, CColor::White(), padX, padY, sp) {}
#endif

CGuiTextPane::~CGuiTextPane() {}

void CGuiTextPane::Draw(const CGuiWidgetDrawParms& parms) const {
#if VERSION >= VERSION_GM8P_00
  if (sDrawPaneRects) {
    CGuiPane::Draw(CGuiWidgetDrawParms(0.2f * parms.GetAlpha(), parms.GetCameraOffset()));
  }
#endif
  if (!GetIsVisible()) {
    return;
  }

  const float* vtxBuf = GetVtxBuf();
  CVector2f dimensions = GetDimensions();
  float width = GetTextSupport().GetTextBoundingWidth() == 0
                    ? 0.f
                    : dimensions.GetX() / GetTextSupport().GetTextBoundingWidth();
  float height = GetTextSupport().GetTextBoundingHeight() == 0
                     ? 0.f
                     : dimensions.GetY() / GetTextSupport().GetTextBoundingHeight();
  CTransform4f local =
      CTransform4f::Translate(CVector3f(vtxBuf[0], vtxBuf[1], vtxBuf[2]) + GetPivot()) *
      CTransform4f::Scale(width, 1.f, height);
  CTransform4f model = GetWorldTransform() * local;
#if VERSION >= VERSION_GM8P_00
  CColor geomCol = GetModifiedColor().WithAlphaModulatedBy(parms.GetAlpha());
  CGraphics::SetDepthWriteMode(GetIsAlwaysDepthRead(), kE_LEqual, GetIsAlwaysDepthWrite());

  if (mDrawShadow) {
    CGraphics::SetModelMatrix(model * CTransform4f::Translate(2.f, 0.f, -2.f));
    mTextSupport.SetGeometryColor(
        CColor(static_cast< uchar >(0), static_cast< uchar >(0), static_cast< uchar >(0))
            .WithAlphaOf(0.99f * geomCol.GetAlpha()));
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
    mTextSupport.Render();
  }

  CGraphics::SetModelMatrix(model);
  CGuiTextSupport& text = mTextSupport;
  text.SetGeometryColor(geomCol);
#else
  CGraphics::SetModelMatrix(model);

  CColor geomCol = GetModifiedColor().WithAlphaModulatedBy(parms.GetAlpha());
  CGuiTextSupport& text = mTextSupport;
  text.SetGeometryColor(geomCol);
  CGraphics::SetDepthWriteMode(GetIsAlwaysDepthRead(), kE_LEqual, GetIsAlwaysDepthWrite());
#endif

  switch (GetDrawFlags()) {
  case kGMDF_Shadeless:
  case kGMDF_Opaque:
    CGraphics::SetBlendMode(kBM_Blend, kBF_One, kBF_Zero, kLO_Clear);
    text.Render();
    break;
  case kGMDF_Alpha:
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
    text.Render();
    break;
  case kGMDF_Additive:
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
    text.Render();
    break;
  case kGMDF_AlphaAdditiveOverdraw: {
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
    text.Render();
    uchar alpha = geomCol.GetAlphau8();
    const CColor alphaColor(alpha, alpha, alpha, static_cast< uchar >(255));
    const CColor additiveColor = CColor::Modulate(geomCol, alphaColor);
    text.SetGeometryColor(additiveColor);
    CGraphics::SetBlendMode(kBM_Blend, kBF_One, kBF_One, kLO_Clear);
    text.Render();
    break;
  }
  }
}

void CGuiTextPane::ScaleDimensions(const CVector3f&) {}

void CGuiTextPane::SetDimensions(const CVector2f& dim, bool initVBO) {
  CGuiPane::SetDimensions(dim, initVBO);
  if (initVBO) {
    InitializeBuffers();
  }
}

rstl::vector< CAssetId > CGuiTextPane::GetFontAssets() const {
  return rstl::vector< CAssetId >(1, GetTextSupport().GetFontID(), rstl::rmemory_allocator());
}

bool CGuiTextPane::GetIsFinishedLoadingWidgetSpecific() const {
  return GetTextSupport().GetIsTextSupportFinishedLoading();
}

void CGuiTextPane::Update(float dt) {
  CGuiWidget::Update(dt);
  TextSupport().Update(dt);
}
