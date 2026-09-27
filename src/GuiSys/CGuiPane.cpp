#include "GuiSys/CGuiPane.hpp"
#include "GuiSys/CGuiWidget.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"

#include <Kyoto/Graphics/CGraphics.hpp>
#include <Kyoto/Streams/COutputStream.hpp>

CGuiWidget* CGuiPane::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp) {
  CGuiWidgetParms parms = ReadWidgetHeader(frame, in);
  float width = in.ReadFloat();
  float height = in.ReadFloat();
  float scaleX = in.ReadFloat();
  float scaleY = in.ReadFloat();
  float scaleZ = in.ReadFloat();
  CGuiPane* widget = rs_new CGuiPane(parms, width, height, CVector3f(scaleX, scaleY, scaleZ));
  widget->ParseBaseInfo(frame, in, parms);
  return widget;
}

void CGuiPane::WriteData(COutputStream& out, bool) const {
  out.WriteReal32(mWidth);
  out.WriteReal32(mHeight);
  out.WriteReal32(mScaleCenter.GetX());
  out.WriteReal32(mScaleCenter.GetY());
  out.WriteReal32(mScaleCenter.GetZ());
}

CGuiPane::CGuiPane(const CGuiWidgetParms& parms, const float width, const float height,
                   const CVector3f& scaleCenter)
: CGuiWidget(parms)
, mWidth(width)
, mHeight(height)
, mPanePoints(0)
, mPanePointCount(4)
, mScaleCenter(scaleCenter) {
  InitializeBuffers();
}

CGuiPane::~CGuiPane() {
  if (mPanePoints && mPanePoints) {
    delete mPanePoints;
  }
}

void CGuiPane::Draw(const CGuiWidgetDrawParms& parms) const {
  CGraphics::SetModelMatrix(GetWorldTransform() * CTransform4f::Translate(mScaleCenter));
  if (GetIsVisible()) {
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
    CColor color = GetModifiedColor().WithAlphaModulatedBy(parms.GetAlpha());
    CGraphics::DrawPrimitive(kP_TriangleStrip, mPanePoints, CVector3f(0.f, -1.f, 0.f), color, mPanePointCount);
  }
  CGuiWidget::Draw(parms);
}

void CGuiPane::ScaleDimensions(const CVector3f& scale) {
  InitializeBuffers();
  const CVector3f& center = ScaleCenter();
  for (int i = 0; i < mPanePointCount * 3; ++i) {
    int idx = i % 3;
    mPanePoints[i] = (mPanePoints[i] - center[idx]) * scale[idx]; // this might be an inline?
    mPanePoints[i] += center[i % 3];
  }
}

void CGuiPane::InitializeBuffers() {
  if (mPanePoints == nullptr) {
    mPanePoints = rs_new float[mPanePointCount * 3];
  }

  mPanePoints[0] = -GetWidth() / 2.f;
  mPanePoints[1] = 0.f;
  mPanePoints[2] = GetHeight() / 2.f;

  mPanePoints[3] = -GetWidth() / 2.f;
  mPanePoints[4] = 0.f;
  mPanePoints[5] = -GetHeight() / 2.f;

  mPanePoints[6] = GetWidth() / 2.f;
  mPanePoints[7] = 0.f;
  mPanePoints[8] = GetHeight() / 2.f;

  mPanePoints[9] = GetWidth() / 2.f;
  mPanePoints[10] = 0.f;
  mPanePoints[11] = -GetHeight() / 2.f;
}

void CGuiPane::SetDimensions(const CVector2f& dim, const bool init) {
  mWidth = dim.GetX();
  mHeight = dim.GetY();
  if (!init) {
    return;
  }
  InitializeBuffers();
}

CVector2f CGuiPane::GetDimensions() const { return CVector2f(mWidth, mHeight); }

FourCC CGuiPane::GetWidgetTypeID() const { return 'PANE'; }
