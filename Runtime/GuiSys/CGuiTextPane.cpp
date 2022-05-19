#include "Runtime/GuiSys/CGuiTextPane.hpp"

#include <array>

#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/Graphics/CGraphicsPalette.hpp"
#include "Runtime/GuiSys/CFontImageDef.hpp"
#include "Runtime/GuiSys/CGuiFrame.hpp"
#include "Runtime/GuiSys/CGuiSys.hpp"
#include "Runtime/GuiSys/CGuiWidgetDrawParms.hpp"

namespace metaforce {
namespace {
constexpr std::array<zeus::CVector3f, 4> NormalPoints{{
    {0.f, 0.f, -1.f},
    {1.f, 0.f, -1.f},
    {1.f, 0.f, 0.f},
    {0.f, 0.f, 0.f},
}};
bool testProjectedLine(const zeus::CVector2f& a, const zeus::CVector2f& b, const zeus::CVector2f& point) {
  const zeus::CVector2f normal = (b - a).perpendicularVector().normalized();
  return point.dot(normal) >= a.dot(normal);
}
} // Anonymous namespace

bool CGuiTextPane::sDrawPaneRects = false;
CGuiTextPane::CGuiTextPane(const CGuiWidgetParms& parms, CSimplePool* sp, const zeus::CVector2f& dim,
                           const zeus::CVector3f& vec, CAssetId fontId, const CGuiTextProperties& props,
                           const zeus::CColor& fontCol, const zeus::CColor& outlineCol, s32 extentX, s32 extentY,
                           CAssetId jpFontId, s32 jpExtentX, s32 jpExtentY)
: CGuiPane(parms, dim, vec)
, xd4_textSupport(fontId, props, fontCol, outlineCol, zeus::skWhite, extentX, extentY, sp, xac_drawFlags) {}

void CGuiTextPane::Update(float dt) {
  CGuiWidget::Update(dt);
  xd4_textSupport.Update(dt);
}

bool CGuiTextPane::GetIsFinishedLoadingWidgetSpecific() { return xd4_textSupport.GetIsTextSupportFinishedLoading(); }

void CGuiTextPane::SetDimensions(const zeus::CVector2f& dim, bool initVBO) {
  CGuiPane::SetDimensions(dim, initVBO);
  if (initVBO)
    InitializeBuffers();
}

void CGuiTextPane::ScaleDimensions(const zeus::CVector3f& scale) {}

void CGuiTextPane::Draw(const CGuiWidgetDrawParms& parms) {
  if (sDrawPaneRects) {
    CGuiPane::Draw({0.2f * parms.x0_alphaMod, parms.x4_cameraOffset});
  }

  if (!GetIsVisible()) {
    return;
  }
  SCOPED_GRAPHICS_DEBUG_GROUP(fmt::format(FMT_STRING("CGuiTextPane::Draw {}"), m_name).c_str(), zeus::skCyan);

  zeus::CVector2f dims = GetDimensions();

  if (xd4_textSupport.x34_extentX != 0) {
    dims.x() /= float(xd4_textSupport.x34_extentX);
  } else {
    dims.x() = 0.f;
  }

  if (xd4_textSupport.x38_extentY != 0) {
    dims.y() /= float(xd4_textSupport.x38_extentY);
  } else {
    dims.y() = 0.f;
  }

  const zeus::CTransform local = zeus::CTransform::Translate(xc0_verts.front() + xc8_scaleCenter) *
                                 zeus::CTransform::Scale(dims.x(), 1.f, dims.y());
  CGraphics::SetModelMatrix(x34_worldXF * local);

  zeus::CColor geomCol = xa8_color2;
  geomCol.a() *= parms.x0_alphaMod;
  xd4_textSupport.SetGeometryColor(geomCol);

  CGraphics::SetDepthWriteMode(xb6_31_depthTest, ERglEnum::LEqual, xb7_24_depthWrite);

  switch (xac_drawFlags) {
  case EGuiModelDrawFlags::Shadeless:
  case EGuiModelDrawFlags::Opaque:
    CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::One, ERglBlendFactor::Zero, ERglLogicOp::Clear);
    xd4_textSupport.Render();
    break;
  case EGuiModelDrawFlags::Alpha:
    CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::InvSrcAlpha,
                            ERglLogicOp::Clear);
    xd4_textSupport.Render();
    break;
  case EGuiModelDrawFlags::Additive:
    CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::One, ERglLogicOp::Clear);
    xd4_textSupport.Render();
    break;
  case EGuiModelDrawFlags::AlphaAdditiveOverdraw:
    CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::InvSrcAlpha,
                            ERglLogicOp::Clear);
    xd4_textSupport.Render();
    xd4_textSupport.SetGeometryColor(geomCol * zeus::CColor(geomCol.a(), 1.f));
    CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::One, ERglBlendFactor::One, ERglLogicOp::Clear);
    xd4_textSupport.Render();
    break;
  }
}

bool CGuiTextPane::TestCursorHit(const zeus::CMatrix4f& vp, const zeus::CVector2f& point) const {
  const zeus::CVector2f dims = GetDimensions();
  const zeus::CTransform local = zeus::CTransform::Translate(xc0_verts.front() + xc8_scaleCenter) *
                                 zeus::CTransform::Scale(dims.x(), 1.f, dims.y());
  const zeus::CMatrix4f mvp = vp * (x34_worldXF * local).toMatrix4f();

  std::array<zeus::CVector2f, 4> projPoints;
  for (size_t i = 0; i < projPoints.size(); ++i) {
    projPoints[i] = mvp.multiplyOneOverW(NormalPoints[i]).toVec2f();
  }

  size_t j;
  for (j = 0; j < 3; ++j) {
    if (!testProjectedLine(projPoints[j], projPoints[j + 1], point)) {
      break;
    }
  }
  return j == 3 && testProjectedLine(projPoints[3], projPoints[0], point);
}

std::shared_ptr<CGuiWidget> CGuiTextPane::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp, u32 version) {
  const CGuiWidgetParms parms = ReadWidgetHeader(frame, in);
  const zeus::CVector2f dim = in.Get<zeus::CVector2f>();
  const zeus::CVector3f vec = in.Get<zeus::CVector3f>();
  const CAssetId fontId = in.Get<CAssetId>();
  const bool wordWrap = in.ReadBool();
  const bool horizontal = in.ReadBool();
  const auto justification = EJustification(in.ReadLong());
  const auto vJustification = EVerticalJustification(in.ReadLong());
  const CGuiTextProperties props(wordWrap, horizontal, justification, vJustification);
  const zeus::CColor fontCol = in.Get<zeus::CColor>();
  const zeus::CColor outlineCol = in.Get<zeus::CColor>();
  const int extentX = static_cast<int>(in.ReadFloat());
  const int extentY = static_cast<int>(in.ReadFloat());
  int jpExtentX = extentX;
  int jpExtentY = extentY;
  CAssetId jpFontId = fontId;
  if (version != 0) {
    jpFontId = in.Get<CAssetId>();
    jpExtentX = in.ReadLong();
    jpExtentY = in.ReadLong();
  }
  auto ret = std::make_shared<CGuiTextPane>(parms, sp, dim, vec, fontId, props, fontCol, outlineCol, extentX, extentY,
                                            jpFontId, jpExtentY, jpExtentY);
  ret->ParseBaseInfo(frame, in, parms);
  ret->InitializeBuffers();
  ret->TextSupport().SetText(u"");
  return ret;
}

} // namespace metaforce
