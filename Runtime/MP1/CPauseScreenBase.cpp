#include "CPauseScreenBase.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiTableGroup.hpp"
#include "GuiSys/CGuiSliderGroup.hpp"
#include "GuiSys/CGuiModel.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "GuiSys/CAuiImagePane.hpp"
#include "GameGlobalObjects.hpp"

namespace urde
{
namespace MP1
{

CPauseScreenBase::CPauseScreenBase(const CStateManager& mgr, const CGuiFrame& frame, const CStringTable& pauseStrg)
: x4_mgr(mgr), x8_frame(frame), xc_pauseStrg(pauseStrg)
{
    InitializeFrameGlue();
}

void CPauseScreenBase::InitializeFrameGlue()
{
    x60_basewidget_pivot = x8_frame.FindWidget("basewidget_pivot");
    x64_basewidget_bgframe = x8_frame.FindWidget("basewidget_bgframe");
    x68_basewidget_leftside = x8_frame.FindWidget("basewidget_leftside");
    x6c_basewidget_leftlog = x8_frame.FindWidget("basewidget_leftlog");
    x70_tablegroup_leftlog = static_cast<CGuiTableGroup*>(x8_frame.FindWidget("tablegroup_leftlog"));
    x74_basewidget_leftguages = x8_frame.FindWidget("basewidget_leftguages");
    x78_model_lefthighlight = static_cast<CGuiModel*>(x8_frame.FindWidget("model_lefthighlight"));
    x7c_basewidget_rightside = x8_frame.FindWidget("basewidget_rightside");
    x80_basewidget_rightlog = x8_frame.FindWidget("basewidget_rightlog");
    x84_tablegroup_rightlog = static_cast<CGuiTableGroup*>(x8_frame.FindWidget("tablegroup_rightlog"));
    x88_basewidget_rightguages = x8_frame.FindWidget("basewidget_rightguages");
    x8c_model_righthighlight = static_cast<CGuiModel*>(x8_frame.FindWidget("model_righthighlight"));
    x90_model_textarrowtop = static_cast<CGuiModel*>(x8_frame.FindWidget("model_textarrowtop"));
    x94_model_textarrowbottom = static_cast<CGuiModel*>(x8_frame.FindWidget("model_textarrowbottom"));
    x98_model_scrollleftup = static_cast<CGuiModel*>(x8_frame.FindWidget("model_scrollleftup"));
    x9c_model_scrollleftdown = static_cast<CGuiModel*>(x8_frame.FindWidget("model_scrollleftdown"));
    xa0_model_scrollrightup = static_cast<CGuiModel*>(x8_frame.FindWidget("model_scrollrightup"));
    xa4_model_scrollrightdown = static_cast<CGuiModel*>(x8_frame.FindWidget("model_scrollrightdown"));
    x94_model_textarrowbottom = static_cast<CGuiModel*>(x8_frame.FindWidget("model_textarrowbottom"));
    x178_textpane_title = static_cast<CGuiTextPane*>(x8_frame.FindWidget("textpane_title"));
    x174_textpane_body = static_cast<CGuiTextPane*>(x8_frame.FindWidget("textpane_body"));
    x180_basewidget_yicon = x8_frame.FindWidget("basewidget_yicon");
    x17c_model_textalpha = static_cast<CGuiModel*>(x8_frame.FindWidget("model_textalpha"));
    x184_textpane_yicon = static_cast<CGuiTextPane*>(x8_frame.FindWidget("textpane_yicon"));
    x188_textpane_ytext = static_cast<CGuiTextPane*>(x8_frame.FindWidget("textpane_ytext"));
    x18c_slidergroup_slider = static_cast<CGuiSliderGroup*>(x8_frame.FindWidget("slidergroup_slider"));
    x190_tablegroup_double = static_cast<CGuiTableGroup*>(x8_frame.FindWidget("tablegroup_double"));
    x194_tablegroup_triple = static_cast<CGuiTableGroup*>(x8_frame.FindWidget("tablegroup_triple"));

    for (int i=0 ; i<5 ; ++i)
    {
        xd8_textpane_titles.push_back(static_cast<CGuiTextPane*>(x8_frame.FindWidget(hecl::Format("xd8_textpane_title%d", i + 1))));
        x144_model_titles.push_back(static_cast<CGuiModel*>(x8_frame.FindWidget(hecl::Format("x144_model_title%d", i + 1))));
        x15c_model_righttitledecos.push_back(static_cast<CGuiModel*>(x8_frame.FindWidget(hecl::Format("x15c_model_righttitledeco%d", i + 1))));
        xa8_textpane_categories.push_back(static_cast<CGuiTextPane*>(x8_frame.FindWidget(hecl::Format("xa8_textpane_category%d", i))));
        xc0_model_categories.push_back(static_cast<CGuiModel*>(x8_frame.FindWidget(hecl::Format("xc0_model_category%d", i))));
    }

    for (int i=0 ; i<20 ; ++i)
        xf0_imagePanes.push_back(static_cast<CAuiImagePane*>(x8_frame.FindWidget(GetImagePaneName(i))));

    x70_tablegroup_leftlog->SetUserSelection(0);
    x84_tablegroup_rightlog->SetUserSelection(1);

    x74_basewidget_leftguages->SetVisibility(false, ETraversalMode::Children);
    x88_basewidget_rightguages->SetVisibility(false, ETraversalMode::Children);
    x6c_basewidget_leftlog->SetColor(g_tweakGuiColors->GetPauseItemAmberColor());
}

static const char* PaneSuffixes[] =
{
    "0",
    "1",
    "2",
    "3",
    "01",
    "12",
    "23",
    "012",
    "123",
    "0123",
    "4",
    "5",
    "6",
    "7",
    "45",
    "56",
    "67",
    "456",
    "567",
    "4567"
};

std::string CPauseScreenBase::GetImagePaneName(u32 i)
{
    return hecl::Format("imagepane_pane%s", PaneSuffixes[i]);
}

}
}
