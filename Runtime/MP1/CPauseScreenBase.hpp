#ifndef __URDE_CPAUSESCREENBASE_HPP__
#define __URDE_CPAUSESCREENBASE_HPP__

#include "RetroTypes.hpp"
#include "zeus/CVector3f.hpp"
#include "GuiSys/CGuiFrame.hpp"

namespace urde
{
class CStateManager;
class CGuiWidget;
class CGuiTableGroup;
class CGuiModel;
class CGuiTextPane;
class CGuiSliderGroup;
class CAuiImagePane;
class CStringTable;
class CRandom16;
class CArchitectureQueue;

namespace MP1
{

class CPauseScreenBase
{
public:
    enum class EMode
    {
        Invalid = -1,
        LeftTable = 0,
        RightTable = 1,
        TextScroll = 2
    };
protected:
    const CStateManager& x4_mgr;
    CGuiFrame& x8_frame;
    const CStringTable& xc_pauseStrg;
    EMode x10_mode = EMode::Invalid;
    float x14_alpha = 0.f;
    int x18_firstViewRightSel = 0;
    int x1c_rightSel = 0;
    zeus::CVector3f x20_;
    zeus::CVector3f x2c_rightTableStart;
    float x38_hightlightPitch = 0.f;
    zeus::CVector3f x3c_sliderStart;
    zeus::CVector3f x48_tableDoubleStart;
    zeus::CVector3f x54_tableTripleStart;
    CGuiWidget* x60_basewidget_pivot;
    CGuiWidget* x64_basewidget_bgframe;
    CGuiWidget* x68_basewidget_leftside = nullptr;
    CGuiWidget* x6c_basewidget_leftlog = nullptr;
    CGuiTableGroup* x70_tablegroup_leftlog = nullptr;
    CGuiWidget* x74_basewidget_leftguages = nullptr;
    CGuiModel* x78_model_lefthighlight = nullptr;
    CGuiWidget* x7c_basewidget_rightside = nullptr;
    CGuiWidget* x80_basewidget_rightlog = nullptr;
    CGuiTableGroup* x84_tablegroup_rightlog = nullptr;
    CGuiWidget* x88_basewidget_rightguages = nullptr;
    CGuiModel* x8c_model_righthighlight = nullptr;
    CGuiModel* x90_model_textarrowtop;
    CGuiModel* x94_model_textarrowbottom;
    CGuiModel* x98_model_scrollleftup;
    CGuiModel* x9c_model_scrollleftdown;
    CGuiModel* xa0_model_scrollrightup;
    CGuiModel* xa4_model_scrollrightdown;
    rstl::reserved_vector<CGuiTextPane*, 5> xa8_textpane_categories;
    rstl::reserved_vector<CGuiModel*, 5> xc0_model_categories;
    rstl::reserved_vector<CGuiTextPane*, 5> xd8_textpane_titles;
    rstl::reserved_vector<CAuiImagePane*, 20> xf0_imagePanes;
    rstl::reserved_vector<CGuiModel*, 5> x144_model_titles;
    rstl::reserved_vector<CGuiModel*, 5> x15c_model_righttitledecos;
    CGuiTextPane* x174_textpane_body = nullptr;
    CGuiTextPane* x178_textpane_title = nullptr;
    CGuiModel* x17c_model_textalpha;
    CGuiWidget* x180_basewidget_yicon = nullptr;
    CGuiTextPane* x184_textpane_yicon = nullptr;
    CGuiTextPane* x188_textpane_ytext = nullptr;
    CGuiSliderGroup* x18c_slidergroup_slider = nullptr;
    CGuiTableGroup* x190_tablegroup_double = nullptr;
    CGuiTableGroup* x194_tablegroup_triple = nullptr;
    union
    {
        struct
        {
            bool x198_24_ready : 1;
            bool x198_25_handledInput : 1;
            bool x198_26_exitPauseScreen : 1;
            bool x198_27_canDraw : 1;
            bool x198_28_pulseTextArrowTop : 1;
            bool x198_29_pulseTextArrowBottom : 1;
        };
        u32 _dummy = 0;
    };
    void InitializeFrameGlue();
    static std::string GetImagePaneName(u32 i);
    void ChangeMode(EMode mode);
    void UpdateSideTable(CGuiTableGroup* table);
    void SetRightTableSelection(int selBegin, int selEnd);

    void OnLeftTableAdvance(CGuiTableGroup* caller);
    void OnRightTableAdvance(CGuiTableGroup* caller);
    void OnTableSelectionChange(CGuiTableGroup* caller, int sel);
    void OnRightTableCancel(CGuiTableGroup* caller);

public:
    CPauseScreenBase(const CStateManager& mgr, CGuiFrame& frame, const CStringTable& pauseStrg);

    bool ShouldExitPauseScreen() const { return x198_26_exitPauseScreen; }
    bool IsReady();
    bool CanDraw() const { return x198_27_canDraw; }
    EMode GetMode() const { return x10_mode; }
    float GetAlpha() const { return x14_alpha; }

    virtual ~CPauseScreenBase() = default;
    virtual bool InputDisabled() const { return false; }
    virtual void TransitioningAway() {}
    virtual void Update(float dt, CRandom16& rand, CArchitectureQueue& archQueue);
    virtual void Touch() {}
    virtual void ProcessControllerInput(const CFinalInput& input);
    virtual void Draw(float transInterp, float totalAlpha, float yOff);
    virtual float GetCameraYBias() const { return 0.f; }
    virtual bool VReady() const=0;
    virtual void VActivate() const=0;
    virtual void RightTableSelectionChanged(int selBegin, int selEnd) {}
    virtual void ChangedMode() {}
    virtual void UpdateRightTable();
    virtual bool ShouldLeftTableAdvance() const { return true; }
    virtual bool ShouldRightTableAdvance() const { return true; }
    virtual u32 GetRightTableCount() const=0;
    virtual bool IsRightLogDynamic() const { return false; }
    virtual void UpdateRightLogColors(bool active, const zeus::CColor& activeColor, zeus::CColor& inactiveColor) {}
    virtual void UpdateRightLogHighlight(bool active, int idx, const zeus::CColor& activeColor, zeus::CColor& inactiveColor) {}
};

}
}

#endif // __URDE_CPAUSESCREENBASE_HPP__
