#include "CPauseScreenBase.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiTableGroup.hpp"
#include "GuiSys/CGuiSliderGroup.hpp"
#include "GuiSys/CGuiModel.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "GuiSys/CAuiImagePane.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "GameGlobalObjects.hpp"
#include "Audio/CSfxManager.hpp"
#include "CGameState.hpp"
#include "GuiSys/CStringTable.hpp"

namespace urde::MP1 {

CPauseScreenBase::CPauseScreenBase(const CStateManager& mgr, CGuiFrame& frame, const CStringTable& pauseStrg)
: x4_mgr(mgr), x8_frame(frame), xc_pauseStrg(pauseStrg) {
  InitializeFrameGlue();
}

void CPauseScreenBase::InitializeFrameGlue() {
  x60_basewidget_pivot = x8_frame.FindWidget("basewidget_pivot");
  x64_basewidget_bgframe = x8_frame.FindWidget("basewidget_bgframe");
  x68_basewidget_leftside = x8_frame.FindWidget("basewidget_leftside");
  x6c_basewidget_leftlog = x8_frame.FindWidget("basewidget_leftlog");
  x70_tablegroup_leftlog = static_cast<CGuiTableGroup*>(x8_frame.FindWidget("tablegroup_leftlog"));
  x74_basewidget_leftguages = x8_frame.FindWidget("basewidget_leftguages");
  x74_basewidget_leftguages->SetColor(zeus::CColor(1.f, 0.f));
  x78_model_lefthighlight = static_cast<CGuiModel*>(x8_frame.FindWidget("model_lefthighlight"));
  x7c_basewidget_rightside = x8_frame.FindWidget("basewidget_rightside");
  x80_basewidget_rightlog = x8_frame.FindWidget("basewidget_rightlog");
  x84_tablegroup_rightlog = static_cast<CGuiTableGroup*>(x8_frame.FindWidget("tablegroup_rightlog"));
  x88_basewidget_rightguages = x8_frame.FindWidget("basewidget_rightguages");
  x88_basewidget_rightguages->SetColor(zeus::CColor(1.f, 0.f));
  x8c_model_righthighlight = static_cast<CGuiModel*>(x8_frame.FindWidget("model_righthighlight"));
  x90_model_textarrowtop = static_cast<CGuiModel*>(x8_frame.FindWidget("model_textarrowtop"));
  x94_model_textarrowbottom = static_cast<CGuiModel*>(x8_frame.FindWidget("model_textarrowbottom"));
  x98_model_scrollleftup = static_cast<CGuiModel*>(x8_frame.FindWidget("model_scrollleftup"));
  x9c_model_scrollleftdown = static_cast<CGuiModel*>(x8_frame.FindWidget("model_scrollleftdown"));
  xa0_model_scrollrightup = static_cast<CGuiModel*>(x8_frame.FindWidget("model_scrollrightup"));
  xa4_model_scrollrightdown = static_cast<CGuiModel*>(x8_frame.FindWidget("model_scrollrightdown"));
  x94_model_textarrowbottom = static_cast<CGuiModel*>(x8_frame.FindWidget("model_textarrowbottom"));
  x178_textpane_title = static_cast<CGuiTextPane*>(x8_frame.FindWidget("textpane_title"));
  x178_textpane_title->TextSupport().SetFontColor(g_tweakGuiColors->GetPauseItemAmberColor());
  x174_textpane_body = static_cast<CGuiTextPane*>(x8_frame.FindWidget("textpane_body"));
  x174_textpane_body->SetColor(zeus::CColor(1.f, 0.f));
  x174_textpane_body->SetIsVisible(true);
  x174_textpane_body->TextSupport().SetFontColor(g_tweakGuiColors->GetPauseItemAmberColor());
  x174_textpane_body->TextSupport().SetPage(0);
  x174_textpane_body->TextSupport().SetText(u"");
  x174_textpane_body->TextSupport().SetJustification(EJustification::Left);
  x174_textpane_body->TextSupport().SetVerticalJustification(EVerticalJustification::Top);
  x174_textpane_body->TextSupport().SetControlTXTRMap(&g_GameState->GameOptions().GetControlTXTRMap());
  x180_basewidget_yicon = x8_frame.FindWidget("basewidget_yicon");
  x180_basewidget_yicon->SetVisibility(false, ETraversalMode::Children);
  x17c_model_textalpha = static_cast<CGuiModel*>(x8_frame.FindWidget("model_textalpha"));
  x184_textpane_yicon = static_cast<CGuiTextPane*>(x8_frame.FindWidget("textpane_yicon"));
  x188_textpane_ytext = static_cast<CGuiTextPane*>(x8_frame.FindWidget("textpane_ytext"));
  x184_textpane_yicon->TextSupport().SetText(
      hecl::UTF8ToChar16(hecl::Format("&image=%8.8X;", u32(g_tweakPlayerRes->xbc_yButton[0].Value()))));
  x188_textpane_ytext->TextSupport().SetText(xc_pauseStrg.GetString(99));
  x188_textpane_ytext->SetColor(g_tweakGuiColors->GetPauseItemAmberColor());
  x18c_slidergroup_slider = static_cast<CGuiSliderGroup*>(x8_frame.FindWidget("slidergroup_slider"));
  x190_tablegroup_double = static_cast<CGuiTableGroup*>(x8_frame.FindWidget("tablegroup_double"));
  x194_tablegroup_triple = static_cast<CGuiTableGroup*>(x8_frame.FindWidget("tablegroup_triple"));

  x2c_rightTableStart = x84_tablegroup_rightlog->GetWorkerWidget(0)->GetIdlePosition();
  x38_highlightPitch = x84_tablegroup_rightlog->GetWorkerWidget(1)->GetIdlePosition().z() - x2c_rightTableStart.z();
  x3c_sliderStart = x18c_slidergroup_slider->GetIdlePosition();
  x48_tableDoubleStart = x190_tablegroup_double->GetIdlePosition();
  x54_tableTripleStart = x194_tablegroup_triple->GetIdlePosition();

  for (int i = 0; i < 5; ++i)
    x70_tablegroup_leftlog->GetWorkerWidget(i)->SetIsSelectable(true);

  for (int i = 0; i < x84_tablegroup_rightlog->GetElementCount(); ++i) {
    CGuiWidget* w = x84_tablegroup_rightlog->GetWorkerWidget(i);
    w->SetLocalTransform(
        zeus::CTransform::Translate(x2c_rightTableStart + zeus::CVector3f(0.f, 0.f, x38_highlightPitch * i)));
    w->SetIsSelectable(true);
  }

  for (int i = 0; i < 5; ++i) {
    xd8_textpane_titles.push_back(
        static_cast<CGuiTextPane*>(x8_frame.FindWidget(hecl::Format("textpane_title%d", i + 1))));
    xd8_textpane_titles.back()->TextSupport().SetText(u"");
    x144_model_titles.push_back(static_cast<CGuiModel*>(x8_frame.FindWidget(hecl::Format("model_title%d", i + 1))));
    x15c_model_righttitledecos.push_back(
        static_cast<CGuiModel*>(x8_frame.FindWidget(hecl::Format("model_righttitledeco%d", i + 1))));
    x15c_model_righttitledecos.back()->SetMouseActive(true);
    xa8_textpane_categories.push_back(
        static_cast<CGuiTextPane*>(x8_frame.FindWidget(hecl::Format("textpane_category%d", i))));
    xc0_model_categories.push_back(static_cast<CGuiModel*>(x8_frame.FindWidget(hecl::Format("model_category%d", i))));
  }

  for (int i = 0; i < 20; ++i)
    xf0_imagePanes.push_back(static_cast<CAuiImagePane*>(x8_frame.FindWidget(GetImagePaneName(i))));

  x70_tablegroup_leftlog->SetUserSelection(0);
  x84_tablegroup_rightlog->SetUserSelection(1);

  x74_basewidget_leftguages->SetVisibility(false, ETraversalMode::Children);
  x88_basewidget_rightguages->SetVisibility(false, ETraversalMode::Children);
  x6c_basewidget_leftlog->SetColor(g_tweakGuiColors->GetPauseItemAmberColor());

  if (IsRightLogDynamic()) {
    zeus::CColor color = g_tweakGuiColors->GetPauseItemAmberColor();
    color.a() = 0.5f;
    UpdateRightLogColors(false, g_tweakGuiColors->GetPauseItemAmberColor(), color);
  } else {
    x80_basewidget_rightlog->SetColor(g_tweakGuiColors->GetPauseItemAmberColor());
  }

  for (CGuiObject* obj = x64_basewidget_bgframe->GetChildObject(); obj; obj = obj->GetNextSibling())
    static_cast<CGuiWidget*>(obj)->SetColor(g_tweakGuiColors->GetPauseItemAmberColor());

  zeus::CColor dimColor = g_tweakGuiColors->GetPauseItemAmberColor();
  dimColor.a() = 0.2f;
  x98_model_scrollleftup->SetColor(dimColor);
  x9c_model_scrollleftdown->SetColor(dimColor);
  xa0_model_scrollrightup->SetColor(dimColor);
  xa4_model_scrollrightdown->SetColor(dimColor);
  x90_model_textarrowtop->SetColor(dimColor);
  x94_model_textarrowbottom->SetColor(dimColor);

  x18c_slidergroup_slider->SetColor(g_tweakGuiColors->GetPauseItemAmberColor());
  x190_tablegroup_double->SetColor(g_tweakGuiColors->GetPauseItemAmberColor());
  x194_tablegroup_triple->SetColor(g_tweakGuiColors->GetPauseItemAmberColor());

  UpdateSideTable(x190_tablegroup_double);
  UpdateSideTable(x194_tablegroup_triple);
  UpdateSideTable(x70_tablegroup_leftlog);
  UpdateSideTable(x84_tablegroup_rightlog);

  x18c_slidergroup_slider->SetVisibility(false, ETraversalMode::Children);

  x190_tablegroup_double->SetIsVisible(false);
  x194_tablegroup_triple->SetIsVisible(false);
  x190_tablegroup_double->SetVertical(false);
  x194_tablegroup_triple->SetVertical(false);

  x70_tablegroup_leftlog->SetMenuAdvanceCallback(
      std::bind(&CPauseScreenBase::OnLeftTableAdvance, this, std::placeholders::_1));
  x70_tablegroup_leftlog->SetMenuSelectionChangeCallback(
      std::bind(&CPauseScreenBase::OnTableSelectionChange, this, std::placeholders::_1, std::placeholders::_2));
  x84_tablegroup_rightlog->SetMenuAdvanceCallback(
      std::bind(&CPauseScreenBase::OnRightTableAdvance, this, std::placeholders::_1));
  x84_tablegroup_rightlog->SetMenuSelectionChangeCallback(
      std::bind(&CPauseScreenBase::OnTableSelectionChange, this, std::placeholders::_1, std::placeholders::_2));
  x84_tablegroup_rightlog->SetMenuCancelCallback(
      std::bind(&CPauseScreenBase::OnRightTableCancel, this, std::placeholders::_1));
  x18c_slidergroup_slider->SetSelectionChangedCallback({});
  x190_tablegroup_double->SetMenuSelectionChangeCallback({});
  x194_tablegroup_triple->SetMenuSelectionChangeCallback({});
}

bool CPauseScreenBase::IsReady() {
  if (x198_24_ready)
    return true;
  x198_24_ready = VReady();
  if (x198_24_ready) {
    VActivate();
    ChangeMode(EMode::LeftTable);
    UpdateSideTable(x70_tablegroup_leftlog);
    UpdateRightTable();
    return true;
  }
  return false;
}

void CPauseScreenBase::ChangeMode(EMode mode) {
  if (x10_mode == mode)
    return;

  EMode oldMode = x10_mode;
  zeus::CColor color = g_tweakGuiColors->GetPauseItemAmberColor();
  zeus::CColor colorDim = color;
  colorDim.a() = 0.5f;

  switch (x10_mode) {
  case EMode::LeftTable:
    x6c_basewidget_leftlog->SetColor(colorDim);
    x70_tablegroup_leftlog->SetIsActive(false);
    break;
  case EMode::Invalid:
  case EMode::RightTable:
    if (IsRightLogDynamic())
      UpdateRightLogColors(false, color, colorDim);
    else
      x80_basewidget_rightlog->SetColor(colorDim);
    x84_tablegroup_rightlog->SetIsActive(false);
    break;
  case EMode::TextScroll:
    CSfxManager::SfxStart(SFXui_table_change_mode, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
    break;
  default:
    break;
  }

  x10_mode = mode;

  switch (x10_mode) {
  case EMode::LeftTable:
    if (oldMode == EMode::RightTable)
      CSfxManager::SfxStart(SFXui_table_change_mode, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
    x6c_basewidget_leftlog->SetColor(color);
    x70_tablegroup_leftlog->SetIsActive(true);
    UpdateSideTable(x70_tablegroup_leftlog);
    x18_firstViewRightSel = 0;
    x1c_rightSel = 0;
    x84_tablegroup_rightlog->SetUserSelection(1);
    UpdateSideTable(x84_tablegroup_rightlog);
    break;
  case EMode::RightTable:
    if (IsRightLogDynamic())
      UpdateRightLogColors(true, color, colorDim);
    else
      x80_basewidget_rightlog->SetColor(colorDim);
    x84_tablegroup_rightlog->SetIsActive(true);
    UpdateSideTable(x84_tablegroup_rightlog);
    break;
  case EMode::TextScroll:
    x6c_basewidget_leftlog->SetColor(colorDim);
    if (IsRightLogDynamic())
      UpdateRightLogColors(true, color, colorDim);
    else
      x80_basewidget_rightlog->SetColor(colorDim);
    x70_tablegroup_leftlog->SetIsActive(false);
    x84_tablegroup_rightlog->SetIsActive(false);
    break;
  default:
    break;
  }

  ChangedMode(oldMode);
}

void CPauseScreenBase::UpdateSideTable(CGuiTableGroup* table) {
  if (!table)
    return;

  zeus::CColor selColor = zeus::CColor::skWhite;
  zeus::CColor deselColor = {1.f, 1.f, 1.f, 0.5f};

  bool tableActive = true;
  if (table == x84_tablegroup_rightlog && x10_mode != EMode::RightTable)
    tableActive = false;

  table->SetColors(selColor, deselColor);

  if (table == x84_tablegroup_rightlog) {
    int sel = x1c_rightSel - x18_firstViewRightSel;
    x8c_model_righthighlight->SetLocalTransform(x8c_model_righthighlight->GetTransform() *
                                                zeus::CTransform::Translate(0.f, 0.f, x38_highlightPitch * sel));
    x8c_model_righthighlight->SetVisibility(x10_mode == EMode::RightTable, ETraversalMode::Children);
    int selInView = x1c_rightSel % 5;
    if (IsRightLogDynamic()) {
      UpdateRightLogHighlight(tableActive, selInView, selColor, deselColor);
    } else {
      for (int i = 0; i < x144_model_titles.size(); ++i)
        x144_model_titles[i]->SetColor((i == selInView && tableActive) ? selColor : deselColor);
    }
  } else {
    int sel = x70_tablegroup_leftlog->GetUserSelection();
    x78_model_lefthighlight->SetLocalTransform(x78_model_lefthighlight->GetTransform() *
                                               zeus::CTransform::Translate(0.f, 0.f, x38_highlightPitch * sel));
    for (int i = 0; i < xc0_model_categories.size(); ++i)
      xc0_model_categories[i]->SetColor(i == sel ? selColor : deselColor);
  }
}

void CPauseScreenBase::Update(float dt, CRandom16& rand, CArchitectureQueue& archQueue) {
  x198_27_canDraw = true;
  x8_frame.Update(dt);
  x14_alpha = std::min(2.f * dt + x14_alpha, 1.f);

  u32 rightCount = GetRightTableCount();
  bool pulseRightUp = x10_mode == EMode::RightTable && x18_firstViewRightSel > 0;
  bool pulseRightDown = x10_mode == EMode::RightTable && x18_firstViewRightSel + 5 < rightCount;
  float rightUpT = pulseRightUp ? CGraphics::GetSecondsMod900() : 0.f;
  float rightDownT = pulseRightDown ? CGraphics::GetSecondsMod900() : 0.f;

  zeus::CColor lowC = g_tweakGuiColors->GetPauseItemAmberColor();
  lowC.a() = 0.2f;
  xa0_model_scrollrightup->SetColor(
      zeus::CColor::lerp(lowC, g_tweakGuiColors->GetPauseItemAmberColor(),
                         zeus::clamp(0.f, (std::sin(5.f * rightUpT - M_PIF / 2.f) + 1.f) * 0.5f, 1.f)));
  xa4_model_scrollrightdown->SetColor(
      zeus::CColor::lerp(lowC, g_tweakGuiColors->GetPauseItemAmberColor(),
                         zeus::clamp(0.f, (std::sin(5.f * rightDownT - M_PIF / 2.f) + 1.f) * 0.5f, 1.f)));

  float textUpT = x198_28_pulseTextArrowTop ? CGraphics::GetSecondsMod900() : 0.f;
  float textDownT = x198_29_pulseTextArrowBottom ? CGraphics::GetSecondsMod900() : 0.f;

  x90_model_textarrowtop->SetColor(
      zeus::CColor::lerp(lowC, g_tweakGuiColors->GetPauseItemAmberColor(),
                         zeus::clamp(0.f, (std::sin(5.f * textUpT - M_PIF / 2.f) + 1.f) * 0.5f, 1.f)));
  x94_model_textarrowbottom->SetColor(
      zeus::CColor::lerp(lowC, g_tweakGuiColors->GetPauseItemAmberColor(),
                         zeus::clamp(0.f, (std::sin(5.f * textDownT - M_PIF / 2.f) + 1.f) * 0.5f, 1.f)));
}

void CPauseScreenBase::ProcessControllerInput(const CFinalInput& input) {
  x198_25_handledInput = false;
  x8_frame.ProcessUserInput(input);
}

void CPauseScreenBase::Draw(float mainAlpha, float frameAlpha, float yOff) {
  zeus::CColor color = zeus::CColor::skWhite;
  color.a() = mainAlpha * x14_alpha;
  x60_basewidget_pivot->SetColor(color);
  color.a() = frameAlpha;
  x64_basewidget_bgframe->SetColor(color);

  CGuiWidgetDrawParms parms(1.f, zeus::CVector3f{0.f, 15.f * yOff, 0.f});
  x8_frame.Draw(parms);
}

void CPauseScreenBase::UpdateRightTable() {
  x18_firstViewRightSel = 0;
  x1c_rightSel = 0;
  x84_tablegroup_rightlog->SetUserSelection(1);
  UpdateSideTable(x84_tablegroup_rightlog);
}

void CPauseScreenBase::SetRightTableSelection(int oldSel, int newSel) {
  int oldRightSel = x1c_rightSel;
  x1c_rightSel = zeus::clamp(0, x1c_rightSel + (newSel - oldSel), int(GetRightTableCount()) - 1);
  if (oldRightSel != x1c_rightSel)
    CSfxManager::SfxStart(SFXui_table_selection_change, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);

  if (x1c_rightSel < x18_firstViewRightSel)
    x18_firstViewRightSel = x1c_rightSel;
  else if (x1c_rightSel >= x18_firstViewRightSel + 5)
    x18_firstViewRightSel = x1c_rightSel - 4;

  x84_tablegroup_rightlog->SetUserSelection(x1c_rightSel + 1 - x18_firstViewRightSel);
  UpdateSideTable(x84_tablegroup_rightlog);
  RightTableSelectionChanged(oldSel, newSel);
}

void CPauseScreenBase::OnLeftTableAdvance(CGuiTableGroup* caller) {
  if (ShouldLeftTableAdvance()) {
    ChangeMode(EMode::RightTable);
    x198_25_handledInput = true;
    CSfxManager::SfxStart(SFXui_advance, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
  }
}

void CPauseScreenBase::OnRightTableAdvance(CGuiTableGroup* caller) {
  if (ShouldRightTableAdvance() && !x198_25_handledInput) {
    ChangeMode(EMode::TextScroll);
    CSfxManager::SfxStart(SFXui_advance, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
  }
}

void CPauseScreenBase::OnTableSelectionChange(CGuiTableGroup* caller, int oldSel) {
  UpdateSideTable(caller);
  if (x70_tablegroup_leftlog == caller) {
    CSfxManager::SfxStart(SFXui_table_selection_change, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
    UpdateRightTable();
  } else {
    SetRightTableSelection(oldSel, x84_tablegroup_rightlog->GetUserSelection());
  }
}

void CPauseScreenBase::OnRightTableCancel(CGuiTableGroup* caller) { ChangeMode(EMode::LeftTable); }

static const char* PaneSuffixes[] = {"0", "1", "2", "3", "01", "12", "23", "012", "123", "0123",
                                     "4", "5", "6", "7", "45", "56", "67", "456", "567", "4567"};

std::string CPauseScreenBase::GetImagePaneName(u32 i) { return hecl::Format("imagepane_pane%s", PaneSuffixes[i]); }

} // namespace urde::MP1
