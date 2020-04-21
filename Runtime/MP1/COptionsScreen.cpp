#include "Runtime/MP1/COptionsScreen.hpp"

#include "Runtime/IMain.hpp"
#include "Runtime/CArchitectureQueue.hpp"
#include "Runtime/GuiSys/CGuiSliderGroup.hpp"
#include "Runtime/GuiSys/CGuiTableGroup.hpp"
#include "Runtime/GuiSys/CGuiTextPane.hpp"
#include "Runtime/Input/RumbleFxTable.hpp"

namespace urde::MP1 {

COptionsScreen::COptionsScreen(const CStateManager& mgr, CGuiFrame& frame, const CStringTable& pauseStrg)
: CPauseScreenBase(mgr, frame, pauseStrg), x1a0_gameCube{std::make_unique<CGameCubeDoll>()} {}

COptionsScreen::~COptionsScreen() { CSfxManager::SfxStop(x1a4_sliderSfx); }

void COptionsScreen::UpdateOptionView() {
  ResetOptionWidgetVisibility();

  const std::pair<int, const SGameOption*>& category = GameOptionsRegistry[x70_tablegroup_leftlog->GetUserSelection()];
  if (category.first == 0)
    return;

  float zOff = x38_highlightPitch * x1c_rightSel;
  const SGameOption& opt = category.second[x1c_rightSel];
  switch (opt.type) {
  case EOptionType::Float:
    x18c_slidergroup_slider->SetIsActive(true);
    x18c_slidergroup_slider->SetVisibility(true, ETraversalMode::Children);
    x18c_slidergroup_slider->SetMinVal(opt.minVal);
    x18c_slidergroup_slider->SetMaxVal(opt.maxVal);
    x18c_slidergroup_slider->SetIncrement(opt.increment);
    x18c_slidergroup_slider->SetCurVal(CGameOptions::GetOption(opt.option));
    x18c_slidergroup_slider->SetLocalPosition(x3c_sliderStart + zeus::CVector3f(0.f, 0.f, zOff));
    break;
  case EOptionType::DoubleEnum:
    x190_tablegroup_double->SetUserSelection(CGameOptions::GetOption(opt.option));
    x190_tablegroup_double->SetIsVisible(true);
    x190_tablegroup_double->SetIsActive(true);
    x190_tablegroup_double->SetWorkersMouseActive(true);
    UpdateSideTable(x190_tablegroup_double);
    x190_tablegroup_double->SetLocalPosition(x48_tableDoubleStart + zeus::CVector3f(0.f, 0.f, zOff));
    break;
  case EOptionType::TripleEnum:
    x194_tablegroup_triple->SetUserSelection(CGameOptions::GetOption(opt.option));
    x194_tablegroup_triple->SetIsVisible(true);
    x194_tablegroup_triple->SetIsActive(true);
    x194_tablegroup_triple->SetWorkersMouseActive(true);
    UpdateSideTable(x194_tablegroup_triple);
    x194_tablegroup_triple->SetLocalPosition(x54_tableTripleStart + zeus::CVector3f(0.f, 0.f, zOff));
    break;
  default:
    break;
  }

  x174_textpane_body->SetMouseActive(false);
}

void COptionsScreen::ResetOptionWidgetVisibility() {
  x18c_slidergroup_slider->SetIsActive(false);
  x18c_slidergroup_slider->SetVisibility(false, ETraversalMode::Children);
  x190_tablegroup_double->SetIsVisible(false);
  x190_tablegroup_double->SetIsActive(false);
  x190_tablegroup_double->SetWorkersMouseActive(false);
  x194_tablegroup_triple->SetIsActive(false);
  x194_tablegroup_triple->SetIsVisible(false);
  x194_tablegroup_triple->SetWorkersMouseActive(false);
  x174_textpane_body->SetMouseActive(true);
}

void COptionsScreen::OnSliderChanged(CGuiSliderGroup* caller, float val) {
  if (x10_mode != EMode::RightTable)
    return;

  EGameOption opt = GameOptionsRegistry[x70_tablegroup_leftlog->GetUserSelection()].second[x1c_rightSel].option;
  CGameOptions::SetOption(opt, caller->GetGurVal());
}

void COptionsScreen::OnEnumChanged(CGuiTableGroup* caller, int oldSel) {
  if (x10_mode != EMode::RightTable)
    return;

  EGameOption opt = GameOptionsRegistry[x70_tablegroup_leftlog->GetUserSelection()].second[x1c_rightSel].option;
  CGameOptions::SetOption(opt, caller->GetUserSelection());

  if (opt == EGameOption::Rumble && caller->GetUserSelection() > 0) {
    x1a8_rumble.HardStopAll();
    x1a8_rumble.Rumble(RumbleFxTable[size_t(ERumbleFxId::PlayerBump)], 1.f, ERumblePriority::One, EIOPort::Zero);
  }

  CPauseScreenBase::UpdateSideTable(caller);
  CSfxManager::SfxStart(SFXui_option_enum_change, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
}

bool COptionsScreen::InputDisabled() const { return x19c_quitGame.operator bool(); }

void COptionsScreen::Update(float dt, CRandom16& rand, CArchitectureQueue& archQueue) {
  x1a8_rumble.Update(dt);
  CPauseScreenBase::Update(dt, rand, archQueue);

  if (x1a4_sliderSfx.operator bool() != (x18c_slidergroup_slider->GetState() != CGuiSliderGroup::EState::None)) {
    if (x18c_slidergroup_slider->GetState() != CGuiSliderGroup::EState::None) {
      x1a4_sliderSfx =
          CSfxManager::SfxStart(SFXui_options_slider_change_lp, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
    } else {
      CSfxManager::SfxStop(x1a4_sliderSfx);
      x1a4_sliderSfx.reset();
    }
  }

  if (x2a0_24_inOptionBody)
    x29c_optionAlpha = std::min(x29c_optionAlpha + 4.f * dt, 1.f);
  else
    x29c_optionAlpha = std::max(x29c_optionAlpha - 4.f * dt, 0.f);

  if (std::fabs(x29c_optionAlpha) < 0.00001f) {
    ResetOptionWidgetVisibility();
    x174_textpane_body->SetIsVisible(false);
  }

  zeus::CColor color = g_tweakGuiColors->GetPauseItemAmberColor();
  color.a() = x29c_optionAlpha;
  x18c_slidergroup_slider->SetColor(color);
  x190_tablegroup_double->SetColor(color);
  x194_tablegroup_triple->SetColor(color);

  if (x19c_quitGame) {
    EQuitAction action = x19c_quitGame->Update(dt);
    if (action == EQuitAction::Yes) {
      archQueue.Push(MakeMsg::CreateQuitGameplay(EArchMsgTarget::Game));
      CSfxManager::SetChannel(CSfxManager::ESfxChannels::Default);
      CSfxManager::SfxStart(SFXui_options_quit_accept, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
    } else if (action == EQuitAction::No) {
      CSfxManager::SfxStart(SFXui_options_quit_reject, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
      x19c_quitGame.reset();
    }
  }

  x1a0_gameCube->Update(dt);
}

void COptionsScreen::Touch() {
  CPauseScreenBase::Touch();
  x1a0_gameCube->Touch();
}

void COptionsScreen::ProcessControllerInput(const CFinalInput& input) {
  if (!x19c_quitGame) {
    bool leftClicked = m_leftClicked;
    bool rightClicked = m_rightClicked;
    CPauseScreenBase::ProcessMouseInput(input, 0.f);
    CPauseScreenBase::ProcessControllerInput(input);
    CGameOptions::TryRestoreDefaults(input, x70_tablegroup_leftlog->GetUserSelection(), x1c_rightSel, false,
                                     rightClicked);
    if (x70_tablegroup_leftlog->GetUserSelection() == 4 &&
        (input.PA() || leftClicked || input.PSpecialKey(boo::ESpecialKey::Enter)))
      x19c_quitGame = std::make_unique<CQuitGameScreen>(EQuitType::QuitGame);
  } else {
    CPauseScreenBase::ResetMouseState();
    x19c_quitGame->ProcessUserInput(input);
  }
}

void COptionsScreen::Draw(float transInterp, float totalAlpha, float yOff) {
  SCOPED_GRAPHICS_DEBUG_GROUP("COptionsScreen::Draw", zeus::skPurple);
  CPauseScreenBase::Draw(transInterp, totalAlpha, yOff);
  x1a0_gameCube->Draw(transInterp * (1.f - x29c_optionAlpha));
  if (x19c_quitGame) {
    CGraphics::SetDepthRange(DEPTH_NEAR, 0.001f);
    x19c_quitGame->Draw();
    CGraphics::SetDepthRange(DEPTH_NEAR, DEPTH_FAR);
  }
}

bool COptionsScreen::VReady() const { return true; }

void COptionsScreen::VActivate() {
  for (int i = 0; i < 5; ++i) {
    if (g_Main->IsUSA() && !g_Main->IsTrilogy()) {
      xa8_textpane_categories[i]->TextSupport().SetText(xc_pauseStrg.GetString(i + 16));
    } else {
      xa8_textpane_categories[i]->TextSupport().SetText(xc_pauseStrg.GetString(i + 18));
    }
  }

  x178_textpane_title->TextSupport().SetText(
      xc_pauseStrg.GetString((g_Main->IsUSA() && !g_Main->IsTrilogy()) ? 15 : 17));

#if 0
    for (int i=5 ; i<5 ; ++i)
        x70_tablegroup_leftlog->GetWorkerWidget(i)->SetIsSelectable(false);
#endif

  x174_textpane_body->TextSupport().SetJustification(EJustification::Center);
  x174_textpane_body->TextSupport().SetVerticalJustification(EVerticalJustification::Bottom);

  int stringOffset = (g_Main->IsUSA() && !g_Main->IsTrilogy()) ? 0 : 3;
  static_cast<CGuiTextPane*>(x190_tablegroup_double->GetWorkerWidget(0))
      ->TextSupport()
      .SetText(xc_pauseStrg.GetString(95 + stringOffset));
  static_cast<CGuiTextPane*>(x190_tablegroup_double->GetWorkerWidget(1))
      ->TextSupport()
      .SetText(xc_pauseStrg.GetString(94 + stringOffset));

  static_cast<CGuiTextPane*>(x194_tablegroup_triple->GetWorkerWidget(0))
      ->TextSupport()
      .SetText(xc_pauseStrg.GetString(96 + stringOffset));
  static_cast<CGuiTextPane*>(x194_tablegroup_triple->GetWorkerWidget(1))
      ->TextSupport()
      .SetText(xc_pauseStrg.GetString(97 + stringOffset));
  static_cast<CGuiTextPane*>(x194_tablegroup_triple->GetWorkerWidget(2))
      ->TextSupport()
      .SetText(xc_pauseStrg.GetString(98 + stringOffset));

  x18c_slidergroup_slider->SetSelectionChangedCallback(
      [this](CGuiSliderGroup* caller, float value) { OnSliderChanged(caller, value); });
  x190_tablegroup_double->SetMenuSelectionChangeCallback(
      [this](CGuiTableGroup* caller, int oldSel) { OnEnumChanged(caller, oldSel); });
  x194_tablegroup_triple->SetMenuSelectionChangeCallback(
      [this](CGuiTableGroup* caller, int oldSel) { OnEnumChanged(caller, oldSel); });
}

void COptionsScreen::RightTableSelectionChanged(int oldSel, int newSel) { UpdateOptionView(); }

void COptionsScreen::ChangedMode(EMode oldMode) {
  if (x10_mode == EMode::RightTable) {
    x174_textpane_body->SetIsVisible(true);
    UpdateOptionView();
    x2a0_24_inOptionBody = true;
  } else {
    x2a0_24_inOptionBody = false;
  }
}

void COptionsScreen::UpdateRightTable() {
  CPauseScreenBase::UpdateRightTable();
  const std::pair<int, const SGameOption*>& category =
      (g_Main->IsUSA() && !g_Main->IsTrilogy()) ? GameOptionsRegistry[x70_tablegroup_leftlog->GetUserSelection()]
                                                : GameOptionsRegistryNew[x70_tablegroup_leftlog->GetUserSelection()];
  for (int i = 0; i < 5; ++i) {
    if (i < category.first) {
      xd8_textpane_titles[i]->TextSupport().SetText(xc_pauseStrg.GetString(category.second[i].stringId));
    } else {
      xd8_textpane_titles[i]->TextSupport().SetText(u"");
    }
  }
}

bool COptionsScreen::ShouldLeftTableAdvance() const { return x70_tablegroup_leftlog->GetUserSelection() != 4; }

bool COptionsScreen::ShouldRightTableAdvance() const { return false; }

u32 COptionsScreen::GetRightTableCount() const {
  return GameOptionsRegistry[x70_tablegroup_leftlog->GetUserSelection()].first;
}

} // namespace urde::MP1
