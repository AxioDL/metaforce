#include "Runtime/GuiSys/CGuiPane.hpp"
#include "Runtime/GuiSys/CGuiWidgetDrawParms.hpp"

#include "Runtime/Graphics/CGraphics.hpp"

namespace metaforce {

CGuiPane::CGuiPane(const CGuiWidgetParms& parms, const zeus::CVector2f& dim, const zeus::CVector3f& scaleCenter)
: CGuiWidget(parms), xb8_dim(dim), xc8_scaleCenter(scaleCenter) {
  CGuiPane::InitializeBuffers();
}

void CGuiPane::Draw(const CGuiWidgetDrawParms& parms) {
  CGraphics::SetModelMatrix(x34_worldXF * zeus::CTransform::Translate(xc8_scaleCenter));
  if (GetIsVisible()) {
    auto col = xa8_color2;
    col.a() = parms.x0_alphaMod * xa8_color2.a();

    CGraphics::SetTevOp(ERglTevStage::Stage0, CTevCombiners::skPassThru);
    CGraphics::DrawPrimitive(GX_TRIANGLESTRIP, xc0_verts.data(), skDefaultNormal, col, xc0_verts.size());
  }
  CGuiWidget::Draw(parms);
}
void CGuiPane::ScaleDimensions(const zeus::CVector3f& scale) {
  InitializeBuffers();
  for (auto& vert : xc0_verts) {
    vert -= xc8_scaleCenter;
    vert *= scale;
    vert += xc8_scaleCenter;
  }
}

void CGuiPane::SetDimensions(const zeus::CVector2f& dim, bool initBuffers) {
  xb8_dim = dim;
  if (initBuffers)
    InitializeBuffers();
}

zeus::CVector2f CGuiPane::GetDimensions() const { return xb8_dim; }

void CGuiPane::InitializeBuffers() {
#if 0
  if (xc0_verts == nullptr) {
    xc0_verts = new float[3 * 4];
  }
#endif
  xc0_verts[0].assign(-xb8_dim.x() * 0.5f, 0.f, xb8_dim.y() * 0.5f);
  xc0_verts[1].assign(-xb8_dim.x() * 0.5f, 0.f, -xb8_dim.y() * 0.5f);
  xc0_verts[2].assign(xb8_dim.x() * 0.5f, 0.f, xb8_dim.y() * 0.5f);
  xc0_verts[3].assign(xb8_dim.x() * 0.5f, 0.f, -xb8_dim.y() * 0.5f);
}

void CGuiPane::WriteData(COutputStream& out, bool flag) const {}

std::shared_ptr<CGuiWidget> CGuiPane::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp) {
  CGuiWidgetParms parms = ReadWidgetHeader(frame, in);
  zeus::CVector2f dim = in.Get<zeus::CVector2f>();
  zeus::CVector3f scaleCenter = in.Get<zeus::CVector3f>();
  std::shared_ptr<CGuiWidget> ret = std::make_shared<CGuiPane>(parms, dim, scaleCenter);
  ret->ParseBaseInfo(frame, in, parms);
  return ret;
}

} // namespace metaforce
