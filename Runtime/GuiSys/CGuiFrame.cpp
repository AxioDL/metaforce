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
    xa0_.resize(8);
    x48_rootWidget.reset(new CGuiWidget(
        CGuiWidget::CGuiWidgetParms(this, false, 0, 0, false, false, false, zeus::CColor::skWhite,
                                    CGuiWidget::EGuiModelDrawFlags::Two, false,
                                    x3c_guiSys.x2c_mode != CGuiSys::EUsageMode::Zero)));
}

}
