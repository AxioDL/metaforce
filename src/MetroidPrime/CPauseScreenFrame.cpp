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
: mType(type)
, mFrame(gpSimplePool->GetObj("FRME_QuitScreen"))
, mLoadedFrame(nullptr)
, mTablegroup_quitgame(nullptr)
, mAction(kQA_None)
#if VERSION >= VERSION_GM8P_00
, mTextpane_title(nullptr)
, mTextpane_yes(nullptr)
, mTextpane_no(nullptr)
#endif
{
  mFrame.Lock();
}

#if VERSION >= VERSION_GM8P_00
CQuitGameScreen::~CQuitGameScreen() {}
#endif

void CQuitGameScreen::ProcessUserInput(const CFinalInput& input) {
  if (input.ControllerNumber() != 0) {
    return;
  }
  if (mLoadedFrame != nullptr) {
    mLoadedFrame->ProcessUserInput(input);
    if (input.PB() && mType != kQT_ContinueFromLastSave) {
      mAction = kQA_No;
    }
  }
}

void CQuitGameScreen::Draw()
#if VERSION != VERSION_GM8J_00
    const
#endif
{
  if (mType == kQT_QuitGame) {
    CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Blend, CCameraFilterPass::kFS_Fullscreen,
                                  CColor::Black().WithAlphaOf(0.5f), nullptr, 1.f);
  }
  const float offsets[] = {0.f, 1.6f, 1.f, 0.f, 1.f};
  if (mLoadedFrame != nullptr) {
    mLoadedFrame->Draw(CGuiWidgetDrawParms(1.f, CVector3f(0.f, 0.f, offsets[mType])));
  }
}

EQuitAction CQuitGameScreen::Update(float dt) {
  if (mLoadedFrame == nullptr && mFrame.TryCache()) {
    FinishedLoading();
  }
  return mAction;
}

void CQuitGameScreen::DoAdvance(CGuiTableGroup* caller) {
  if (caller->GetUserSelection() == 0) {
    CSfxManager::SfxStart(0x598, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
    mAction = kQA_Yes;
  } else {
    CSfxManager::SfxStart(0x597, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
    mAction = kQA_No;
  }
}

void CQuitGameScreen::DoSelectionChange(CGuiTableGroup* caller, int oldSel) {
  SetColors();
  CSfxManager::SfxStart(0x590, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                        CSfxManager::kAllAreas);
}

void CQuitGameScreen::FinishedLoading() {
  mLoadedFrame = mFrame.GetObject();
  mTablegroup_quitgame =
      static_cast< CGuiTableGroup* >(mLoadedFrame->FindWidget("tablegroup_quitgame"));
  mTablegroup_quitgame->SetVertical(false);
  mTablegroup_quitgame->SetMenuAdvanceCallback(
      TFunctor1FromMethod< CQuitGameScreen, CGuiTableGroup* const >::Make(
          *this, &CQuitGameScreen::DoAdvance));
  mTablegroup_quitgame->SetMenuSelectionChangeCallback(
      TFunctor2FromMethod< CQuitGameScreen, CGuiTableGroup* const, const int >::Make(
          *this, &CQuitGameScreen::DoSelectionChange));
#if VERSION >= VERSION_GM8P_00
  mTextpane_title = static_cast< CGuiTextPane* >(mLoadedFrame->FindWidget("textpane_title"));
  mTextpane_title->TextSupport().SetText(
      rstl::wstring_l(gpStringTable->GetString(skQuitTitles[mType])));
  mTextpane_yes = static_cast< CGuiTextPane* >(mLoadedFrame->FindWidget("textpane_yes"));
  mTextpane_yes->TextSupport().SetText(rstl::wstring_l(gpStringTable->GetString(22)));
  mTextpane_no = static_cast< CGuiTextPane* >(mLoadedFrame->FindWidget("textpane_no"));
  mTextpane_no->TextSupport().SetText(rstl::wstring_l(gpStringTable->GetString(23)));
#else
  CGuiTextPane* title = static_cast< CGuiTextPane* >(mLoadedFrame->FindWidget("textpane_title"));
  title->TextSupport().SetText(rstl::wstring_l(gpStringTable->GetString(skQuitTitles[mType])));
  CGuiTextPane* yes = static_cast< CGuiTextPane* >(mLoadedFrame->FindWidget("textpane_yes"));
  yes->TextSupport().SetText(rstl::wstring_l(gpStringTable->GetString(22)));
  CGuiTextPane* no = static_cast< CGuiTextPane* >(mLoadedFrame->FindWidget("textpane_no"));
  no->TextSupport().SetText(rstl::wstring_l(gpStringTable->GetString(23)));
#endif
  const int defaults[] = {1, 0, 1, 1, 0};
  mTablegroup_quitgame->SetUserSelection(defaults[mType]);
  SetColors();
}

void CQuitGameScreen::SetColors() {
  const CColor selected(uchar(200), uchar(200), uchar(200), uchar(255));
  const CColor unselected(uchar(50), uchar(50), uchar(50), uchar(255));
  const int selection = mTablegroup_quitgame->GetUserSelection();
  for (int i = 0; i < 2; ++i) {
    CGuiWidget* worker = mTablegroup_quitgame->GetWorkerWidget(i);
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
    return options.mHudAlpha;
  case kGO_HelmetOpacity:
    return options.mHelmetAlpha;
  case kGO_HUDLag:
    return options.GetHUDLag() ? 1 : 0;
  case kGO_HintSystem:
    return options.GetIsHintSystemEnabled() ? 1 : 0;
  case kGO_ScreenBrightness:
    return options.mScreenBrightness;
  case kGO_ScreenOffsetX:
    return options.mScreenXOffset;
  case kGO_ScreenOffsetY:
    return options.mScreenYOffset;
  case kGO_ScreenStretch:
    return options.mScreenStretch;
  case kGO_SFXVolume:
    return options.mSfxVol;
  case kGO_MusicVolume:
    return options.mMusicVol;
  case kGO_SoundMode:
    return options.mSoundMode;
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
    options.mHudAlpha = value;
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
#if VERSION >= VERSION_GM8E_02
      options.SetHudAlpha(255);
#else
      options.mHudAlpha = 255;
#endif
      options.SetHelmetAlpha(255);
      options.SetHUDLag(skDefaultHudLag);
      options.SetIsHintSystemEnabled(skDefaultHintSystem);
#if VERSION >= VERSION_GM8E_02
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
, mQuitGame(nullptr)
, mGameCube(rs_new CGameCubeDoll())
, mOptionAlpha(0.f)
, mInOptionBody(false) {}

COptionsScreen::~COptionsScreen() { CSfxManager::SfxStop(mSliderSfx); }

bool COptionsScreen::VReady() const { return true; }

bool COptionsScreen::InputDisabled() const { return !mQuitGame.null(); }

void COptionsScreen::Update(float dt, CRandom16& rand, CArchitectureQueue& queue) {
  mRumble.Update(dt);
  CPauseScreenBase::Update(dt, rand, queue);
  const bool sliding = mSlidergroup_slider->GetState() != CGuiSliderGroup::kS_None;
  if (bool(mSliderSfx) != sliding) {
    if (sliding) {
      mSliderSfx = CSfxManager::SfxStart(0x5ab, 0x7f, 0x40, false, CSfxManager::kMedPriority,
                                             false, CSfxManager::kAllAreas);
    } else {
      CSfxManager::SfxStop(mSliderSfx);
      mSliderSfx.Clear();
    }
  }
  if (mInOptionBody) {
    mOptionAlpha = rstl::min_val(1.f, mOptionAlpha + 4.f * dt);
  } else {
    mOptionAlpha = rstl::max_val(0.f, mOptionAlpha - 4.f * dt);
  }
  if (close_enough(mOptionAlpha, 0.f)) {
    ResetOptionWidgetVisibility();
    mTextpane_body->SetIsVisible(false);
  }
  const CColor color =
      gpTweakGuiColors->GetPauseItemAmberColor().WithAlphaModulatedBy(mOptionAlpha);
  mSlidergroup_slider->SetColor(color);
  mTablegroup_double->SetColor(color);
  mTablegroup_triple->SetColor(color);
  if (!mQuitGame.null()) {
    const EQuitAction action = mQuitGame->Update(dt);
    if (action == kQA_Yes) {
      queue.Push(MakeMsg::CreateQuitGameplay(kAMT_Game));
      CSfxManager::SetChannel(CSfxManager::kSC_Default);
      CSfxManager::SfxStart(0x58e, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
    } else if (action == kQA_No) {
      CSfxManager::SfxStart(0x58f, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
      mQuitGame = nullptr;
    }
  }
  mGameCube->Update(dt);
}

void COptionsScreen::Touch() {
  CPauseScreenBase::Touch();
  mGameCube->Touch();
}

void COptionsScreen::ProcessInput(const CFinalInput& input) {
  if (mQuitGame.null()) {
    CPauseScreenBase::ProcessInput(input);
    CGameOptions::TryRestoreDefaults(input, mTablegroup_leftlog->GetUserSelection(),
                                     mRightSel, false);
    if (mTablegroup_leftlog->GetUserSelection() == 4 && input.PA()) {
      mQuitGame = rs_new CQuitGameScreen(kQT_QuitGame);
    }
  } else {
    mQuitGame->ProcessUserInput(input);
  }
}

void COptionsScreen::Draw(float transInterp, float totalAlpha, float yOff) const {
  CPauseScreenBase::Draw(transInterp, totalAlpha, yOff);
  mGameCube->Draw(transInterp * (1.f - mOptionAlpha));
  if (!mQuitGame.null()) {
    CGraphics::SetDepthRange(0.f, 0.001f);
    mQuitGame->Draw();
    CGraphics::SetDepthRange(0.f, 1.f);
  }
}

void COptionsScreen::VActivate() {
  for (int i = 0; i < 5; ++i) {
    CGuiTextPane* pane = mTextpane_categories[i];
    pane->TextSupport().SetText(rstl::wstring(mPauseStrg.GetString(i + 16)));
  }
  mTextpane_title->TextSupport().SetText(rstl::wstring(mPauseStrg.GetString(15)));
  for (int i = ARRAY_SIZE(skGameOptions); i < mTextpane_categories.capacity(); ++i) {
    mTablegroup_leftlog->GetWorkerWidget(i)->SetIsSelectable(false);
  }
  mTextpane_body->TextSupport().SetJustification(kJustification_Center);
  mTextpane_body->TextSupport().SetVerticalJustification(kVerticalJustification_Bottom);
  CGuiTextPane* off = static_cast< CGuiTextPane* >(mTablegroup_double->GetWorkerWidget(0));
  off->TextSupport().SetText(rstl::wstring_l(mPauseStrg.GetString(95)));
  CGuiTextPane* on = static_cast< CGuiTextPane* >(mTablegroup_double->GetWorkerWidget(1));
  on->TextSupport().SetText(rstl::wstring_l(mPauseStrg.GetString(94)));
  CGuiTextPane* mono = static_cast< CGuiTextPane* >(mTablegroup_triple->GetWorkerWidget(0));
  mono->TextSupport().SetText(rstl::wstring_l(mPauseStrg.GetString(96)));
  CGuiTextPane* stereo = static_cast< CGuiTextPane* >(mTablegroup_triple->GetWorkerWidget(1));
  stereo->TextSupport().SetText(rstl::wstring_l(mPauseStrg.GetString(97)));
  CGuiTextPane* surround = static_cast< CGuiTextPane* >(mTablegroup_triple->GetWorkerWidget(2));
  surround->TextSupport().SetText(rstl::wstring_l(mPauseStrg.GetString(98)));
  mSlidergroup_slider->SetSelectionChangedCallback(
      TFunctor2FromMethod< COptionsScreen, CGuiSliderGroup* const, const float >::Make(
          *this, &COptionsScreen::OnSliderChanged));
  mTablegroup_double->SetMenuSelectionChangeCallback(
      TFunctor2FromMethod< COptionsScreen, CGuiTableGroup* const, const int >::Make(
          *this, &COptionsScreen::OnEnumChanged));
  mTablegroup_triple->SetMenuSelectionChangeCallback(
      TFunctor2FromMethod< COptionsScreen, CGuiTableGroup* const, const int >::Make(
          *this, &COptionsScreen::OnEnumChanged));
}

bool COptionsScreen::ShouldRightTableAdvance() { return false; }

bool COptionsScreen::ShouldLeftTableAdvance() {
  return mTablegroup_leftlog->GetUserSelection() != 4;
}

uint COptionsScreen::GetRightTableCount() const {
  return skGameOptions[mTablegroup_leftlog->GetUserSelection()].count;
}

void COptionsScreen::UpdateRightTable() {
  CPauseScreenBase::UpdateRightTable();
  const SOptionCategory& category = skGameOptions[mTablegroup_leftlog->GetUserSelection()];
  for (int i = 0; i < 5; ++i) {
    CGuiTextPane* pane = mTextpane_titles[i];
    if (i < category.count) {
      pane->TextSupport().SetText(
          rstl::wstring(mPauseStrg.GetString(category.options[i].stringId)));
    } else {
      pane->TextSupport().SetText(rstl::wstring_l(L""));
    }
  }
}

void COptionsScreen::ChangedMode(EMode oldMode) {
  if (mMode == kM_RightTable) {
    mTextpane_body->SetIsVisible(true);
    UpdateOptionView();
    mInOptionBody = true;
  } else {
    mInOptionBody = false;
  }
}

void COptionsScreen::RightTableSelectionChanged(int oldSel, int newSel) { UpdateOptionView(); }

void COptionsScreen::ResetOptionWidgetVisibility() {
  mSlidergroup_slider->SetIsActive(false);
  mSlidergroup_slider->SetVisibility(false, kTM_Children);
  mTablegroup_double->SetIsVisible(false);
  mTablegroup_double->SetIsActive(false);
  mTablegroup_triple->SetIsActive(false);
  mTablegroup_triple->SetIsVisible(false);
}

void COptionsScreen::UpdateOptionView() {
  ResetOptionWidgetVisibility();
  const SOptionCategory& category = skGameOptions[mTablegroup_leftlog->GetUserSelection()];
  if (category.count == 0) {
    return;
  }
  const SGameOption& opt = category.options[mRightSel];
  const float zOff = mHighlightPitch * mRightSel;
  switch (opt.type) {
  case kOT_Float:
    mSlidergroup_slider->SetIsActive(true);
    mSlidergroup_slider->SetVisibility(true, kTM_Children);
    mSlidergroup_slider->SetMinVal(opt.minVal);
    mSlidergroup_slider->SetMaxVal(opt.maxVal);
    mSlidergroup_slider->SetIncrement(opt.increment);
    mSlidergroup_slider->SetCurVal(CGameOptions::GetOption(opt.option));
    mSlidergroup_slider->SetLocalPosition(mSliderStart + CVector3f(0.f, 0.f, zOff));
    break;
  case kOT_DoubleEnum:
    mTablegroup_double->SetUserSelection(CGameOptions::GetOption(opt.option));
    mTablegroup_double->SetIsVisible(true);
    mTablegroup_double->SetIsActive(true);
    UpdateSideTable(mTablegroup_double);
    mTablegroup_double->SetLocalPosition(mTableDoubleStart + CVector3f(0.f, 0.f, zOff));
    break;
  case kOT_TripleEnum:
    mTablegroup_triple->SetUserSelection(CGameOptions::GetOption(opt.option));
    mTablegroup_triple->SetIsVisible(true);
    mTablegroup_triple->SetIsActive(true);
    UpdateSideTable(mTablegroup_triple);
    mTablegroup_triple->SetLocalPosition(mTableTripleStart + CVector3f(0.f, 0.f, zOff));
    break;
  case kOT_RestoreDefaults:
    break;
  default:
    break;
  }
}

void COptionsScreen::OnSliderChanged(CGuiSliderGroup* caller, float value) {
  if (mMode == kM_RightTable) {
    const SOptionCategory& category = skGameOptions[mTablegroup_leftlog->GetUserSelection()];
    const EGameOption option = category.options[mRightSel].option;
    CGameOptions::SetOption(option, caller->GetCurVal());
  }
}

void COptionsScreen::OnEnumChanged(CGuiTableGroup* caller, int oldSel) {
  if (mMode == kM_RightTable) {
    const SOptionCategory& category = skGameOptions[mTablegroup_leftlog->GetUserSelection()];
    const SGameOption& option = category.options[mRightSel];
    const int selection = caller->GetUserSelection();
    CGameOptions::SetOption(option.option, selection);
    if (option.option == kGO_Rumble && selection > 0) {
      mRumble.HardStopAll();
      mRumble.Rumble(skRumbleFxTable[kRFX_PlayerBump], 1.f, kRP_One, kIOP_Player1);
    }
    CPauseScreenBase::UpdateSideTable(caller);
    CSfxManager::SfxStart(0x59d, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
  }
}

SOptionsFrontEndFrame::SOptionsFrontEndFrame()
: mUiAlpha(0.f)
, mFrme(gpSimplePool->GetObj("FRME_OptionsFrontEnd"))
, mPauseScreen(gpSimplePool->GetObj("STRG_PauseScreen"))
, mLoadedFrame(nullptr)
, mLoadedPauseStrg(nullptr)
, mTablegroup_leftmenu(nullptr)
, mTablegroup_rightmenu(nullptr)
, mTablegroup_double(nullptr)
, mTablegroup_triple(nullptr)
, mSlidergroup_slider(nullptr)
, mRowPitch(0.f)
, mVisible(true)
, mExitOptions(false) {
  mFrme.Lock();
  mPauseScreen.Lock();
}

SOptionsFrontEndFrame::~SOptionsFrontEndFrame() { CSfxManager::SfxStop(mSliderSfx); }

bool SOptionsFrontEndFrame::PumpLoad() {
  if (mLoadedFrame != nullptr) {
    return true;
  }
  if (mFrme.TryCache() && mPauseScreen.TryCache()) {
    CGuiFrame* frame = mFrme.GetObject();
    if (frame->GetIsFinishedLoading()) {
      mLoadedFrame = frame;
      mLoadedPauseStrg = mPauseScreen.GetObject();
      FinishedLoading();
      return true;
    }
  }
  return false;
}

void SOptionsFrontEndFrame::FinishedLoading() {
  mTablegroup_leftmenu =
      static_cast< CGuiTableGroup* >(mLoadedFrame->FindWidget("tablegroup_leftmenu"));
  mTablegroup_rightmenu =
      static_cast< CGuiTableGroup* >(mLoadedFrame->FindWidget("tablegroup_rightmenu"));
  mTablegroup_double =
      static_cast< CGuiTableGroup* >(mLoadedFrame->FindWidget("tablegroup_double"));
  mTablegroup_triple =
      static_cast< CGuiTableGroup* >(mLoadedFrame->FindWidget("tablegroup_triple"));
  mSlidergroup_slider =
      static_cast< CGuiSliderGroup* >(mLoadedFrame->FindWidget("slidergroup_slider"));
  mTablegroup_leftmenu->SetMenuAdvanceCallback(
      TFunctor1FromMethod< SOptionsFrontEndFrame, CGuiTableGroup* const >::Make(
          *this, &SOptionsFrontEndFrame::DoLeftMenuAdvance));
  mTablegroup_leftmenu->SetMenuSelectionChangeCallback(
      TFunctor2FromMethod< SOptionsFrontEndFrame, CGuiTableGroup* const, const int >::Make(
          *this, &SOptionsFrontEndFrame::DoMenuSelectionChange));
  mRowPitch = mTablegroup_leftmenu->GetWorkerWidget(1)->GetIdlePosition().GetZ() -
                 mTablegroup_leftmenu->GetWorkerWidget(0)->GetIdlePosition().GetZ();
  mTablegroup_rightmenu->SetMenuSelectionChangeCallback(
      TFunctor2FromMethod< SOptionsFrontEndFrame, CGuiTableGroup* const, const int >::Make(
          *this, &SOptionsFrontEndFrame::DoMenuSelectionChange));
  mTablegroup_rightmenu->SetMenuCancelCallback(
      TFunctor1FromMethod< SOptionsFrontEndFrame, CGuiTableGroup* const >::Make(
          *this, &SOptionsFrontEndFrame::DoMenuCancel));
  mTablegroup_double->SetMenuSelectionChangeCallback(
      TFunctor2FromMethod< SOptionsFrontEndFrame, CGuiTableGroup* const, const int >::Make(
          *this, &SOptionsFrontEndFrame::DoMenuSelectionChange));
  mTablegroup_double->SetMenuCancelCallback(
      TFunctor1FromMethod< SOptionsFrontEndFrame, CGuiTableGroup* const >::Make(
          *this, &SOptionsFrontEndFrame::DoMenuCancel));
  mTablegroup_triple->SetMenuSelectionChangeCallback(
      TFunctor2FromMethod< SOptionsFrontEndFrame, CGuiTableGroup* const, const int >::Make(
          *this, &SOptionsFrontEndFrame::DoMenuSelectionChange));
  mTablegroup_triple->SetMenuCancelCallback(
      TFunctor1FromMethod< SOptionsFrontEndFrame, CGuiTableGroup* const >::Make(
          *this, &SOptionsFrontEndFrame::DoMenuCancel));
  mSlidergroup_slider->SetSelectionChangedCallback(
      TFunctor2FromMethod< SOptionsFrontEndFrame, CGuiSliderGroup* const, const float >::Make(
          *this, &SOptionsFrontEndFrame::DoSliderChange));

  SetTextPanePair(mLoadedFrame, "textpane_double0", mLoadedPauseStrg->GetString(95));
  SetTextPanePair(mLoadedFrame, "textpane_double1", mLoadedPauseStrg->GetString(94));
  SetTextPanePair(mLoadedFrame, "textpane_triple0", mLoadedPauseStrg->GetString(96));
  SetTextPanePair(mLoadedFrame, "textpane_triple1", mLoadedPauseStrg->GetString(97));
  SetTextPanePair(mLoadedFrame, "textpane_triple2", mLoadedPauseStrg->GetString(98));
  SetTextPanePair(mLoadedFrame, "textpane_title", gpStringTable->GetString(99));
  if (CGuiTextPane* proceed =
          static_cast< CGuiTextPane* >(mLoadedFrame->FindWidget("textpane_proceed"))) {
    proceed->TextSupport().SetText(rstl::wstring_l(gpStringTable->GetString(85)));
  }
  if (CGuiTextPane* cancel =
          static_cast< CGuiTextPane* >(mLoadedFrame->FindWidget("textpane_cancel"))) {
    cancel->TextSupport().SetText(rstl::wstring_l(gpStringTable->GetString(82)));
  }
  for (int i = 0; i < 4; ++i) {
    char name[32];
#if NONMATCHING
    snprintf(name, sizeof(name), "textpane_filename%d", i);
#else
    sprintf(name, "textpane_filename%d", i);
#endif
    SetTextPanePair(mLoadedFrame, name, mLoadedPauseStrg->GetString(16 + i));
  }
  mTablegroup_double->SetVertical(false);
  mTablegroup_triple->SetVertical(false);
  mTablegroup_leftmenu->SetIsActive(true);
  mTablegroup_rightmenu->SetIsActive(false);
  SetTableColors(mTablegroup_leftmenu);
  SetTableColors(mTablegroup_rightmenu);
  SetTableColors(mTablegroup_double);
  SetTableColors(mTablegroup_triple);
  SetRightUIText();
  DeactivateRightMenu();
}

void SOptionsFrontEndFrame::SetRightUIText() {
  const SOptionCategory& options = skGameOptions[mTablegroup_leftmenu->GetUserSelection()];
  for (int i = 0; i < 5; ++i) {
    char name[32];
#if defined(TARGET_PC)
    snprintf(name, sizeof(name), "textpane_right%d", i);
#else
    sprintf(name, "textpane_right%d", i);
#endif
    if (i < options.count) {
      SetTextPanePair(mLoadedFrame, name,
                      mLoadedPauseStrg->GetString(options.options[i].stringId));
      mTablegroup_rightmenu->GetWorkerWidget(i)->SetIsSelectable(true);
    } else {
      SetTextPanePair(mLoadedFrame, name, L"");
      mTablegroup_rightmenu->GetWorkerWidget(i)->SetIsSelectable(false);
    }
  }
}

void SOptionsFrontEndFrame::HandleRightSelectionChange() {
  DeactivateRightMenu();
  const SOptionCategory& category = skGameOptions[mTablegroup_leftmenu->GetUserSelection()];
  const SGameOption& option = category.options[mTablegroup_rightmenu->GetUserSelection()];
  switch (option.type) {
  case kOT_Float:
    mSlidergroup_slider->SetIsActive(true);
    mSlidergroup_slider->SetVisibility(true, kTM_Children);
    mSlidergroup_slider->SetMinVal(option.minVal);
    mSlidergroup_slider->SetMaxVal(option.maxVal);
    mSlidergroup_slider->SetIncrement(option.increment);
    mSlidergroup_slider->SetCurVal(CGameOptions::GetOption(option.option));
    mSlidergroup_slider->SetO2PTransform(
        CTransform4f::Translate(0.f, 0.f,
                                mTablegroup_rightmenu->GetUserSelection() * mRowPitch) *
        mSlidergroup_slider->GetTransform());
    break;
  case kOT_DoubleEnum:
    mTablegroup_double->SetUserSelection(CGameOptions::GetOption(option.option));
    mTablegroup_double->SetIsVisible(true);
    mTablegroup_double->SetIsActive(true);
    mTablegroup_double->SetO2PTransform(
        CTransform4f::Translate(0.f, 0.f,
                                mTablegroup_rightmenu->GetUserSelection() * mRowPitch) *
        mTablegroup_double->GetTransform());
    SetTableColors(mTablegroup_double);
    break;
  case kOT_TripleEnum:
    mTablegroup_triple->SetUserSelection(CGameOptions::GetOption(option.option));
    mTablegroup_triple->SetIsVisible(true);
    mTablegroup_triple->SetIsActive(true);
    mTablegroup_triple->SetO2PTransform(
        CTransform4f::Translate(0.f, 0.f,
                                mTablegroup_rightmenu->GetUserSelection() * mRowPitch) *
        mTablegroup_triple->GetTransform());
    SetTableColors(mTablegroup_triple);
    break;
  default:
    break;
  }
}

void SOptionsFrontEndFrame::DeactivateRightMenu() {
  mTablegroup_double->SetIsActive(false);
  mTablegroup_triple->SetIsActive(false);
  mSlidergroup_slider->SetIsActive(false);
  mTablegroup_double->SetVisibility(false, kTM_Children);
  mTablegroup_triple->SetVisibility(false, kTM_Children);
  mSlidergroup_slider->SetVisibility(false, kTM_Children);
}

void SOptionsFrontEndFrame::DoLeftMenuAdvance(CGuiTableGroup* caller) {
  if (caller == mTablegroup_leftmenu) {
    HandleRightSelectionChange();
    mTablegroup_rightmenu->SetUserSelection(0);
    mTablegroup_leftmenu->SetIsActive(false);
    mTablegroup_rightmenu->SetIsActive(true);
    CSfxManager::SfxStart(0x448, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
    CSfxManager::SfxStart(0x443, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
  }
}

void SOptionsFrontEndFrame::DoMenuSelectionChange(CGuiTableGroup* caller, int oldSel) {
  SetTableColors(caller);
  if (caller == mTablegroup_leftmenu) {
    SetRightUIText();
    CSfxManager::SfxStart(0x445, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
  } else if (caller == mTablegroup_rightmenu) {
    HandleRightSelectionChange();
    CSfxManager::SfxStart(0x445, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
  } else if (caller == mTablegroup_double || caller == mTablegroup_triple) {
    if (mTablegroup_rightmenu->GetIsActive()) {
      const SOptionCategory& category = skGameOptions[mTablegroup_leftmenu->GetUserSelection()];
      const SGameOption& option = category.options[mTablegroup_rightmenu->GetUserSelection()];
      const int selection = caller->GetUserSelection();
      CGameOptions::SetOption(option.option, selection);
      CSfxManager::SfxStart(0x447, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
      if (option.option == kGO_Rumble && selection > 0) {
        mRumbleGen.HardStopAll();
        mRumbleGen.Rumble(skRumbleFxTable[kRFX_PlayerBump], 1.f, kRP_One, kIOP_Player1);
      }
    }
  }
}

void SOptionsFrontEndFrame::DoMenuCancel(CGuiTableGroup* caller) {
  if (caller == mTablegroup_rightmenu) {
    DeactivateRightMenu();
    mTablegroup_leftmenu->SetIsActive(true);
    mTablegroup_rightmenu->SetIsActive(false);
    mTablegroup_rightmenu->SetUserSelection(0);
    SetTableColors(mTablegroup_rightmenu);
    CSfxManager::SfxStart(0x446, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
  }
}

void SOptionsFrontEndFrame::Update(float dt, const CSaveGameScreen* saveUI) {
  mRumbleGen.Update(dt);
  mVisible = saveUI == nullptr || saveUI->GetUIType() == CSaveGameScreen::kUIT_SaveReady;
  if (!PumpLoad()) {
    return;
  }
  mUiAlpha = rstl::min_val(1.f, mUiAlpha + dt);
  mLoadedFrame->Update(dt);
  const bool sliding = mSlidergroup_slider->GetState() != CGuiSliderGroup::kS_None;
  if (bool(mSliderSfx) != sliding) {
    if (sliding) {
      mSliderSfx = CSfxManager::SfxStart(0x5b2, 0x7f, 0x40, false, CSfxManager::kMedPriority,
                                            false, CSfxManager::kAllAreas);
    } else {
      CSfxManager::SfxStop(mSliderSfx);
      mSliderSfx.Clear();
    }
  }
}

bool SOptionsFrontEndFrame::ProcessUserInput(const CFinalInput& input, CSaveGameScreen* saveUI) {
  mExitOptions = false;
  if (saveUI != nullptr) {
    saveUI->ProcessUserInput(input);
  }
  if (mLoadedFrame != nullptr && mVisible) {
    if (input.PB() && mTablegroup_leftmenu->GetIsActive()) {
      mExitOptions = true;
      CSfxManager::SfxStart(0x446, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
    } else {
      mLoadedFrame->ProcessUserInput(input);
      CGameOptions::TryRestoreDefaults(input, mTablegroup_leftmenu->GetUserSelection(),
                                       mTablegroup_rightmenu->GetUserSelection(), true);
    }
  }
  return !mExitOptions;
}

void SOptionsFrontEndFrame::Draw() const {
  if (mLoadedFrame != nullptr && mVisible) {
    mLoadedFrame->Draw(CGuiWidgetDrawParms(mUiAlpha, CVector3f::Zero()));
  }
}

void SOptionsFrontEndFrame::SetTableColors(CGuiTableGroup* table) const {
  const CColor selected(uchar(255), uchar(255), uchar(255), uchar(255));
  const CColor unselected(uchar(160), uchar(160), uchar(160), uchar(200));
  table->SetColors(selected, unselected);
}

void SOptionsFrontEndFrame::DoSliderChange(CGuiSliderGroup* caller, float value) {
  if (mTablegroup_rightmenu->GetIsActive()) {
    const SOptionCategory& category = skGameOptions[mTablegroup_leftmenu->GetUserSelection()];
    const EGameOption option =
        category.options[mTablegroup_rightmenu->GetUserSelection()].option;
    CGameOptions::SetOption(option, caller->GetCurVal());
  }
}
