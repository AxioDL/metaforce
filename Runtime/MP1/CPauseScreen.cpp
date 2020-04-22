#include "Runtime/MP1/CPauseScreen.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Audio/CSfxManager.hpp"
#include "Runtime/GuiSys/CGuiSys.hpp"
#include "Runtime/GuiSys/CGuiTextPane.hpp"
#include "Runtime/GuiSys/CGuiWidgetDrawParms.hpp"
#include "Runtime/Input/ControlMapper.hpp"
#include "Runtime/MP1/CLogBookScreen.hpp"
#include "Runtime/MP1/COptionsScreen.hpp"

namespace urde::MP1 {

CPauseScreen::CPauseScreen(ESubScreen subscreen, const CDependencyGroup& suitDgrp, const CDependencyGroup& ballDgrp)
: x0_initialSubScreen(subscreen)
, x14_strgPauseScreen(g_SimplePool->GetObj("STRG_PauseScreen"))
, x20_suitDgrp(suitDgrp)
, x24_ballDgrp(ballDgrp)
, x28_pauseScreenInstructions(g_SimplePool->GetObj("FRME_PauseScreenInstructions"))
, x54_frmePauseScreenId(g_ResFactory->GetResourceIdByName("FRME_PauseScreen")->id) {
  SObjectTag frmeTag(FOURCC('FRME'), x54_frmePauseScreenId);
  x58_frmePauseScreenBufSz = g_ResFactory->ResourceSize(frmeTag);
  x5c_frmePauseScreenBuf.reset(new u8[x58_frmePauseScreenBufSz]);
  x60_loadTok = g_ResFactory->LoadResourceAsync(frmeTag, x5c_frmePauseScreenBuf.get());
  CSfxManager::SfxStart(SFXui_pause_screen_enter, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
  x7c_screens.resize(2);
}

CPauseScreen::~CPauseScreen() {
  if (x60_loadTok)
    x60_loadTok->PostCancelRequest();
}

std::unique_ptr<CPauseScreenBase> CPauseScreen::BuildPauseSubScreen(ESubScreen subscreen, const CStateManager& mgr,
                                                                    CGuiFrame& frame) const {
  switch (subscreen) {
  case ESubScreen::LogBook:
    return std::make_unique<CLogBookScreen>(mgr, frame, *x14_strgPauseScreen);
  case ESubScreen::Options:
    return std::make_unique<COptionsScreen>(mgr, frame, *x14_strgPauseScreen);
  case ESubScreen::Inventory:
    return std::make_unique<CInventoryScreen>(mgr, frame, *x14_strgPauseScreen, x20_suitDgrp, x24_ballDgrp);
  default:
    return {};
  }
}

void CPauseScreen::InitializeFrameGlue() {
  x38_textpane_l1 = static_cast<CGuiTextPane*>(x34_loadedPauseScreenInstructions->FindWidget("textpane_l1"));
  x38_textpane_l1->SetMouseActive(true);
  x3c_textpane_r = static_cast<CGuiTextPane*>(x34_loadedPauseScreenInstructions->FindWidget("textpane_r"));
  x3c_textpane_r->SetMouseActive(true);
  x40_textpane_a = static_cast<CGuiTextPane*>(x34_loadedPauseScreenInstructions->FindWidget("textpane_a"));
  x40_textpane_a->SetMouseActive(true);
  x44_textpane_b = static_cast<CGuiTextPane*>(x34_loadedPauseScreenInstructions->FindWidget("textpane_b"));
  x44_textpane_b->SetMouseActive(true);
  x48_textpane_return = static_cast<CGuiTextPane*>(x34_loadedPauseScreenInstructions->FindWidget("textpane_return"));
  x48_textpane_return->SetMouseActive(true);
  x4c_textpane_next = static_cast<CGuiTextPane*>(x34_loadedPauseScreenInstructions->FindWidget("textpane_next"));
  x4c_textpane_next->SetMouseActive(true);
  x50_textpane_back = static_cast<CGuiTextPane*>(x34_loadedPauseScreenInstructions->FindWidget("textpane_back"));
  x50_textpane_back->SetMouseActive(true);

  x40_textpane_a->TextSupport().SetText(x14_strgPauseScreen->GetString(7)); // OPTIONS
  x40_textpane_a->TextSupport().SetFontColor(g_tweakGuiColors->GetPauseItemAmberColor());
  x44_textpane_b->TextSupport().SetText(x14_strgPauseScreen->GetString(6)); // LOG BOOK
  x44_textpane_b->TextSupport().SetFontColor(g_tweakGuiColors->GetPauseItemAmberColor());
  x40_textpane_a->SetColor(zeus::skClear);
  x44_textpane_b->SetColor(zeus::skClear);

  if (CGuiWidget* deco = x34_loadedPauseScreenInstructions->FindWidget("basewidget_deco")) {
    zeus::CColor color = g_tweakGuiColors->GetPauseItemAmberColor();
    color.a() *= 0.75f;
    deco->SetColor(color);
  }

  x34_loadedPauseScreenInstructions->SetMouseDownCallback(
      [this](CGuiWidget* caller, bool resume) { OnWidgetMouseDown(caller, resume); });
  x34_loadedPauseScreenInstructions->SetMouseUpCallback(
      [this](CGuiWidget* caller, bool cancel) { OnWidgetMouseUp(caller, cancel); });
}

bool CPauseScreen::CheckLoadComplete(const CStateManager& mgr) {
  if (x90_resourcesLoaded)
    return true;
  if (!x14_strgPauseScreen.IsLoaded())
    return false;
  if (!x34_loadedPauseScreenInstructions) {
    if (!x28_pauseScreenInstructions.IsLoaded())
      return false;
    if (!x28_pauseScreenInstructions->GetIsFinishedLoading())
      return false;
    x34_loadedPauseScreenInstructions = x28_pauseScreenInstructions.GetObj();
    x34_loadedPauseScreenInstructions->SetMaxAspect(1.77f);
    InitializeFrameGlue();
  }
  if (x60_loadTok) {
    if (!x60_loadTok->IsComplete())
      return false;
    for (int i = 0; i < 2; ++i) {
      CMemoryInStream s(x5c_frmePauseScreenBuf.get(), x58_frmePauseScreenBufSz);
      x64_frameInsts.push_back(CGuiFrame::CreateFrame(x54_frmePauseScreenId, *g_GuiSys, s, g_SimplePool));
      x64_frameInsts.back()->SetMaxAspect(1.77f);
    }
    x5c_frmePauseScreenBuf.reset();
    x60_loadTok.reset();
  }
  if (!x64_frameInsts[0]->GetIsFinishedLoading() || !x64_frameInsts[1]->GetIsFinishedLoading())
    return false;
  x90_resourcesLoaded = true;
  StartTransition(FLT_EPSILON, mgr, x0_initialSubScreen, 2);
  x91_initialTransition = true;
  return true;
}

void CPauseScreen::StartTransition(float time, const CStateManager& mgr, ESubScreen subscreen, int b) {
  if (subscreen == xc_nextSubscreen)
    return;
  xc_nextSubscreen = subscreen;
  x4_ = b;
  x10_alphaInterp = time;
  std::unique_ptr<CPauseScreenBase>& newScreenSlot = x7c_screens[1 - x78_activeIdx];
  std::unique_ptr<CGuiFrame>& newScreenInst = x64_frameInsts[1 - x78_activeIdx];
  newScreenSlot = BuildPauseSubScreen(xc_nextSubscreen, mgr, *newScreenInst);
  if (x7c_screens[x78_activeIdx])
    x7c_screens[x78_activeIdx]->TransitioningAway();
  x91_initialTransition = false;
}

bool CPauseScreen::InputEnabled() const {
  if (xc_nextSubscreen != x8_curSubscreen)
    return false;

  if (const std::unique_ptr<CPauseScreenBase>& screenSlot = x7c_screens[x78_activeIdx])
    if (screenSlot->InputDisabled())
      return false;

  if (const std::unique_ptr<CPauseScreenBase>& screenSlot = x7c_screens[1 - x78_activeIdx])
    if (screenSlot->InputDisabled())
      return false;

  return true;
}

CPauseScreen::ESubScreen CPauseScreen::GetPreviousSubscreen(ESubScreen screen) {
  switch (screen) {
  case ESubScreen::Inventory:
    return ESubScreen::Options;
  case ESubScreen::Options:
    return ESubScreen::LogBook;
  case ESubScreen::LogBook:
    return ESubScreen::Inventory;
  default:
    return ESubScreen::ToGame;
  }
}

CPauseScreen::ESubScreen CPauseScreen::GetNextSubscreen(ESubScreen screen) {
  switch (screen) {
  case ESubScreen::Inventory:
    return ESubScreen::LogBook;
  case ESubScreen::Options:
    return ESubScreen::Inventory;
  case ESubScreen::LogBook:
    return ESubScreen::Options;
  default:
    return ESubScreen::ToGame;
  }
}

void CPauseScreen::ProcessControllerInput(const CStateManager& mgr, const CFinalInput& input) {
  if (!IsLoaded())
    return;

  if (x8_curSubscreen == ESubScreen::ToGame)
    return;

  m_returnClicked = false;
  m_nextClicked = false;
  m_backClicked = false;
  m_lClicked = false;
  m_rClicked = false;

  CFinalInput useInput = input;

  bool bExits = false;
  if (std::unique_ptr<CPauseScreenBase>& curScreen = x7c_screens[x78_activeIdx]) {
    float yOff = 0.f;
    if (curScreen->CanDraw())
      yOff = curScreen->GetCameraYBias();
    CGuiWidgetDrawParms parms(1.f, zeus::CVector3f{0.f, 15.f * yOff, 0.f});
    x34_loadedPauseScreenInstructions->ProcessMouseInput(useInput, parms);
    useInput.x2e_b31_PStart |= m_returnClicked;
    useInput.x2d_b28_PA |= m_nextClicked;
    useInput.x2d_b29_PB |= m_backClicked;

    if (curScreen->GetMode() == CPauseScreenBase::EMode::LeftTable)
      bExits = true;
    curScreen->ProcessControllerInput(useInput);
  }

  if (InputEnabled()) {
    bool invalid = x8_curSubscreen == ESubScreen::ToGame;
    if (useInput.PStart() ||
        ((useInput.PB() || useInput.PSpecialKey(boo::ESpecialKey::Esc)) && bExits) ||
        (x7c_screens[x78_activeIdx] && x7c_screens[x78_activeIdx]->ShouldExitPauseScreen())) {
      CSfxManager::SfxStart(SFXui_pause_screen_exit, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
      StartTransition(0.5f, mgr, ESubScreen::ToGame, 2);
    } else {
      if (ControlMapper::GetPressInput(ControlMapper::ECommands::PreviousPauseScreen, useInput) || m_lClicked) {
        CSfxManager::SfxStart(SFXui_pause_screen_change, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        StartTransition(0.5f, mgr, GetPreviousSubscreen(x8_curSubscreen), invalid ? 2 : 0);
      } else if (ControlMapper::GetPressInput(ControlMapper::ECommands::NextPauseScreen, useInput) || m_rClicked) {
        CSfxManager::SfxStart(SFXui_pause_screen_change, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        StartTransition(0.5f, mgr, GetNextSubscreen(x8_curSubscreen), invalid ? 2 : 1);
      }
    }
  }

  x38_textpane_l1->TextSupport().SetText(
    fmt::format(FMT_STRING("&image={};"), g_tweakPlayerRes->x74_lTrigger[
        ControlMapper::GetDigitalInput(ControlMapper::ECommands::PreviousPauseScreen, useInput) || m_lDown]));
  x3c_textpane_r->TextSupport().SetText(
    fmt::format(FMT_STRING("&image={};"), g_tweakPlayerRes->x80_rTrigger[
        ControlMapper::GetDigitalInput(ControlMapper::ECommands::NextPauseScreen, useInput) || m_rDown]));
  x48_textpane_return->TextSupport().SetText(
    fmt::format(FMT_STRING("&image={};"), g_tweakPlayerRes->x8c_startButton[useInput.DStart() || m_returnDown]));
  x50_textpane_back->TextSupport().SetText(
    fmt::format(FMT_STRING("&image={};"), g_tweakPlayerRes->x98_aButton[useInput.DA() || m_backDown]));
  x4c_textpane_next->TextSupport().SetText(
    fmt::format(FMT_STRING("&image={};"), g_tweakPlayerRes->xa4_bButton[useInput.DB() || m_nextDown]));
}

void CPauseScreen::TransitionComplete() {
  std::unique_ptr<CPauseScreenBase>& curScreen = x7c_screens[x78_activeIdx];
  curScreen.reset();
  x78_activeIdx = 1 - x78_activeIdx;
  x8_curSubscreen = xc_nextSubscreen;
  x40_textpane_a->TextSupport().SetText(x14_strgPauseScreen->GetString(int(GetPreviousSubscreen(x8_curSubscreen)) + 6));
  x44_textpane_b->TextSupport().SetText(x14_strgPauseScreen->GetString(int(GetNextSubscreen(x8_curSubscreen)) + 6));
}

void CPauseScreen::OnWidgetMouseDown(CGuiWidget* widget, bool resume) {
  if (widget == x48_textpane_return)
    m_returnDown = true;
  else if (widget == x4c_textpane_next)
    m_backDown = true;
  else if (widget == x50_textpane_back)
    m_nextDown = true;
  else if (widget == x38_textpane_l1 || widget == x40_textpane_a)
    m_lDown = true;
  else if (widget == x3c_textpane_r || widget == x44_textpane_b)
    m_rDown = true;
}

void CPauseScreen::OnWidgetMouseUp(CGuiWidget* widget, bool cancel) {
  if (widget == x48_textpane_return)
    m_returnDown = false;
  else if (widget == x4c_textpane_next)
    m_backDown = false;
  else if (widget == x50_textpane_back)
    m_nextDown = false;
  else if (widget == x38_textpane_l1 || widget == x40_textpane_a)
    m_lDown = false;
  else if (widget == x3c_textpane_r || widget == x44_textpane_b)
    m_rDown = false;
  if (cancel)
    return;
  if (widget == x48_textpane_return)
    m_returnClicked = true;
  else if (widget == x4c_textpane_next)
    m_backClicked = true;
  else if (widget == x50_textpane_back)
    m_nextClicked = true;
  else if (widget == x38_textpane_l1 || widget == x40_textpane_a)
    m_lClicked = true;
  else if (widget == x3c_textpane_r || widget == x44_textpane_b)
    m_rClicked = true;
}

void CPauseScreen::Update(float dt, const CStateManager& mgr, CRandom16& rand, CArchitectureQueue& archQueue) {
  if (!CheckLoadComplete(mgr))
    return;

  std::unique_ptr<CPauseScreenBase>& curScreen = x7c_screens[x78_activeIdx];
  std::unique_ptr<CPauseScreenBase>& otherScreen = x7c_screens[1 - x78_activeIdx];

  if (x8_curSubscreen != xc_nextSubscreen) {
    x10_alphaInterp = std::max(0.f, x10_alphaInterp - dt);
    if (!curScreen || !curScreen->InputDisabled()) {
      if (!otherScreen || otherScreen->IsReady()) {
        if (x10_alphaInterp == 0.f)
          TransitionComplete();
      }
    }
  }

  if (std::unique_ptr<CPauseScreenBase>& curScreen = x7c_screens[x78_activeIdx]) {
    curScreen->Update(dt, rand, archQueue);
    zeus::CColor color = zeus::skWhite;
    color.a() = std::min(curScreen->GetAlpha(), x8_curSubscreen != xc_nextSubscreen ? x10_alphaInterp / 0.5f : 1.f);
    x40_textpane_a->SetColor(color);
    x44_textpane_b->SetColor(color);
  }
}

void CPauseScreen::PreDraw() {
  if (!IsLoaded())
    return;
  if (std::unique_ptr<CPauseScreenBase>& curScreen = x7c_screens[x78_activeIdx])
    if (curScreen->CanDraw())
      curScreen->Touch();
}

void CPauseScreen::Draw() {
  if (!IsLoaded())
    return;
  SCOPED_GRAPHICS_DEBUG_GROUP("CPauseScreen::Draw", zeus::skPurple);

  float totalAlpha = 0.f;
  float yOff = 0.f;
  std::unique_ptr<CPauseScreenBase>& curScreen = x7c_screens[x78_activeIdx];
  if (curScreen && curScreen->CanDraw()) {
    float useInterp = x10_alphaInterp == 0.f ? 1.f : x10_alphaInterp / 0.5f;
    float initInterp = std::min(curScreen->GetAlpha(), useInterp);
    if (xc_nextSubscreen == ESubScreen::ToGame)
      totalAlpha = useInterp;
    else if (x91_initialTransition)
      totalAlpha = initInterp;
    else
      totalAlpha = 1.f;

    curScreen->Draw(x8_curSubscreen != xc_nextSubscreen ? useInterp : 1.f, totalAlpha, 0.f);
    yOff = curScreen->GetCameraYBias();
  }

  CGuiWidgetDrawParms parms(totalAlpha, zeus::CVector3f{0.f, 15.f * yOff, 0.f});
  x34_loadedPauseScreenInstructions->Draw(parms);
}

bool CPauseScreen::ShouldSwitchToMapScreen() const {
  return IsLoaded() && x8_curSubscreen == ESubScreen::ToMap && xc_nextSubscreen == ESubScreen::ToMap;
}

bool CPauseScreen::ShouldSwitchToInGame() const {
  return IsLoaded() && x8_curSubscreen == ESubScreen::ToGame && xc_nextSubscreen == ESubScreen::ToGame;
}

float CPauseScreen::GetHelmetCamYOff() const {
  CPauseScreenBase* screen = x7c_screens[x78_activeIdx].get();
  if (screen)
    return screen->GetCameraYBias();
  return 0.f;
}

} // namespace urde::MP1
