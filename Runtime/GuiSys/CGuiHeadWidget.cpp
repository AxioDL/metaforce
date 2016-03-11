#include "CGuiHeadWidget.hpp"
#include "CGuiFrame.hpp"

namespace urde
{

CGuiHeadWidget::CGuiHeadWidget(const CGuiWidgetParms& parms)
: CGuiWidget(parms) {}

CGuiHeadWidget* CGuiHeadWidget::Create(CGuiFrame* frame, CInputStream& in, bool flag)
{
    CGuiWidgetParms parms = ReadWidgetHeader(frame, in, flag);
    CGuiHeadWidget* ret = new CGuiHeadWidget(parms);
    frame->SetHeadWidget(ret);
    ret->ParseBaseInfo(frame, in, parms);
    return ret;
}

}
