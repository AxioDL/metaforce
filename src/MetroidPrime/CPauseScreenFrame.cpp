#include "MetroidPrime/COptionsScreen.hpp"
#include "MetroidPrime/CQuitGameScreen.hpp"
#include "MetroidPrime/SOptionsFrontEndFrame.hpp"

#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiSliderGroup.hpp"
#include "GuiSys/CGuiTableGroup.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/CArchitectureQueue.hpp"
#include "MetroidPrime/CGameCubeDoll.hpp"
#include "MetroidPrime/CSaveGameScreen.hpp"
#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"
#include "MetroidPrime/Decode.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/RumbleFxTable.hpp"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"
#include "rstl/StringExtras.hpp"
#include "rstl/math.hpp"
#include <stdio.h>

static const int skQuitTitles[] = {24, 25, 26, 27, 28};

CQuitGameScreen::CQuitGameScreen(EQuitType type)
: x0_type(type)
, x4_frame(gpSimplePool->GetObj("FRME_QuitScreen"))
, x10_loadedFrame(nullptr)
, x14_tablegroup_quitgame(nullptr)
, x18_action(kQA_None) {
  x4_frame.Lock();
}

void CQuitGameScreen::ProcessUserInput(const CFinalInput& input) {
  if (input.ControllerNumber() != 0) {
    return;
  }
  if (x10_loadedFrame != nullptr) {
    x10_loadedFrame->ProcessUserInput(input);
    if (input.PB() && x0_type != kQT_ContinueFromLastSave) {
      x18_action = kQA_No;
    }
  }
}

void CQuitGameScreen::Draw() const {
  if (x0_type == kQT_QuitGame) {
    CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Blend, CCameraFilterPass::kFS_Fullscreen,
                                  CColor::Black().WithAlphaOf(0.5f), nullptr, 1.f);
  }
  const float offsets[] = {0.f, 1.6f, 1.f, 0.f, 1.f};
  if (x10_loadedFrame != nullptr) {
    x10_loadedFrame->Draw(CGuiWidgetDrawParms(1.f, CVector3f(0.f, 0.f, offsets[x0_type])));
  }
}

EQuitAction CQuitGameScreen::Update(float dt) {
  if (x10_loadedFrame == nullptr && x4_frame.TryCache()) {
    FinishedLoading();
  }
  return x18_action;
}

void CQuitGameScreen::DoAdvance(CGuiTableGroup* caller) {
  if (caller->GetUserSelection() == 0) {
    CSfxManager::SfxStart(0x598, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
    x18_action = kQA_Yes;
  } else {
    CSfxManager::SfxStart(0x597, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
    x18_action = kQA_No;
  }
}

void CQuitGameScreen::DoSelectionChange(CGuiTableGroup* caller, int oldSel) {
  SetColors();
  CSfxManager::SfxStart(0x590, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                        CSfxManager::kAllAreas);
}

void CQuitGameScreen::FinishedLoading() {
  x10_loadedFrame = x4_frame.GetObject();
  x14_tablegroup_quitgame =
      static_cast< CGuiTableGroup* >(x10_loadedFrame->FindWidget("tablegroup_quitgame"));
  x14_tablegroup_quitgame->SetVertical(false);
  x14_tablegroup_quitgame->SetMenuAdvanceCallback(
      TFunctor1FromMethod< CQuitGameScreen, CGuiTableGroup* const >::Make(
          *this, &CQuitGameScreen::DoAdvance));
  x14_tablegroup_quitgame->SetMenuSelectionChangeCallback(
      TFunctor2FromMethod< CQuitGameScreen, CGuiTableGroup* const, const int >::Make(
          *this, &CQuitGameScreen::DoSelectionChange));
  CGuiTextPane* title = static_cast< CGuiTextPane* >(x10_loadedFrame->FindWidget("textpane_title"));
  title->TextSupport().SetText(rstl::wstring_l(gpStringTable->GetString(skQuitTitles[x0_type])));
  CGuiTextPane* yes = static_cast< CGuiTextPane* >(x10_loadedFrame->FindWidget("textpane_yes"));
  yes->TextSupport().SetText(rstl::wstring_l(gpStringTable->GetString(22)));
  CGuiTextPane* no = static_cast< CGuiTextPane* >(x10_loadedFrame->FindWidget("textpane_no"));
  no->TextSupport().SetText(rstl::wstring_l(gpStringTable->GetString(23)));
  const int defaults[] = {1, 0, 1, 1, 0};
  x14_tablegroup_quitgame->SetUserSelection(defaults[x0_type]);
  SetColors();
}

void CQuitGameScreen::SetColors() {
  const CColor selected(uchar(200), uchar(200), uchar(200), uchar(255));
  const CColor unselected(uchar(50), uchar(50), uchar(50), uchar(255));
  const int selection = x14_tablegroup_quitgame->GetUserSelection();
  for (int i = 0; i < 2; ++i) {
    CGuiWidget* worker = x14_tablegroup_quitgame->GetWorkerWidget(i);
    worker->SetColor(i == selection ? selected : unselected);
  }
}

static void SetTextPanePair(CGuiFrame* frame, const char* name, const wchar_t* text) {
  CGuiTextPane* pane = static_cast< CGuiTextPane* >(frame->FindWidget(name));
  pane->TextSupport().SetText(rstl::wstring(text));
  CGuiTextPane* shadow =
      static_cast< CGuiTextPane* >(frame->FindWidget(CBasics::Stringize("%sb", name)));
  shadow->TextSupport().SetText(rstl::wstring(text));
}

enum EOptionType { kOT_Float, kOT_DoubleEnum, kOT_TripleEnum, kOT_RestoreDefaults };
struct SGameOption {
  EGameOption option;
  int stringId;
  float minVal;
  float maxVal;
  float increment;
  EOptionType type;
};

static const SGameOption skVisorOptions[] = {
    {kGO_VisorOpacity, 21, 0.f, 255.f, 1.f, kOT_Float},
    {kGO_HelmetOpacity, 22, 0.f, 255.f, 1.f, kOT_Float},
    {kGO_HUDLag, 23, 0.f, 1.f, 1.f, kOT_DoubleEnum},
    {kGO_HintSystem, 24, 0.f, 1.f, 1.f, kOT_DoubleEnum},
    {kGO_RestoreDefaults, 35, 0.f, 1.f, 1.f, kOT_RestoreDefaults},
};
static const SGameOption skDisplayOptions[] = {
    {kGO_ScreenBrightness, 25, 0.f, 8.f, 1.f, kOT_Float},
    {kGO_ScreenOffsetX, 26, -30.f, 30.f, 1.f, kOT_Float},
    {kGO_ScreenOffsetY, 27, -30.f, 30.f, 1.f, kOT_Float},
    {kGO_ScreenStretch, 28, -10.f, 10.f, 1.f, kOT_Float},
    {kGO_RestoreDefaults, 35, 0.f, 1.f, 1.f, kOT_RestoreDefaults},
};
static const SGameOption skSoundOptions[] = {
    {kGO_SFXVolume, 29, 0.f, 127.f, 1.f, kOT_Float},
    {kGO_MusicVolume, 30, 0.f, 127.f, 1.f, kOT_Float},
    {kGO_SoundMode, 31, 0.f, 2.f, 1.f, kOT_TripleEnum},
    {kGO_RestoreDefaults, 35, 0.f, 1.f, 1.f, kOT_RestoreDefaults},
};
static const SGameOption skControllerOptions[] = {
    {kGO_ReverseYAxis, 32, 0.f, 1.f, 1.f, kOT_DoubleEnum},
    {kGO_Rumble, 33, 0.f, 1.f, 1.f, kOT_DoubleEnum},
    {kGO_SwapBeamControls, 34, 0.f, 1.f, 1.f, kOT_DoubleEnum},
    {kGO_RestoreDefaults, 35, 0.f, 1.f, 1.f, kOT_RestoreDefaults},
};
struct SOptionCategory {
  int count;
  const SGameOption* options;
};
static SOptionCategory skGameOptions[] = {
    {5, skVisorOptions},      {5, skDisplayOptions}, {4, skSoundOptions},
    {4, skControllerOptions}, {0, nullptr},
};

int CGameOptions::GetOption(EGameOption option) {
  const CGameOptions& options = gpGameState->GameOptions();
  switch (option) {
  case kGO_VisorOpacity:
    return options.x60_hudAlpha;
  case kGO_HelmetOpacity:
    return options.x64_helmetAlpha;
  case kGO_HUDLag:
    return options.GetHUDLag() ? 1 : 0;
  case kGO_HintSystem:
    return options.GetIsHintSystemEnabled() ? 1 : 0;
  case kGO_ScreenBrightness:
    return options.x48_screenBrightness;
  case kGO_ScreenOffsetX:
    return options.x4c_screenXOffset;
  case kGO_ScreenOffsetY:
    return options.x50_screenYOffset;
  case kGO_ScreenStretch:
    return options.x54_screenStretch;
  case kGO_SFXVolume:
    return options.x58_sfxVol;
  case kGO_MusicVolume:
    return options.x5c_musicVol;
  case kGO_SoundMode:
    return options.x44_soundMode;
  case kGO_ReverseYAxis:
    return options.GetInvertYAxis() ? 1 : 0;
  case kGO_Rumble:
    return options.GetIsRumbleEnabled() ? 1 : 0;
  case kGO_SwapBeamControls:
    return options.GetSwapBeamControls() ? 1 : 0;
  default:
    return 0;
  }
}

void CGameOptions::SetOption(EGameOption option, int value) {
  CGameOptions& options = gpGameState->GameOptions();
  switch (option) {
  case kGO_VisorOpacity:
    options.x60_hudAlpha = value;
    break;
  case kGO_HelmetOpacity:
    options.SetHelmetAlpha(value);
    break;
  case kGO_HUDLag:
    options.SetHUDLag(value > 0);
    break;
  case kGO_HintSystem:
    options.SetIsHintSystemEnabled(value > 0);
    break;
  case kGO_ScreenBrightness:
    options.SetScreenBrightness(value, true);
    break;
  case kGO_ScreenOffsetX:
    options.SetScreenPositionX(value, true);
    break;
  case kGO_ScreenOffsetY:
    options.SetScreenPositionY(value, true);
    break;
  case kGO_ScreenStretch:
    options.SetScreenStretch(value, true);
    break;
  case kGO_SFXVolume:
    options.SetSfxVolume(value, true);
    break;
  case kGO_MusicVolume:
    options.SetMusicVolume(value, true);
    break;
  case kGO_SoundMode:
    options.SetSurroundMode(static_cast< CAudioSys::ESurroundModes >(value), true);
    break;
  case kGO_ReverseYAxis:
    options.SetInvertYAxis(value > 0);
    break;
  case kGO_Rumble:
    options.SetIsRumbleEnabled(value > 0);
    break;
  case kGO_SwapBeamControls:
    options.ToggleControls(value > 0);
    break;
  default:
    break;
  }
}

void CGameOptions::TryRestoreDefaults(const CFinalInput& input, int category, int option,
                                      bool frontEnd) {
  const SOptionCategory& cat = skGameOptions[category];
  if (cat.count != 0 && cat.options[option].option == kGO_RestoreDefaults && input.PA()) {
    if (frontEnd) {
      CSfxManager::SfxStart(0x448, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
      CSfxManager::SfxStart(0x443, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
    } else {
      CSfxManager::SfxStart(0x598, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
    }
    CGameOptions& options = gpGameState->GameOptions();
    switch (category) {
    case 0:
#if VERSION >= VERSION_GM8P_00
      options.SetHudAlpha(255);
#else
      options.x60_hudAlpha = 255;
#endif
      options.SetHelmetAlpha(255);
      options.SetHUDLag(skDefaultHudLag);
      options.SetIsHintSystemEnabled(skDefaultHintSystem);
#if VERSION >= VERSION_GM8P_00
      options.fn_80200564(skDefaultPalFlag);
#endif
      break;
    case 1:
      options.SetScreenBrightness(4, true);
      options.SetScreenPositionX(0, true);
      options.SetScreenPositionY(0, true);
      options.SetScreenStretch(0, true);
      break;
    case 2:
      options.SetSfxVolume(127, true);
      options.SetMusicVolume(127, true);
      options.SetSurroundMode(CAudioSys::kSM_Stereo, true);
      break;
    case 3:
      options.SetInvertYAxis(skDefaultInvertY);
      options.SetIsRumbleEnabled(skDefaultRumble);
      options.ToggleControls(skDefaultSwapBeamsControls);
      break;
    default:
      break;
    }
  }
}

COptionsScreen::COptionsScreen(const CStateManager& mgr, CGuiFrame& frame,
                               const CStringTable& pauseStrg)
: CPauseScreenBase(mgr, frame, pauseStrg)
, x19c_quitGame(nullptr)
, x1a0_gameCube(rs_new CGameCubeDoll())
, x29c_optionAlpha(0.f)
, x2a0_24_inOptionBody(false) {}

COptionsScreen::~COptionsScreen() { CSfxManager::SfxStop(x1a4_sliderSfx); }

bool COptionsScreen::VReady() const { return true; }

bool COptionsScreen::InputDisabled() const { return !x19c_quitGame.null(); }

void COptionsScreen::Update(float dt, CRandom16& rand, CArchitectureQueue& queue) {
  x1a8_rumble.Update(dt);
  CPauseScreenBase::Update(dt, rand, queue);
  const bool sliding = x18c_slidergroup_slider->GetState() != CGuiSliderGroup::kS_None;
  if (bool(x1a4_sliderSfx) != sliding) {
    if (sliding) {
      x1a4_sliderSfx = CSfxManager::SfxStart(0x5ab, 0x7f, 0x40, false, CSfxManager::kMedPriority,
                                             false, CSfxManager::kAllAreas);
    } else {
      CSfxManager::SfxStop(x1a4_sliderSfx);
      x1a4_sliderSfx.Clear();
    }
  }
  if (x2a0_24_inOptionBody) {
    x29c_optionAlpha = rstl::min_val(1.f, x29c_optionAlpha + 4.f * dt);
  } else {
    x29c_optionAlpha = rstl::max_val(0.f, x29c_optionAlpha - 4.f * dt);
  }
  if (close_enough(x29c_optionAlpha, 0.f)) {
    ResetOptionWidgetVisibility();
    x174_textpane_body->SetIsVisible(false);
  }
  const CColor color =
      gpTweakGuiColors->GetPauseItemAmberColor().WithAlphaModulatedBy(x29c_optionAlpha);
  x18c_slidergroup_slider->SetColor(color);
  x190_tablegroup_double->SetColor(color);
  x194_tablegroup_triple->SetColor(color);
  if (!x19c_quitGame.null()) {
    const EQuitAction action = x19c_quitGame->Update(dt);
    if (action == kQA_Yes) {
      queue.Push(MakeMsg::CreateQuitGameplay(kAMT_Game));
      CSfxManager::SetChannel(CSfxManager::kSC_Default);
      CSfxManager::SfxStart(0x58e, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
    } else if (action == kQA_No) {
      CSfxManager::SfxStart(0x58f, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
      x19c_quitGame = nullptr;
    }
  }
  x1a0_gameCube->Update(dt);
}

void COptionsScreen::Touch() {
  CPauseScreenBase::Touch();
  x1a0_gameCube->Touch();
}

void COptionsScreen::ProcessInput(const CFinalInput& input) {
  if (x19c_quitGame.null()) {
    CPauseScreenBase::ProcessInput(input);
    CGameOptions::TryRestoreDefaults(input, x70_tablegroup_leftlog->GetUserSelection(),
                                     x1c_rightSel, false);
    if (x70_tablegroup_leftlog->GetUserSelection() == 4 && input.PA()) {
      x19c_quitGame = rs_new CQuitGameScreen(kQT_QuitGame);
    }
  } else {
    x19c_quitGame->ProcessUserInput(input);
  }
}

void COptionsScreen::Draw(float transInterp, float totalAlpha, float yOff) const {
  CPauseScreenBase::Draw(transInterp, totalAlpha, yOff);
  x1a0_gameCube->Draw(transInterp * (1.f - x29c_optionAlpha));
  if (!x19c_quitGame.null()) {
    CGraphics::SetDepthRange(0.f, 0.001f);
    x19c_quitGame->Draw();
    CGraphics::SetDepthRange(0.f, 1.f);
  }
}

void COptionsScreen::VActivate() {
  for (int i = 0; i < 5; ++i) {
    CGuiTextPane* pane = xa8_textpane_categories[i];
    pane->TextSupport().SetText(rstl::wstring(xc_pauseStrg.GetString(i + 16)));
  }
  x178_textpane_title->TextSupport().SetText(rstl::wstring(xc_pauseStrg.GetString(15)));
  for (int i = ARRAY_SIZE(skGameOptions); i < xa8_textpane_categories.capacity(); ++i) {
    x70_tablegroup_leftlog->GetWorkerWidget(i)->SetIsSelectable(false);
  }
  x174_textpane_body->TextSupport().SetJustification(kJustification_Center);
  x174_textpane_body->TextSupport().SetVerticalJustification(kVerticalJustification_Bottom);
  CGuiTextPane* off = static_cast< CGuiTextPane* >(x190_tablegroup_double->GetWorkerWidget(0));
  off->TextSupport().SetText(rstl::wstring_l(xc_pauseStrg.GetString(95)));
  CGuiTextPane* on = static_cast< CGuiTextPane* >(x190_tablegroup_double->GetWorkerWidget(1));
  on->TextSupport().SetText(rstl::wstring_l(xc_pauseStrg.GetString(94)));
  CGuiTextPane* mono = static_cast< CGuiTextPane* >(x194_tablegroup_triple->GetWorkerWidget(0));
  mono->TextSupport().SetText(rstl::wstring_l(xc_pauseStrg.GetString(96)));
  CGuiTextPane* stereo = static_cast< CGuiTextPane* >(x194_tablegroup_triple->GetWorkerWidget(1));
  stereo->TextSupport().SetText(rstl::wstring_l(xc_pauseStrg.GetString(97)));
  CGuiTextPane* surround = static_cast< CGuiTextPane* >(x194_tablegroup_triple->GetWorkerWidget(2));
  surround->TextSupport().SetText(rstl::wstring_l(xc_pauseStrg.GetString(98)));
  x18c_slidergroup_slider->SetSelectionChangedCallback(
      TFunctor2FromMethod< COptionsScreen, CGuiSliderGroup* const, const float >::Make(
          *this, &COptionsScreen::OnSliderChanged));
  x190_tablegroup_double->SetMenuSelectionChangeCallback(
      TFunctor2FromMethod< COptionsScreen, CGuiTableGroup* const, const int >::Make(
          *this, &COptionsScreen::OnEnumChanged));
  x194_tablegroup_triple->SetMenuSelectionChangeCallback(
      TFunctor2FromMethod< COptionsScreen, CGuiTableGroup* const, const int >::Make(
          *this, &COptionsScreen::OnEnumChanged));
}

bool COptionsScreen::ShouldRightTableAdvance() { return false; }

bool COptionsScreen::ShouldLeftTableAdvance() {
  return x70_tablegroup_leftlog->GetUserSelection() != 4;
}

uint COptionsScreen::GetRightTableCount() const {
  return skGameOptions[x70_tablegroup_leftlog->GetUserSelection()].count;
}

void COptionsScreen::UpdateRightTable() {
  CPauseScreenBase::UpdateRightTable();
  const SOptionCategory& category = skGameOptions[x70_tablegroup_leftlog->GetUserSelection()];
  for (int i = 0; i < 5; ++i) {
    CGuiTextPane* pane = xd8_textpane_titles[i];
    if (i < category.count) {
      pane->TextSupport().SetText(
          rstl::wstring(xc_pauseStrg.GetString(category.options[i].stringId)));
    } else {
      pane->TextSupport().SetText(rstl::wstring_l(L""));
    }
  }
}

void COptionsScreen::ChangedMode(EMode oldMode) {
  if (x10_mode == kM_RightTable) {
    x174_textpane_body->SetIsVisible(true);
    UpdateOptionView();
    x2a0_24_inOptionBody = true;
  } else {
    x2a0_24_inOptionBody = false;
  }
}

void COptionsScreen::RightTableSelectionChanged(int oldSel, int newSel) { UpdateOptionView(); }

void COptionsScreen::ResetOptionWidgetVisibility() {
  x18c_slidergroup_slider->SetIsActive(false);
  x18c_slidergroup_slider->SetVisibility(false, kTM_Children);
  x190_tablegroup_double->SetIsVisible(false);
  x190_tablegroup_double->SetIsActive(false);
  x194_tablegroup_triple->SetIsActive(false);
  x194_tablegroup_triple->SetIsVisible(false);
}

void COptionsScreen::UpdateOptionView() {
  ResetOptionWidgetVisibility();
  const SOptionCategory& category = skGameOptions[x70_tablegroup_leftlog->GetUserSelection()];
  if (category.count == 0) {
    return;
  }
  const SGameOption& opt = category.options[x1c_rightSel];
  const float zOff = x38_highlightPitch * x1c_rightSel;
  switch (opt.type) {
  case kOT_Float:
    x18c_slidergroup_slider->SetIsActive(true);
    x18c_slidergroup_slider->SetVisibility(true, kTM_Children);
    x18c_slidergroup_slider->SetMinVal(opt.minVal);
    x18c_slidergroup_slider->SetMaxVal(opt.maxVal);
    x18c_slidergroup_slider->SetIncrement(opt.increment);
    x18c_slidergroup_slider->SetCurVal(CGameOptions::GetOption(opt.option));
    x18c_slidergroup_slider->SetLocalPosition(x3c_sliderStart + CVector3f(0.f, 0.f, zOff));
    break;
  case kOT_DoubleEnum:
    x190_tablegroup_double->SetUserSelection(CGameOptions::GetOption(opt.option));
    x190_tablegroup_double->SetIsVisible(true);
    x190_tablegroup_double->SetIsActive(true);
    UpdateSideTable(x190_tablegroup_double);
    x190_tablegroup_double->SetLocalPosition(x48_tableDoubleStart + CVector3f(0.f, 0.f, zOff));
    break;
  case kOT_TripleEnum:
    x194_tablegroup_triple->SetUserSelection(CGameOptions::GetOption(opt.option));
    x194_tablegroup_triple->SetIsVisible(true);
    x194_tablegroup_triple->SetIsActive(true);
    UpdateSideTable(x194_tablegroup_triple);
    x194_tablegroup_triple->SetLocalPosition(x54_tableTripleStart + CVector3f(0.f, 0.f, zOff));
    break;
  case kOT_RestoreDefaults:
    break;
  default:
    break;
  }
}

void COptionsScreen::OnSliderChanged(CGuiSliderGroup* caller, float value) {
  if (x10_mode == kM_RightTable) {
    const SOptionCategory& category = skGameOptions[x70_tablegroup_leftlog->GetUserSelection()];
    const EGameOption option = category.options[x1c_rightSel].option;
    CGameOptions::SetOption(option, caller->GetCurVal());
  }
}

void COptionsScreen::OnEnumChanged(CGuiTableGroup* caller, int oldSel) {
  if (x10_mode == kM_RightTable) {
    const SOptionCategory& category = skGameOptions[x70_tablegroup_leftlog->GetUserSelection()];
    const SGameOption& option = category.options[x1c_rightSel];
    const int selection = caller->GetUserSelection();
    CGameOptions::SetOption(option.option, selection);
    if (option.option == kGO_Rumble && selection > 0) {
      x1a8_rumble.HardStopAll();
      x1a8_rumble.Rumble(skRumbleFxTable[kRFX_PlayerBump], 1.f, kRP_One, kIOP_Player1);
    }
    CPauseScreenBase::UpdateSideTable(caller);
    CSfxManager::SfxStart(0x59d, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
  }
}

SOptionsFrontEndFrame::SOptionsFrontEndFrame()
: x0_uiAlpha(0.f)
, x4_frme(gpSimplePool->GetObj("FRME_OptionsFrontEnd"))
, x10_pauseScreen(gpSimplePool->GetObj("STRG_PauseScreen"))
, x1c_loadedFrame(nullptr)
, x20_loadedPauseStrg(nullptr)
, x24_tablegroup_leftmenu(nullptr)
, x28_tablegroup_rightmenu(nullptr)
, x2c_tablegroup_double(nullptr)
, x30_tablegroup_triple(nullptr)
, x34_slidergroup_slider(nullptr)
, x38_rowPitch(0.f)
, x134_24_visible(true)
, x134_25_exitOptions(false) {
  x4_frme.Lock();
  x10_pauseScreen.Lock();
}

SOptionsFrontEndFrame::~SOptionsFrontEndFrame() { CSfxManager::SfxStop(x3c_sliderSfx); }

bool SOptionsFrontEndFrame::PumpLoad() {
  if (x1c_loadedFrame != nullptr) {
    return true;
  }
  if (x4_frme.TryCache() && x10_pauseScreen.TryCache()) {
    CGuiFrame* frame = x4_frme.GetObject();
    if (frame->GetIsFinishedLoading()) {
      x1c_loadedFrame = frame;
      x20_loadedPauseStrg = x10_pauseScreen.GetObject();
      FinishedLoading();
      return true;
    }
  }
  return false;
}

void SOptionsFrontEndFrame::FinishedLoading() {
  x24_tablegroup_leftmenu =
      static_cast< CGuiTableGroup* >(x1c_loadedFrame->FindWidget("tablegroup_leftmenu"));
  x28_tablegroup_rightmenu =
      static_cast< CGuiTableGroup* >(x1c_loadedFrame->FindWidget("tablegroup_rightmenu"));
  x2c_tablegroup_double =
      static_cast< CGuiTableGroup* >(x1c_loadedFrame->FindWidget("tablegroup_double"));
  x30_tablegroup_triple =
      static_cast< CGuiTableGroup* >(x1c_loadedFrame->FindWidget("tablegroup_triple"));
  x34_slidergroup_slider =
      static_cast< CGuiSliderGroup* >(x1c_loadedFrame->FindWidget("slidergroup_slider"));
  x24_tablegroup_leftmenu->SetMenuAdvanceCallback(
      TFunctor1FromMethod< SOptionsFrontEndFrame, CGuiTableGroup* const >::Make(
          *this, &SOptionsFrontEndFrame::DoLeftMenuAdvance));
  x24_tablegroup_leftmenu->SetMenuSelectionChangeCallback(
      TFunctor2FromMethod< SOptionsFrontEndFrame, CGuiTableGroup* const, const int >::Make(
          *this, &SOptionsFrontEndFrame::DoMenuSelectionChange));
  x38_rowPitch = x24_tablegroup_leftmenu->GetWorkerWidget(1)->GetIdlePosition().GetZ() -
                 x24_tablegroup_leftmenu->GetWorkerWidget(0)->GetIdlePosition().GetZ();
  x28_tablegroup_rightmenu->SetMenuSelectionChangeCallback(
      TFunctor2FromMethod< SOptionsFrontEndFrame, CGuiTableGroup* const, const int >::Make(
          *this, &SOptionsFrontEndFrame::DoMenuSelectionChange));
  x28_tablegroup_rightmenu->SetMenuCancelCallback(
      TFunctor1FromMethod< SOptionsFrontEndFrame, CGuiTableGroup* const >::Make(
          *this, &SOptionsFrontEndFrame::DoMenuCancel));
  x2c_tablegroup_double->SetMenuSelectionChangeCallback(
      TFunctor2FromMethod< SOptionsFrontEndFrame, CGuiTableGroup* const, const int >::Make(
          *this, &SOptionsFrontEndFrame::DoMenuSelectionChange));
  x2c_tablegroup_double->SetMenuCancelCallback(
      TFunctor1FromMethod< SOptionsFrontEndFrame, CGuiTableGroup* const >::Make(
          *this, &SOptionsFrontEndFrame::DoMenuCancel));
  x30_tablegroup_triple->SetMenuSelectionChangeCallback(
      TFunctor2FromMethod< SOptionsFrontEndFrame, CGuiTableGroup* const, const int >::Make(
          *this, &SOptionsFrontEndFrame::DoMenuSelectionChange));
  x30_tablegroup_triple->SetMenuCancelCallback(
      TFunctor1FromMethod< SOptionsFrontEndFrame, CGuiTableGroup* const >::Make(
          *this, &SOptionsFrontEndFrame::DoMenuCancel));
  x34_slidergroup_slider->SetSelectionChangedCallback(
      TFunctor2FromMethod< SOptionsFrontEndFrame, CGuiSliderGroup* const, const float >::Make(
          *this, &SOptionsFrontEndFrame::DoSliderChange));

  SetTextPanePair(x1c_loadedFrame, "textpane_double0", x20_loadedPauseStrg->GetString(95));
  SetTextPanePair(x1c_loadedFrame, "textpane_double1", x20_loadedPauseStrg->GetString(94));
  SetTextPanePair(x1c_loadedFrame, "textpane_triple0", x20_loadedPauseStrg->GetString(96));
  SetTextPanePair(x1c_loadedFrame, "textpane_triple1", x20_loadedPauseStrg->GetString(97));
  SetTextPanePair(x1c_loadedFrame, "textpane_triple2", x20_loadedPauseStrg->GetString(98));
  SetTextPanePair(x1c_loadedFrame, "textpane_title", gpStringTable->GetString(99));
  if (CGuiTextPane* proceed =
          static_cast< CGuiTextPane* >(x1c_loadedFrame->FindWidget("textpane_proceed"))) {
    proceed->TextSupport().SetText(rstl::wstring_l(gpStringTable->GetString(85)));
  }
  if (CGuiTextPane* cancel =
          static_cast< CGuiTextPane* >(x1c_loadedFrame->FindWidget("textpane_cancel"))) {
    cancel->TextSupport().SetText(rstl::wstring_l(gpStringTable->GetString(82)));
  }
  for (int i = 0; i < 4; ++i) {
    char name[32];
#if NONMATCHING
    snprintf(name, sizeof(name), "textpane_filename%d", i);
#else
    sprintf(name, "textpane_filename%d", i);
#endif
    SetTextPanePair(x1c_loadedFrame, name, x20_loadedPauseStrg->GetString(16 + i));
  }
  x2c_tablegroup_double->SetVertical(false);
  x30_tablegroup_triple->SetVertical(false);
  x24_tablegroup_leftmenu->SetIsActive(true);
  x28_tablegroup_rightmenu->SetIsActive(false);
  SetTableColors(x24_tablegroup_leftmenu);
  SetTableColors(x28_tablegroup_rightmenu);
  SetTableColors(x2c_tablegroup_double);
  SetTableColors(x30_tablegroup_triple);
  SetRightUIText();
  DeactivateRightMenu();
}

void SOptionsFrontEndFrame::SetRightUIText() {
  const SOptionCategory& options = skGameOptions[x24_tablegroup_leftmenu->GetUserSelection()];
  for (int i = 0; i < 5; ++i) {
    char name[32];
#if defined(TARGET_PC)
    snprintf(name, sizeof(name), "textpane_right%d", i);
#else
    sprintf(name, "textpane_right%d", i);
#endif
    if (i < options.count) {
      SetTextPanePair(x1c_loadedFrame, name,
                      x20_loadedPauseStrg->GetString(options.options[i].stringId));
      x28_tablegroup_rightmenu->GetWorkerWidget(i)->SetIsSelectable(true);
    } else {
      SetTextPanePair(x1c_loadedFrame, name, L"");
      x28_tablegroup_rightmenu->GetWorkerWidget(i)->SetIsSelectable(false);
    }
  }
}

void SOptionsFrontEndFrame::HandleRightSelectionChange() {
  DeactivateRightMenu();
  const SOptionCategory& category = skGameOptions[x24_tablegroup_leftmenu->GetUserSelection()];
  const SGameOption& option = category.options[x28_tablegroup_rightmenu->GetUserSelection()];
  switch (option.type) {
  case kOT_Float:
    x34_slidergroup_slider->SetIsActive(true);
    x34_slidergroup_slider->SetVisibility(true, kTM_Children);
    x34_slidergroup_slider->SetMinVal(option.minVal);
    x34_slidergroup_slider->SetMaxVal(option.maxVal);
    x34_slidergroup_slider->SetIncrement(option.increment);
    x34_slidergroup_slider->SetCurVal(CGameOptions::GetOption(option.option));
    x34_slidergroup_slider->SetO2PTransform(
        CTransform4f::Translate(0.f, 0.f,
                                x28_tablegroup_rightmenu->GetUserSelection() * x38_rowPitch) *
        x34_slidergroup_slider->GetTransform());
    break;
  case kOT_DoubleEnum:
    x2c_tablegroup_double->SetUserSelection(CGameOptions::GetOption(option.option));
    x2c_tablegroup_double->SetIsVisible(true);
    x2c_tablegroup_double->SetIsActive(true);
    x2c_tablegroup_double->SetO2PTransform(
        CTransform4f::Translate(0.f, 0.f,
                                x28_tablegroup_rightmenu->GetUserSelection() * x38_rowPitch) *
        x2c_tablegroup_double->GetTransform());
    SetTableColors(x2c_tablegroup_double);
    break;
  case kOT_TripleEnum:
    x30_tablegroup_triple->SetUserSelection(CGameOptions::GetOption(option.option));
    x30_tablegroup_triple->SetIsVisible(true);
    x30_tablegroup_triple->SetIsActive(true);
    x30_tablegroup_triple->SetO2PTransform(
        CTransform4f::Translate(0.f, 0.f,
                                x28_tablegroup_rightmenu->GetUserSelection() * x38_rowPitch) *
        x30_tablegroup_triple->GetTransform());
    SetTableColors(x30_tablegroup_triple);
    break;
  default:
    break;
  }
}

void SOptionsFrontEndFrame::DeactivateRightMenu() {
  x2c_tablegroup_double->SetIsActive(false);
  x30_tablegroup_triple->SetIsActive(false);
  x34_slidergroup_slider->SetIsActive(false);
  x2c_tablegroup_double->SetVisibility(false, kTM_Children);
  x30_tablegroup_triple->SetVisibility(false, kTM_Children);
  x34_slidergroup_slider->SetVisibility(false, kTM_Children);
}

void SOptionsFrontEndFrame::DoLeftMenuAdvance(CGuiTableGroup* caller) {
  if (caller == x24_tablegroup_leftmenu) {
    HandleRightSelectionChange();
    x28_tablegroup_rightmenu->SetUserSelection(0);
    x24_tablegroup_leftmenu->SetIsActive(false);
    x28_tablegroup_rightmenu->SetIsActive(true);
    CSfxManager::SfxStart(0x448, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
    CSfxManager::SfxStart(0x443, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
  }
}

void SOptionsFrontEndFrame::DoMenuSelectionChange(CGuiTableGroup* caller, int oldSel) {
  SetTableColors(caller);
  if (caller == x24_tablegroup_leftmenu) {
    SetRightUIText();
    CSfxManager::SfxStart(0x445, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
  } else if (caller == x28_tablegroup_rightmenu) {
    HandleRightSelectionChange();
    CSfxManager::SfxStart(0x445, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
  } else if (caller == x2c_tablegroup_double || caller == x30_tablegroup_triple) {
    if (x28_tablegroup_rightmenu->GetIsActive()) {
      const SOptionCategory& category = skGameOptions[x24_tablegroup_leftmenu->GetUserSelection()];
      const SGameOption& option = category.options[x28_tablegroup_rightmenu->GetUserSelection()];
      const int selection = caller->GetUserSelection();
      CGameOptions::SetOption(option.option, selection);
      CSfxManager::SfxStart(0x447, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
      if (option.option == kGO_Rumble && selection > 0) {
        x40_rumbleGen.HardStopAll();
        x40_rumbleGen.Rumble(skRumbleFxTable[kRFX_PlayerBump], 1.f, kRP_One, kIOP_Player1);
      }
    }
  }
}

void SOptionsFrontEndFrame::DoMenuCancel(CGuiTableGroup* caller) {
  if (caller == x28_tablegroup_rightmenu) {
    DeactivateRightMenu();
    x24_tablegroup_leftmenu->SetIsActive(true);
    x28_tablegroup_rightmenu->SetIsActive(false);
    x28_tablegroup_rightmenu->SetUserSelection(0);
    SetTableColors(x28_tablegroup_rightmenu);
    CSfxManager::SfxStart(0x446, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
  }
}

void SOptionsFrontEndFrame::Update(float dt, const CSaveGameScreen* saveUI) {
  x40_rumbleGen.Update(dt);
  x134_24_visible = saveUI == nullptr || saveUI->GetUIType() == CSaveGameScreen::kUIT_SaveReady;
  if (!PumpLoad()) {
    return;
  }
  x0_uiAlpha = rstl::min_val(1.f, x0_uiAlpha + dt);
  x1c_loadedFrame->Update(dt);
  const bool sliding = x34_slidergroup_slider->GetState() != CGuiSliderGroup::kS_None;
  if (bool(x3c_sliderSfx) != sliding) {
    if (sliding) {
      x3c_sliderSfx = CSfxManager::SfxStart(0x5b2, 0x7f, 0x40, false, CSfxManager::kMedPriority,
                                            false, CSfxManager::kAllAreas);
    } else {
      CSfxManager::SfxStop(x3c_sliderSfx);
      x3c_sliderSfx.Clear();
    }
  }
}

bool SOptionsFrontEndFrame::ProcessUserInput(const CFinalInput& input, CSaveGameScreen* saveUI) {
  x134_25_exitOptions = false;
  if (saveUI != nullptr) {
    saveUI->ProcessUserInput(input);
  }
  if (x1c_loadedFrame != nullptr && x134_24_visible) {
    if (input.PB() && x24_tablegroup_leftmenu->GetIsActive()) {
      x134_25_exitOptions = true;
      CSfxManager::SfxStart(0x446, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
    } else {
      x1c_loadedFrame->ProcessUserInput(input);
      CGameOptions::TryRestoreDefaults(input, x24_tablegroup_leftmenu->GetUserSelection(),
                                       x28_tablegroup_rightmenu->GetUserSelection(), true);
    }
  }
  return !x134_25_exitOptions;
}

void SOptionsFrontEndFrame::Draw() const {
  if (x1c_loadedFrame != nullptr && x134_24_visible) {
    x1c_loadedFrame->Draw(CGuiWidgetDrawParms(x0_uiAlpha, CVector3f::Zero()));
  }
}

void SOptionsFrontEndFrame::SetTableColors(CGuiTableGroup* table) const {
  const CColor selected(uchar(255), uchar(255), uchar(255), uchar(255));
  const CColor unselected(uchar(160), uchar(160), uchar(160), uchar(200));
  table->SetColors(selected, unselected);
}

void SOptionsFrontEndFrame::DoSliderChange(CGuiSliderGroup* caller, float value) {
  if (x28_tablegroup_rightmenu->GetIsActive()) {
    const SOptionCategory& category = skGameOptions[x24_tablegroup_leftmenu->GetUserSelection()];
    const EGameOption option =
        category.options[x28_tablegroup_rightmenu->GetUserSelection()].option;
    CGameOptions::SetOption(option, caller->GetCurVal());
  }
}
