#include "Runtime/GuiSys/CGuiSys.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/GuiSys/CAuiEnergyBarT01.hpp"
#include "Runtime/GuiSys/CAuiImagePane.hpp"
#include "Runtime/GuiSys/CAuiMeter.hpp"
#include "Runtime/GuiSys/CGuiCamera.hpp"
#include "Runtime/GuiSys/CGuiFrame.hpp"
#include "Runtime/GuiSys/CGuiGroup.hpp"
#include "Runtime/GuiSys/CGuiHeadWidget.hpp"
#include "Runtime/GuiSys/CGuiLight.hpp"
#include "Runtime/GuiSys/CGuiModel.hpp"
#include "Runtime/GuiSys/CGuiPane.hpp"
#include "Runtime/GuiSys/CGuiSliderGroup.hpp"
#include "Runtime/GuiSys/CGuiTableGroup.hpp"
#include "Runtime/GuiSys/CGuiTextPane.hpp"
#include "Runtime/GuiSys/CGuiWidget.hpp"
#include "Runtime/GuiSys/CTextExecuteBuffer.hpp"
#include "Runtime/GuiSys/CTextParser.hpp"

namespace metaforce {

CGuiSys* g_GuiSys = nullptr;
CTextExecuteBuffer* g_TextExecuteBuf = nullptr;
CTextParser* g_TextParser = nullptr;

std::shared_ptr<CGuiWidget> CGuiSys::CreateWidgetInGame(FourCC type, CInputStream& in, CGuiFrame* frame,
                                                        CSimplePool* sp, u32 version) {
  switch (type.toUint32()) {
  case SBIG('BWIG'):
    return CGuiWidget::Create(frame, in, sp);
  case SBIG('HWIG'):
    return CGuiHeadWidget::Create(frame, in, sp);
  case SBIG('LITE'):
    return CGuiLight::Create(frame, in, sp);
  case SBIG('CAMR'):
    return CGuiCamera::Create(frame, in, sp);
  case SBIG('GRUP'):
    return CGuiGroup::Create(frame, in, sp);
  case SBIG('PANE'):
    return CGuiPane::Create(frame, in, sp);
  case SBIG('IMGP'):
    return CAuiImagePane::Create(frame, in, sp);
  case SBIG('METR'):
    return CAuiMeter::Create(frame, in, sp);
  case SBIG('MODL'):
    return CGuiModel::Create(frame, in, sp);
  case SBIG('TBGP'):
    return CGuiTableGroup::Create(frame, in, sp);
  case SBIG('SLGP'):
    return CGuiSliderGroup::Create(frame, in, sp);
  case SBIG('TXPN'):
    return CGuiTextPane::Create(frame, in, sp, version);
  case SBIG('ENRG'):
    return CAuiEnergyBarT01::Create(frame, in, sp);
  default:
    return {};
  }
}

CGuiSys::CGuiSys(IFactory& resFactory, CSimplePool& resStore, EUsageMode mode)
: x0_resFactory(resFactory)
, x4_resStore(resStore)
, x8_mode(mode)
, xc_textExecuteBuf(std::make_unique<CTextExecuteBuffer>())
, x10_textParser(std::make_unique<CTextParser>(resStore)) {
  g_TextExecuteBuf = xc_textExecuteBuf.get();
  g_TextParser = x10_textParser.get();
}

void CGuiSys::OnViewportResize() {
  for (CGuiFrame* frame : m_registeredFrames)
    ViewportResizeFrame(frame);
}

void CGuiSys::ViewportResizeFrame(CGuiFrame* frame) {
  if (frame->m_aspectConstraint > 0.f) {
    float hPad, vPad;
    if (CGraphics::GetViewportAspect() >= frame->m_aspectConstraint) {
      hPad = frame->m_aspectConstraint / CGraphics::GetViewportAspect();
      vPad = frame->m_aspectConstraint / 1.38f;
    } else {
      hPad = 1.f;
      vPad = CGraphics::GetViewportAspect() / 1.38f;
    }
    frame->m_aspectTransform = zeus::CTransform::Scale({hPad, 1.f, vPad});
  } else if (frame->m_maxAspect > 0.f) {
    if (CGraphics::GetViewportAspect() > frame->m_maxAspect)
      frame->m_aspectTransform =
          zeus::CTransform::Scale({frame->m_maxAspect / CGraphics::GetViewportAspect(), 1.f, 1.f});
    else
      frame->m_aspectTransform = zeus::CTransform();
  }
}

} // namespace metaforce
