#include "CGuiTextPane.hpp"
#include "CFontImageDef.hpp"
#include "CGuiFrame.hpp"
#include "CGuiSys.hpp"
#include "CGuiWidgetDrawParms.hpp"
#include "Graphics/CGraphics.hpp"
#include "Graphics/CGraphicsPalette.hpp"

namespace urde {

CGuiTextPane::CGuiTextPane(const CGuiWidgetParms& parms, CSimplePool* sp, const zeus::CVector2f& dim,
                           const zeus::CVector3f& vec, CAssetId fontId, const CGuiTextProperties& props,
                           const zeus::CColor& fontCol, const zeus::CColor& outlineCol, s32 extentX, s32 extentY)
: CGuiPane(parms, dim, vec)
, xd4_textSupport(fontId, props, fontCol, outlineCol, zeus::skWhite, extentX, extentY, sp, xac_drawFlags) {}

void CGuiTextPane::Update(float dt) {
  CGuiWidget::Update(dt);
  xd4_textSupport.Update(dt);
}

bool CGuiTextPane::GetIsFinishedLoadingWidgetSpecific() const {
  return xd4_textSupport.GetIsTextSupportFinishedLoading();
}

void CGuiTextPane::SetDimensions(const zeus::CVector2f& dim, bool initVBO) {
  CGuiPane::SetDimensions(dim, initVBO);
  if (initVBO)
    InitializeBuffers();
}

void CGuiTextPane::ScaleDimensions(const zeus::CVector3f& scale) {}

void CGuiTextPane::Draw(const CGuiWidgetDrawParms& parms) const {
  if (!GetIsVisible())
    return;
  SCOPED_GRAPHICS_DEBUG_GROUP(fmt::format(fmt("CGuiTextPane::Draw {}"), m_name).c_str(), zeus::skCyan);

  zeus::CVector2f dims = GetDimensions();

  if (xd4_textSupport.x34_extentX)
    dims.x() /= float(xd4_textSupport.x34_extentX);
  else
    dims.x() = 0.f;

  if (xd4_textSupport.x38_extentY)
    dims.y() /= float(xd4_textSupport.x38_extentY);
  else
    dims.y() = 0.f;

  zeus::CTransform local = zeus::CTransform::Translate(xc0_verts.front().m_pos + xc8_scaleCenter) *
                           zeus::CTransform::Scale(dims.x(), 1.f, dims.y());
  CGraphics::SetModelMatrix(x34_worldXF * local);

  zeus::CColor geomCol = xa8_color2;
  geomCol.a() *= parms.x0_alphaMod;
  const_cast<CGuiTextPane*>(this)->xd4_textSupport.SetGeometryColor(geomCol);

#if 0
    CGraphics::SetDepthWriteMode(xb6_31_depthTest, ERglEnum::LEqual, xb7_24_depthWrite);

    switch (xac_drawFlags)
    {
    case EGuiModelDrawFlags::Shadeless:
    case EGuiModelDrawFlags::Opaque:
        CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::One,
                                ERglBlendFactor::Zero, ERglLogicOp::Clear);
        xd4_textSupport.Render();
        break;
    case EGuiModelDrawFlags::Alpha:
        CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha,
                                ERglBlendFactor::InvSrcAlpha, ERglLogicOp::Clear);
        xd4_textSupport.Render();
        break;
    case EGuiModelDrawFlags::Additive:
        CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha,
                                ERglBlendFactor::One, ERglLogicOp::Clear);
        xd4_textSupport.Render();
        break;
    case EGuiModelDrawFlags::AlphaAdditiveOverdraw:
        CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha,
                                ERglBlendFactor::InvSrcAlpha, ERglLogicOp::Clear);
        xd4_textSupport.Render();
        const_cast<CGuiTextPane*>(this)->xd4_textSupport.SetGeometryColor
            (geomCol * zeus::CColor(geomCol.a, geomCol.a, geomCol.a, 1.f));
        CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::One,
                                ERglBlendFactor::One, ERglLogicOp::Clear);
        xd4_textSupport.Render();
        break;
    }
#else
  xd4_textSupport.Render();
#endif
}

static const zeus::CVector3f NormalPoints[] = {
  {0.f, 0.f, -1.f},
  {1.f, 0.f, -1.f},
  {1.f, 0.f, 0.f},
  {0.f, 0.f, 0.f}
};

static bool testProjectedLine(const zeus::CVector2f& a, const zeus::CVector2f& b, const zeus::CVector2f& point) {
  zeus::CVector2f normal = (b - a).perpendicularVector().normalized();
  return point.dot(normal) >= a.dot(normal);
}

bool CGuiTextPane::TestCursorHit(const zeus::CMatrix4f& vp, const zeus::CVector2f& point) const {
  zeus::CVector2f dims = GetDimensions();
  zeus::CTransform local = zeus::CTransform::Translate(xc0_verts.front().m_pos + xc8_scaleCenter) *
                           zeus::CTransform::Scale(dims.x(), 1.f, dims.y());
  zeus::CMatrix4f mvp = vp * (x34_worldXF * local).toMatrix4f();

  zeus::CVector2f projPoints[4];
  for (int i = 0; i < 4; ++i)
    projPoints[i] = mvp.multiplyOneOverW(NormalPoints[i]).toVec2f();

  int j;
  for (j = 0; j < 3; ++j) {
    if (!testProjectedLine(projPoints[j], projPoints[j + 1], point))
      break;
  }
  return j == 3 && testProjectedLine(projPoints[3], projPoints[0], point);
}

std::shared_ptr<CGuiWidget> CGuiTextPane::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp) {
  CGuiWidgetParms parms = ReadWidgetHeader(frame, in);
  zeus::CVector2f dim = zeus::CVector2f::ReadBig(in);
  zeus::CVector3f vec = zeus::CVector3f::ReadBig(in);
  u32 fontId = in.readUint32Big();
  bool wordWrap = in.readBool();
  bool horizontal = in.readBool();
  EJustification justification = EJustification(in.readUint32Big());
  EVerticalJustification vJustification = EVerticalJustification(in.readUint32Big());
  CGuiTextProperties props(wordWrap, horizontal, justification, vJustification);
  zeus::CColor fontCol;
  fontCol.readRGBABig(in);
  zeus::CColor outlineCol;
  outlineCol.readRGBABig(in);
  int extentX = in.readFloatBig();
  int extentY = in.readFloatBig();
  std::shared_ptr<CGuiTextPane> ret =
      std::make_shared<CGuiTextPane>(parms, sp, dim, vec, fontId, props, fontCol, outlineCol, extentX, extentY);
  ret->ParseBaseInfo(frame, in, parms);
  ret->InitializeBuffers();
  ret->TextSupport().SetText(u"");
  return ret;
}

} // namespace urde
