#ifndef _CPAUSESCREENBASE
#define _CPAUSESCREENBASE

#include "Kyoto/Math/CVector3f.hpp"
#include "rstl/reserved_vector.hpp"
#include "types.h"

class CArchitectureQueue;
class CAuiImagePane;
class CColor;
class CFinalInput;
class CGuiFrame;
class CGuiModel;
class CGuiSliderGroup;
class CGuiTableGroup;
class CGuiTextPane;
class CGuiWidget;
class CRandom16;
class CStateManager;
class CStringTable;

class CPauseScreenBase {
public:
  enum EMode { kM_Invalid = -1, kM_LeftTable, kM_RightTable, kM_TextScroll };

  virtual ~CPauseScreenBase();
  virtual bool InputDisabled() const;
  virtual void TransitioningAway();
  virtual void Update(float dt, CRandom16& rand, CArchitectureQueue& queue);
  virtual void Touch();
  virtual void ProcessInput(const CFinalInput& input);
  virtual void Draw(float transInterp, float totalAlpha, float yOff) const;
  virtual float GetCameraYBias() const { return 0.f; }
  virtual bool VReady() const = 0;
  virtual void VActivate() = 0;
  virtual void RightTableSelectionChanged(int oldSel, int newSel);
  virtual void ChangedMode(EMode oldMode);
  virtual void UpdateRightTable();
  virtual bool ShouldLeftTableAdvance();
  virtual bool ShouldRightTableAdvance();
  virtual uint GetRightTableCount() const = 0;
  virtual bool IsRightLogDynamic() const { return false; }
  virtual void UpdateRightLogColors(bool active, const CColor& activeColor,
                                    const CColor& inactiveColor) {}
  virtual void UpdateRightLogHighlight(bool active, int idx, const CColor& activeColor,
                                       const CColor& inactiveColor) {}

  CPauseScreenBase(const CStateManager& mgr, CGuiFrame& frame, const CStringTable& pauseStrg);
  bool ShouldExitPauseScreen() const;
  bool IsReady();
  bool CanDraw() const { return mCanDraw; }
  EMode GetMode() const { return mMode; }
  float GetAlpha() const { return mAlpha; }

protected:
  void InitializeFrameGlue();
  void ChangeMode(EMode mode);
  void UpdateSideTable(CGuiTableGroup* table);
  void SetRightTableSelection(int oldSel, int newSel);
  void OnLeftTableAdvance(CGuiTableGroup* caller);
  void OnRightTableAdvance(CGuiTableGroup* caller);
  void OnTableSelectionChange(CGuiTableGroup* caller, int oldSel);
  void OnRightTableCancel(CGuiTableGroup* caller);

  const CStateManager& mMgr;
  CGuiFrame& mFrame;
  const CStringTable& mPauseStrg;
  EMode mMode;
  float mAlpha;
  int mFirstViewRightSel;
  int mRightSel;
  CVector3f x20_;
  CVector3f mRightTableStart;
  float mHighlightPitch;
  CVector3f mSliderStart;
  CVector3f mTableDoubleStart;
  CVector3f mTableTripleStart;
  CGuiWidget* mBasewidget_pivot;
  CGuiWidget* mBasewidget_bgframe;
  CGuiWidget* mBasewidget_leftside;
  CGuiWidget* mBasewidget_leftlog;
  CGuiTableGroup* mTablegroup_leftlog;
  CGuiWidget* mBasewidget_leftguages;
  CGuiModel* mModel_lefthighlight;
  CGuiWidget* mBasewidget_rightside;
  CGuiWidget* mBasewidget_rightlog;
  CGuiTableGroup* mTablegroup_rightlog;
  CGuiWidget* mBasewidget_rightguages;
  CGuiModel* mModel_righthighlight;
  CGuiModel* mModel_textarrowtop;
  CGuiModel* mModel_textarrowbottom;
  CGuiModel* mModel_scrollleftup;
  CGuiModel* mModel_scrollleftdown;
  CGuiModel* mModel_scrollrightup;
  CGuiModel* mModel_scrollrightdown;
  rstl::reserved_vector< CGuiTextPane*, 5 > mTextpane_categories;
  rstl::reserved_vector< CGuiModel*, 5 > mModel_categories;
  rstl::reserved_vector< CGuiTextPane*, 5 > mTextpane_titles;
  rstl::reserved_vector< CAuiImagePane*, 20 > mImagePanes;
  rstl::reserved_vector< CGuiModel*, 5 > mModel_titles;
  rstl::reserved_vector< CGuiModel*, 5 > mModel_righttitledecos;
  CGuiTextPane* mTextpane_body;
  CGuiTextPane* mTextpane_title;
  CGuiModel* mModel_textalpha;
  CGuiWidget* mBasewidget_yicon;
  CGuiTextPane* mTextpane_yicon;
  CGuiTextPane* mTextpane_ytext;
  CGuiSliderGroup* mSlidergroup_slider;
  CGuiTableGroup* mTablegroup_double;
  CGuiTableGroup* mTablegroup_triple;
  bool mReady : 1;
  bool mHandledInput : 1;
  bool mExitPauseScreen : 1;
  bool mCanDraw : 1;
  bool mPulseTextArrowTop : 1;
  bool mPulseTextArrowBottom : 1;
};
CHECK_SIZEOF(CPauseScreenBase, 0x19c)

#endif // _CPAUSESCREENBASE
