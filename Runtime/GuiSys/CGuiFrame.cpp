#include "CGuiFrame.hpp"
#include "CGuiWidget.hpp"
#include "CGuiSys.hpp"
#include "zeus/CColor.hpp"

namespace urde
{

CGuiFrame::CGuiFrame(u32 id, const std::string& name, CGuiSys& sys, int a, int b, int c)
: x4_name(name), x14_id(id), x1c_transitionOpts(EFrameTransitionOptions::Zero),
  x3c_guiSys(sys), xb0_a(a), xb4_b(b), xb8_c(c), xbc_24_flag1(false)
{
    xa0_lights.resize(8);
    x48_rootWidget.reset(new CGuiWidget(
        CGuiWidget::CGuiWidgetParms(this, false, 0, 0, false, false, false, zeus::CColor::skWhite,
                                    CGuiWidget::EGuiModelDrawFlags::Two, false,
                                    x3c_guiSys.x2c_mode != CGuiSys::EUsageMode::Zero)));
}

CGuiWidget* CGuiFrame::FindWidget(const std::string& name) const
{
    s16 id = x64_idDB.FindWidgetID(name);
    if (id == -1)
        return nullptr;
    return FindWidget(id);
}

CGuiWidget* CGuiFrame::FindWidget(s16 id) const
{
    return x48_rootWidget->FindWidget(id);
}

void CGuiFrame::ResetControllerStatus()
{
    x0_controllerStatus[0] = false;
    x0_controllerStatus[1] = false;
    x0_controllerStatus[2] = false;
    x0_controllerStatus[3] = false;
}


}
