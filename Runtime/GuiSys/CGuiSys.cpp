#include "CGuiSys.hpp"
#include "CGuiWidget.hpp"
#include "CGuiHeadWidget.hpp"
#include "CGuiLight.hpp"
#include "CGuiCamera.hpp"
#include "CGuiGroup.hpp"
#include "CGuiPane.hpp"
#include "CAuiImagePane.hpp"
#include "CAuiMeter.hpp"
#include "CGuiModel.hpp"
#include "CGuiTableGroup.hpp"
#include "CGuiSliderGroup.hpp"
#include "CGuiTextPane.hpp"
#include "CAuiEnergyBarT01.hpp"
#include "CTextParser.hpp"
#include "CSimplePool.hpp"
#include "CTextExecuteBuffer.hpp"
#include "CGuiFrame.hpp"

namespace urde {

CGuiSys* g_GuiSys = nullptr;
CTextExecuteBuffer* g_TextExecuteBuf = nullptr;
CTextParser* g_TextParser = nullptr;

std::shared_ptr<CGuiWidget> CGuiSys::CreateWidgetInGame(FourCC type, CInputStream& in, CGuiFrame* frame,
                                                        CSimplePool* sp) {
  switch (type) {
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
    return CGuiTextPane::Create(frame, in, sp);
  case SBIG('ENRG'):
    return CAuiEnergyBarT01::Create(frame, in, sp);
  default:
    break;
  }
  return {};
}

CGuiSys::CGuiSys(IFactory& resFactory, CSimplePool& resStore, EUsageMode mode)
: x0_resFactory(resFactory)
, x4_resStore(resStore)
, x8_mode(mode)
, xc_textExecuteBuf(new CTextExecuteBuffer())
, x10_textParser(new CTextParser(resStore)) {
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
    if (g_Viewport.aspect >= frame->m_aspectConstraint) {
      hPad = frame->m_aspectConstraint / g_Viewport.aspect;
      vPad = frame->m_aspectConstraint / 1.38f;
    } else {
      hPad = 1.f;
      vPad = g_Viewport.aspect / 1.38f;
    }
    frame->m_aspectTransform = zeus::CTransform::Scale({hPad, 1.f, vPad});
  } else if (frame->m_maxAspect > 0.f) {
    if (g_Viewport.aspect > frame->m_maxAspect)
      frame->m_aspectTransform = zeus::CTransform::Scale({frame->m_maxAspect / g_Viewport.aspect, 1.f, 1.f});
    else
      frame->m_aspectTransform = zeus::CTransform();
  }
}

} // namespace urde
