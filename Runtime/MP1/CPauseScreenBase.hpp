#pragma once

#include <string>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/GuiSys/CGuiFrame.hpp"

#include <zeus/CVector3f.hpp>

namespace urde {
class CArchitectureQueue;
class CAuiImagePane;
class CGuiModel;
class CGuiSliderGroup;
class CGuiTableGroup;
class CGuiTextPane;
class CGuiWidget;
class CRandom16;
class CStateManager;
class CStringTable;

namespace MP1 {

class CPauseScreenBase {
public:
  enum class EMode { Invalid = -1, LeftTable = 0, RightTable = 1, TextScroll = 2 };

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
  float x38_highlightPitch = 0.f;
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
  rstl::reserved_vector<CGuiModel*, 5> m_model_lefttitledecos;
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
  bool x198_24_ready : 1 = false;
  bool x198_25_handledInput : 1 = false;
  bool x198_26_exitPauseScreen : 1 = false;
  bool x198_27_canDraw : 1 = false;
  bool x198_28_pulseTextArrowTop : 1 = false;
  bool x198_29_pulseTextArrowBottom : 1 = false;
  bool m_isLogBook : 1;
  bool m_bodyUpClicked : 1 = false;
  bool m_bodyDownClicked : 1 = false;
  bool m_bodyClicked : 1 = false;
  bool m_leftClicked : 1 = false;
  bool m_rightClicked : 1 = false;
  bool m_playRightTableSfx : 1 = true;

  void InitializeFrameGlue();
  void ChangeMode(EMode mode, bool playSfx = true);
  void UpdateSideTable(CGuiTableGroup* table);
  void SetRightTableSelection(int oldSel, int newSel);

  void OnLeftTableAdvance(CGuiTableGroup* caller);
  void OnRightTableAdvance(CGuiTableGroup* caller);
  void OnTableSelectionChange(CGuiTableGroup* caller, int oldSel);
  void OnRightTableCancel(CGuiTableGroup* caller);

  void OnWidgetMouseUp(CGuiWidget* widget, bool cancel);
  void OnWidgetScroll(CGuiWidget* widget, const boo::SScrollDelta& delta, int accumX, int accumY);

public:
  static std::string GetImagePaneName(size_t i);

  CPauseScreenBase(const CStateManager& mgr, CGuiFrame& frame, const CStringTable& pauseStrg, bool isLogBook = false);

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
  bool ProcessMouseInput(const CFinalInput& input, float yOff);
  void ResetMouseState();
  virtual void Draw(float transInterp, float totalAlpha, float yOff);
  virtual float GetCameraYBias() const { return 0.f; }
  virtual bool VReady() const = 0;
  virtual void VActivate() = 0;
  virtual void RightTableSelectionChanged(int oldSel, int newSel) {}
  virtual void ChangedMode(EMode oldMode) {}
  virtual void UpdateRightTable();
  virtual bool ShouldLeftTableAdvance() const { return true; }
  virtual bool ShouldRightTableAdvance() const { return true; }
  virtual u32 GetRightTableCount() const = 0;
  virtual bool IsRightLogDynamic() const { return false; }
  virtual void UpdateRightLogColors(bool active, const zeus::CColor& activeColor, const zeus::CColor& inactiveColor) {}
  virtual void UpdateRightLogHighlight(bool active, int idx, const zeus::CColor& activeColor,
                                       const zeus::CColor& inactiveColor) {}
};

} // namespace MP1
} // namespace urde
