#include "Runtime/GuiSys/CGuiHeadWidget.hpp"
#include "Runtime/GuiSys/CGuiFrame.hpp"

namespace urde {

CGuiHeadWidget::CGuiHeadWidget(const CGuiWidgetParms& parms) : CGuiWidget(parms) {}

std::shared_ptr<CGuiWidget> CGuiHeadWidget::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp) {
  CGuiWidgetParms parms = ReadWidgetHeader(frame, in);
  std::shared_ptr<CGuiHeadWidget> ret = std::make_shared<CGuiHeadWidget>(parms);
  frame->SetHeadWidget(ret->shared_from_this());
  ret->ParseBaseInfo(frame, in, parms);
  return ret;
}

} // namespace urde
