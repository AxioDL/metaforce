#include "MetroidPrime/CPauseScreen.hpp"
#include "MetroidPrime/CInventoryScreen.hpp"
#include "MetroidPrime/CLogBookScreen.hpp"
#include "MetroidPrime/COptionsScreen.hpp"

#include "GuiSys/CAuiImagePane.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiModel.hpp"
#include "GuiSys/CGuiSliderGroup.hpp"
#include "GuiSys/CGuiSys.hpp"
#include "GuiSys/CGuiTableGroup.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/CDvdRequest.hpp"
#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/CControlMapper.hpp"
#include "MetroidPrime/Factories/CScannableObjectInfo.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerRes.hpp"
#include "rstl/StringExtras.hpp"
#include "rstl/math.hpp"
#include <float.h>

CPauseScreen::ESubScreen CPauseScreen::GetPreviousSubscreen(ESubScreen screen) {
  if (screen == kSS_Inventory) {
    return kSS_Options;
  }
  if (screen == kSS_Options) {
    return kSS_LogBook;
  }
  if (screen == kSS_LogBook) {
    return kSS_Inventory;
  }
  return kSS_ToGame;
}

CPauseScreen::ESubScreen CPauseScreen::GetNextSubscreen(ESubScreen screen) {
  if (screen == kSS_Inventory) {
    return kSS_LogBook;
  }
  if (screen == kSS_Options) {
    return kSS_Inventory;
  }
  if (screen == kSS_LogBook) {
    return kSS_Options;
  }
  return kSS_ToGame;
}

CPauseScreen::CPauseScreen(int subscreen, const CDependencyGroup& suitDgrp,
                           const CDependencyGroup& ballDgrp)
: mInitialSubscreen(static_cast< ESubScreen >(subscreen))
, mDirection(2)
, mCurSubscreen(kSS_ToGame)
, mNextSubscreen(kSS_ToGame)
, mAlphaInterp(0.f)
, mStrgPauseScreen(gpSimplePool->GetObj("STRG_PauseScreen"))
, mSuitDgrp(suitDgrp)
, mBallDgrp(ballDgrp)
, mPauseScreenInstructions(gpSimplePool->GetObj("FRME_PauseScreenInstructions"))
, mLoadedPauseScreenInstructions(nullptr)
, mTextpane_l1(nullptr)
, mTextpane_r(nullptr)
, mTextpane_a(nullptr)
, mTextpane_b(nullptr)
, mTextpane_return(nullptr)
, mTextpane_next(nullptr)
, mTextpane_back(nullptr)
, mFrmePauseScreenId(gpResourceFactory->GetResourceIdByName("FRME_PauseScreen")->GetId())
, mFrmePauseScreenBufSz(
      gpResourceFactory->ResourceSize(SObjectTag('FRME', mFrmePauseScreenId)))
, mFrmePauseScreenBuf(
      static_cast< char* >(CMemory::Alloc(mFrmePauseScreenBufSz, IAllocator::kHI_RoundUpLen)))
, mLoadTok(gpResourceFactory->GetResLoader().LoadResourceAsync(
      SObjectTag('FRME', mFrmePauseScreenId), mFrmePauseScreenBuf.get()))
, mActiveIdx(0)
, mScreens(rstl::auto_ptr< CPauseScreenBase >())
, mResourcesLoaded(false)
, mInitialTransition(true) {
  mStrgPauseScreen.Lock();
  mPauseScreenInstructions.Lock();
  CSfxManager::SfxStart(0x59b, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                        CSfxManager::kAllAreas);
}

CPauseScreen::~CPauseScreen() { CFrameDelayedKiller::StallAndFlushAllAllocations(); }

void CPauseScreen::InitializeFrameGlue() {
  mTextpane_l1 =
      static_cast< CGuiTextPane* >(mLoadedPauseScreenInstructions->FindWidget("textpane_l1"));
  mTextpane_r =
      static_cast< CGuiTextPane* >(mLoadedPauseScreenInstructions->FindWidget("textpane_r"));
  mTextpane_a =
      static_cast< CGuiTextPane* >(mLoadedPauseScreenInstructions->FindWidget("textpane_a"));
  mTextpane_b =
      static_cast< CGuiTextPane* >(mLoadedPauseScreenInstructions->FindWidget("textpane_b"));
  mTextpane_return = static_cast< CGuiTextPane* >(
      mLoadedPauseScreenInstructions->FindWidget("textpane_return"));
  mTextpane_next =
      static_cast< CGuiTextPane* >(mLoadedPauseScreenInstructions->FindWidget("textpane_next"));
  mTextpane_back =
      static_cast< CGuiTextPane* >(mLoadedPauseScreenInstructions->FindWidget("textpane_back"));
  mTextpane_a->TextSupport().SetText(mStrgPauseScreen.GetObject()->GetString(7));
  mTextpane_a->TextSupport().SetFontColor(gpTweakGuiColors->GetPauseItemAmberColor());
  mTextpane_b->TextSupport().SetText(mStrgPauseScreen.GetObject()->GetString(6));
  mTextpane_b->TextSupport().SetFontColor(gpTweakGuiColors->GetPauseItemAmberColor());
  mTextpane_a->SetColor(CColor(0u));
  mTextpane_b->SetColor(CColor(0u));
  CGuiWidget* deco = mLoadedPauseScreenInstructions->FindWidget("basewidget_deco");
  if (deco != nullptr) {
    deco->SetColor(gpTweakGuiColors->GetPauseItemAmberColor().WithAlphaModulatedBy(0.75f));
  }
}

bool CPauseScreen::CheckLoadComplete(const CStateManager& mgr) {
  if (mResourcesLoaded) {
    return true;
  }
  if (!mStrgPauseScreen.TryCache()) {
    return false;
  }
  if (mLoadedPauseScreenInstructions == nullptr) {
    if (!mPauseScreenInstructions.TryCache()) {
      return false;
    }
    if (!mPauseScreenInstructions.GetObject()->GetIsFinishedLoading()) {
      return false;
    }
    mLoadedPauseScreenInstructions = mPauseScreenInstructions.GetObject();
    InitializeFrameGlue();
  }
  if (!mLoadTok.null()) {
    if (mLoadTok->IsComplete()) {
      {
        CMemoryInStream in(mFrmePauseScreenBuf.get(), mFrmePauseScreenBufSz);
        mFrameInsts.push_back(
            CGuiFrame::CreateFrame(mFrmePauseScreenId, *gGuiSystem, in, gpSimplePool));
      }
      {
        CMemoryInStream in(mFrmePauseScreenBuf.get(), mFrmePauseScreenBufSz);
        mFrameInsts.push_back(
            CGuiFrame::CreateFrame(mFrmePauseScreenId, *gGuiSystem, in, gpSimplePool));
      }
      mLoadTok = nullptr;
      mFrmePauseScreenBuf = nullptr;
      mFrmePauseScreenBufSz = 0;
    } else {
      return false;
    }
  }

  if (!mFrameInsts[0]->GetIsFinishedLoading() || !mFrameInsts[1]->GetIsFinishedLoading()) {
    return false;
  }
  mResourcesLoaded = true;
  StartTransition(FLT_EPSILON, mgr, mInitialSubscreen, 2);
  mInitialTransition = true;
  return true;
}

CPauseScreenBase* CPauseScreen::BuildPauseSubScreen(ESubScreen screen, const CStateManager& mgr,
                                                    CGuiFrame& frame) const {
  const CStringTable& pauseStrg = *mStrgPauseScreen.GetObject();
  switch (screen) {
  case kSS_LogBook:
    return rs_new CLogBookScreen(mgr, frame, pauseStrg);
  case kSS_Options:
    return rs_new COptionsScreen(mgr, frame, pauseStrg);
  case kSS_Inventory:
    return rs_new CInventoryScreen(mgr, frame, pauseStrg, mSuitDgrp, mBallDgrp);
  case kSS_ToGame:
  case kSS_ToMap:
  default:
    return nullptr;
  }
}

void CPauseScreen::StartTransition(float time, const CStateManager& mgr, ESubScreen screen,
                                   int direction) {
  if (screen == mNextSubscreen) {
    return;
  }
  mNextSubscreen = screen;
  mDirection = direction;
  mAlphaInterp = time;
  rstl::auto_ptr< CPauseScreenBase >& curScreen = mScreens[mActiveIdx];
  rstl::auto_ptr< CPauseScreenBase >& nextScreen = mScreens[1 - mActiveIdx];
  rstl::auto_ptr< CGuiFrame >& nextFrame = mFrameInsts[1 - mActiveIdx];
  nextScreen = BuildPauseSubScreen(mNextSubscreen, mgr, *nextFrame);
  if (!curScreen.null()) {
    curScreen->TransitioningAway();
  }
  mInitialTransition = false;
}

void CPauseScreen::Update(float dt, const CStateManager& mgr, CRandom16& rand,
                          CArchitectureQueue& queue) {
  if (!CheckLoadComplete(mgr)) {
    return;
  }
  rstl::auto_ptr< CPauseScreenBase >& curScreen = mScreens[mActiveIdx];
  rstl::auto_ptr< CPauseScreenBase >& otherScreen = mScreens[1 - mActiveIdx];
  if (mCurSubscreen != mNextSubscreen) {
    mAlphaInterp = rstl::max_val(0.f, mAlphaInterp - dt);
    if (curScreen.null() || !curScreen->InputDisabled()) {
      if (otherScreen.null() || otherScreen->IsReady()) {
        if (mAlphaInterp == 0.f) {
          TransitionComplete();
        }
      }
    }
  }
  rstl::auto_ptr< CPauseScreenBase >& screen = mScreens[mActiveIdx];
  if (!screen.null()) {
    screen->Update(dt, rand, queue);
    float interp = mAlphaInterp / 0.5f;
    float alpha =
        rstl::min_val(screen->GetAlpha(), mCurSubscreen != mNextSubscreen ? interp : 1.f);
    mTextpane_a->SetColor(CColor::White().WithAlphaOf(alpha));
    mTextpane_b->SetColor(CColor::White().WithAlphaOf(alpha));
  }
}

void CPauseScreen::PreDraw() {
  if (!IsLoaded()) {
    return;
  }
  rstl::auto_ptr< CPauseScreenBase >& screen = mScreens[mActiveIdx];
  if (!screen.null() && screen->CanDraw()) {
    screen->Touch();
  }
}

void CPauseScreen::TransitionComplete() {
  mScreens[mActiveIdx] = rstl::auto_ptr< CPauseScreenBase >();
  mActiveIdx = 1 - mActiveIdx;
  mCurSubscreen = mNextSubscreen;
  mTextpane_a->TextSupport().SetText(
      mStrgPauseScreen.GetObject()->GetString(GetPreviousSubscreen(mCurSubscreen) + 6));
  mTextpane_b->TextSupport().SetText(
      mStrgPauseScreen.GetObject()->GetString(GetNextSubscreen(mCurSubscreen) + 6));
}

void CPauseScreen::Draw() const {
  if (!IsLoaded()) {
    return;
  }
  float interp = mAlphaInterp == 0.f ? 1.f : mAlphaInterp / 0.5f;
  float yOff = 0.f;
  float totalAlpha = 0.f;
  const rstl::auto_ptr< CPauseScreenBase >& screen = mScreens[mActiveIdx];
  if (!screen.null() && screen->CanDraw()) {
    float initInterp = rstl::min_val(screen->GetAlpha(), interp);
    float alpha;
    if (IsInvalidSubscreen(mNextSubscreen)) {
      alpha = interp;
    } else if (mInitialTransition) {
      alpha = initInterp;
    } else {
      alpha = 1.f;
    }
    totalAlpha = alpha;
    screen->Draw(mCurSubscreen != mNextSubscreen ? interp : 1.f, alpha, 0.f);
    yOff = screen->GetCameraYBias();
  }
  mLoadedPauseScreenInstructions->Draw(
      CGuiWidgetDrawParms(totalAlpha, CVector3f(0.f, 15.f * yOff, 0.f)));
}

void CPauseScreen::ProcessControllerInput(const CStateManager& mgr, const CFinalInput& input) {
  if (!IsLoaded()) {
    return;
  }
  if (IsInvalidSubscreen(mCurSubscreen)) {
    return;
  }
  bool bExits = false;
  rstl::auto_ptr< CPauseScreenBase >& screen = mScreens[mActiveIdx];
  if (!screen.null()) {
    if (screen->GetMode() == CPauseScreenBase::kM_LeftTable) {
      bExits = true;
    }
    screen->ProcessInput(input);
  }
  if (InputEnabled()) {
    bool invalid = IsInvalidSubscreen(mCurSubscreen);
    if (input.PStart() || (input.PB() && bExits) ||
        (!mScreens[mActiveIdx].null() &&
         mScreens[mActiveIdx]->ShouldExitPauseScreen())) {
      CSfxManager::SfxStart(0x59a, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
      StartTransition(0.5f, mgr, kSS_ToGame, 2);
    } else if (ControlMapper::GetPressInput(ControlMapper::kC_PreviousPauseScreen, input)) {
      CSfxManager::SfxStart(0x599, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
      int direction = 0;
      if (invalid) {
        direction = 2;
      }
      StartTransition(0.5f, mgr, GetPreviousSubscreen(mCurSubscreen), direction);
    } else if (ControlMapper::GetPressInput(ControlMapper::kC_NextPauseScreen, input)) {
      CSfxManager::SfxStart(0x599, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
      int direction = 1;
      if (invalid) {
        direction = 2;
      }
      StartTransition(0.5f, mgr, GetNextSubscreen(mCurSubscreen), direction);
    }
  }
  int l = input.DLTrigger() ? 1 : 0;
  int r = input.DRTrigger() ? 1 : 0;
  int start = input.DStart() ? 1 : 0;
  int a = input.DA() ? 1 : 0;
  int b = input.DB() ? 1 : 0;
  const wchar_t image[] = L"&image=";
  const wchar_t end[] = L";";
  rstl::wstring text;
  text.reserve(256);
  text.append(image, -1);
  text.append(CStringExtras::ConvertToUNICODE(
      CBasics::Stringize("%8.8X", gpTweakPlayerRes->mLTrigger[l])));
  text.append(end, -1);
  mTextpane_l1->TextSupport().SetText(text);
  text.assign(image, -1);
  text.append(CStringExtras::ConvertToUNICODE(
      CBasics::Stringize("%8.8X", gpTweakPlayerRes->mRTrigger[r])));
  text.append(end, -1);
  mTextpane_r->TextSupport().SetText(text);
  text.assign(image, -1);
  text.append(CStringExtras::ConvertToUNICODE(
      CBasics::Stringize("%8.8X", gpTweakPlayerRes->mStartButton[start])));
  text.append(end, -1);
  mTextpane_return->TextSupport().SetText(text);
  text.assign(image, -1);
  text.append(CStringExtras::ConvertToUNICODE(
      CBasics::Stringize("%8.8X", gpTweakPlayerRes->mAButton[a])));
  text.append(end, -1);
  mTextpane_back->TextSupport().SetText(text);
  text.assign(image, -1);
  text.append(CStringExtras::ConvertToUNICODE(
      CBasics::Stringize("%8.8X", gpTweakPlayerRes->mBButton[b])));
  text.append(end, -1);
  mTextpane_next->TextSupport().SetText(text);
}

bool CPauseScreen::InputEnabled() const {
  if (mCurSubscreen != mNextSubscreen) {
    return false;
  }
  const rstl::auto_ptr< CPauseScreenBase >& screen = mScreens[mActiveIdx];
  if (!screen.null() && screen->InputDisabled()) {
    return false;
  }
  const rstl::auto_ptr< CPauseScreenBase >& otherScreen = mScreens[1 - mActiveIdx];
  if (!otherScreen.null() && otherScreen->InputDisabled()) {
    return false;
  }
  return true;
}

bool CPauseScreen::IsLoaded() const { return mResourcesLoaded; }

bool CPauseScreen::ShouldSwitchToMapScreen() const {
  return IsLoaded() && mCurSubscreen == kSS_ToMap && mNextSubscreen == kSS_ToMap;
}

bool CPauseScreen::ShouldSwitchToInGame() const {
  return IsLoaded() && mCurSubscreen == kSS_ToGame && mNextSubscreen == kSS_ToGame;
}

float CPauseScreen::GetHelmetCamYOff() const {
  CPauseScreenBase* screen = mScreens[mActiveIdx].get();
  if (screen == nullptr) {
    return 0.f;
  }
  return screen->GetCameraYBias();
}

CPauseScreenBase::CPauseScreenBase(const CStateManager& mgr, CGuiFrame& frame,
                                   const CStringTable& pauseStrg)
: mMgr(mgr)
, mFrame(frame)
, mPauseStrg(pauseStrg)
, mMode(kM_Invalid)
, mAlpha(0.f)
, mFirstViewRightSel(0)
, mRightSel(0)
, x20_(CVector3f::Zero())
, mRightTableStart(CVector3f::Zero())
, mHighlightPitch(0.f)
, mSliderStart(CVector3f::Zero())
, mTableDoubleStart(CVector3f::Zero())
, mTableTripleStart(CVector3f::Zero())
, mBasewidget_leftside(nullptr)
, mBasewidget_leftlog(nullptr)
, mTablegroup_leftlog(nullptr)
, mBasewidget_leftguages(nullptr)
, mModel_lefthighlight(nullptr)
, mBasewidget_rightside(nullptr)
, mBasewidget_rightlog(nullptr)
, mTablegroup_rightlog(nullptr)
, mBasewidget_rightguages(nullptr)
, mModel_righthighlight(nullptr)
, mTextpane_body(nullptr)
, mTextpane_title(nullptr)
, mBasewidget_yicon(nullptr)
, mTextpane_yicon(nullptr)
, mTextpane_ytext(nullptr)
, mSlidergroup_slider(nullptr)
, mTablegroup_double(nullptr)
, mTablegroup_triple(nullptr)
, mReady(false)
, mHandledInput(false)
, mExitPauseScreen(false)
, mCanDraw(false)
, mPulseTextArrowTop(false)
, mPulseTextArrowBottom(false) {
  InitializeFrameGlue();
}

CPauseScreenBase::~CPauseScreenBase() { CFrameDelayedKiller::StallAndFlushAllAllocations(); }

void CPauseScreenBase::InitializeFrameGlue() {
  mBasewidget_pivot = mFrame.FindWidget("basewidget_pivot");
  mBasewidget_bgframe = mFrame.FindWidget("basewidget_bgframe");
  mBasewidget_leftside = mFrame.FindWidget("basewidget_leftside");
  mBasewidget_leftlog = mFrame.FindWidget("basewidget_leftlog");
  mTablegroup_leftlog =
      static_cast< CGuiTableGroup* >(mFrame.FindWidget("tablegroup_leftlog"));
  mBasewidget_leftguages = mFrame.FindWidget("basewidget_leftguages");
  mModel_lefthighlight = static_cast< CGuiModel* >(mFrame.FindWidget("model_lefthighlight"));
  mBasewidget_rightside = mFrame.FindWidget("basewidget_rightside");
  mBasewidget_rightlog = mFrame.FindWidget("basewidget_rightlog");
  mTablegroup_rightlog =
      static_cast< CGuiTableGroup* >(mFrame.FindWidget("tablegroup_rightlog"));
  mBasewidget_rightguages = mFrame.FindWidget("basewidget_rightguages");
  mModel_righthighlight = static_cast< CGuiModel* >(mFrame.FindWidget("model_righthighlight"));
  mModel_textarrowtop = static_cast< CGuiModel* >(mFrame.FindWidget("model_textarrowtop"));
  mModel_textarrowbottom =
      static_cast< CGuiModel* >(mFrame.FindWidget("model_textarrowbottom"));
  mModel_scrollleftup = static_cast< CGuiModel* >(mFrame.FindWidget("model_scrollleftup"));
  mModel_scrollleftdown = static_cast< CGuiModel* >(mFrame.FindWidget("model_scrollleftdown"));
  mModel_scrollrightup = static_cast< CGuiModel* >(mFrame.FindWidget("model_scrollrightup"));
  mModel_scrollrightdown =
      static_cast< CGuiModel* >(mFrame.FindWidget("model_scrollrightdown"));
  mTextpane_title = static_cast< CGuiTextPane* >(mFrame.FindWidget("textpane_title"));
  mTextpane_title->TextSupport().SetFontColor(gpTweakGuiColors->GetPauseItemAmberColor());
  mTextpane_body = static_cast< CGuiTextPane* >(mFrame.FindWidget("textpane_body"));
  mTextpane_body->SetIsVisible(true);
  mTextpane_body->TextSupport().SetFontColor(gpTweakGuiColors->GetPauseItemAmberColor());
  mTextpane_body->TextSupport().SetPage(0);
  mTextpane_body->TextSupport().SetText(rstl::wstring_l(L""));
  mTextpane_body->TextSupport().SetJustification(kJustification_Left);
  mTextpane_body->TextSupport().SetVerticalJustification(kVerticalJustification_Top);
  mTextpane_body->TextSupport().SetControlTXTRMap(
      &gpGameState->GameOptions().GetControlTXTRMap());
  mBasewidget_yicon = mFrame.FindWidget("basewidget_yicon");
  mBasewidget_yicon->SetVisibility(false, kTM_Children);
  mModel_textalpha = static_cast< CGuiModel* >(mFrame.FindWidget("model_textalpha"));
  mTextpane_yicon = static_cast< CGuiTextPane* >(mFrame.FindWidget("textpane_yicon"));
  mTextpane_ytext = static_cast< CGuiTextPane* >(mFrame.FindWidget("textpane_ytext"));
  {
    const wchar_t image[] = L"&image=";
    const wchar_t end[] = L";";
    rstl::wstring text;
    text.reserve(32);
    text.assign(image, -1);
    text.append(CStringExtras::ConvertToUNICODE(
        CBasics::Stringize("%8.8X", gpTweakPlayerRes->mYButton[0])));
    text.append(end, -1);
    mTextpane_yicon->TextSupport().SetText(text);
    mTextpane_ytext->TextSupport().SetText(rstl::wstring_l(mPauseStrg.GetString(99)));
    mTextpane_ytext->SetColor(gpTweakGuiColors->GetPauseItemAmberColor());
  }
  mSlidergroup_slider =
      static_cast< CGuiSliderGroup* >(mFrame.FindWidget("slidergroup_slider"));
  mTablegroup_double = static_cast< CGuiTableGroup* >(mFrame.FindWidget("tablegroup_double"));
  mTablegroup_triple = static_cast< CGuiTableGroup* >(mFrame.FindWidget("tablegroup_triple"));
  mRightTableStart =
      mTablegroup_rightlog->GetWorkerWidget(0)->GetIdleXform().GetTranslation();
  mHighlightPitch =
      mTablegroup_rightlog->GetWorkerWidget(1)->GetIdleXform().GetTranslation().GetZ() -
      mRightTableStart.GetZ();
  mSliderStart = mSlidergroup_slider->GetIdleXform().GetTranslation();
  mTableDoubleStart = mTablegroup_double->GetIdleXform().GetTranslation();
  mTableTripleStart = mTablegroup_triple->GetIdleXform().GetTranslation();
  for (int i = 0; i < 5; ++i) {
    mTablegroup_leftlog->GetWorkerWidget(i)->SetIsSelectable(true);
  }
  CGuiWidget* worker;
  for (int i = 0; i < mTablegroup_rightlog->GetElementCount(); ++i) {
    worker = mTablegroup_rightlog->GetWorkerWidget(i);
    worker->SetO2PTransform(
        CTransform4f::Translate(mRightTableStart + CVector3f(0.f, 0.f, mHighlightPitch * i)));
    worker->SetIsSelectable(true);
  }
  for (int i = 0; i < 5; ++i) {
    CGuiTextPane* title = static_cast< CGuiTextPane* >(
        mFrame.FindWidget(CBasics::Stringize("textpane_title%d", i + 1)));
    title->TextSupport().SetText(rstl::wstring_l(L""));
    mTextpane_titles.push_back(title);
    mModel_titles.push_back(
        static_cast< CGuiModel* >(mFrame.FindWidget(CBasics::Stringize("model_title%d", i + 1))));
  }
  for (int i = 0; i < 5; ++i) {
    mModel_righttitledecos.push_back(static_cast< CGuiModel* >(
        mFrame.FindWidget(CBasics::Stringize("model_righttitledeco%d", i + 1))));
  }
  for (int i = 0; i < 5; ++i) {
    CGuiTextPane* category = static_cast< CGuiTextPane* >(
        mFrame.FindWidget(CBasics::Stringize("textpane_category%d", i)));
    category->TextSupport().SetText(rstl::wstring_l(L""));
    mTextpane_categories.push_back(category);
    mModel_categories.push_back(
        static_cast< CGuiModel* >(mFrame.FindWidget(CBasics::Stringize("model_category%d", i))));
  }
  for (int i = 0; i < 20; ++i) {
    mImagePanes.push_back(static_cast< CAuiImagePane* >(
        mFrame.FindWidget(CScannableObjectInfo::GetImagePaneName(i))));
  }
  mTablegroup_leftlog->SetUserSelection(0);
  mTablegroup_rightlog->SetUserSelection(1);
  mBasewidget_leftguages->SetVisibility(false, kTM_Children);
  mBasewidget_rightguages->SetVisibility(false, kTM_Children);
  mBasewidget_leftlog->SetColor(gpTweakGuiColors->GetPauseItemAmberColor());
  if (IsRightLogDynamic()) {
    UpdateRightLogColors(false, gpTweakGuiColors->GetPauseItemAmberColor(),
                         gpTweakGuiColors->GetPauseItemAmberColor().WithAlphaOf(0.5f));
  } else {
    mBasewidget_rightlog->SetColor(gpTweakGuiColors->GetPauseItemAmberColor());
  }
  for (CGuiObject* obj = mBasewidget_bgframe->ChildObject(); obj != nullptr;
       obj = obj->NextSibling()) {
    static_cast< CGuiWidget* >(obj)->SetColor(gpTweakGuiColors->GetPauseItemAmberColor());
  }
  CColor dimColor = gpTweakGuiColors->GetPauseItemAmberColor().WithAlphaOf(0.2f);
  mModel_scrollleftup->SetColor(dimColor);
  mModel_scrollleftdown->SetColor(dimColor);
  mModel_scrollrightup->SetColor(dimColor);
  mModel_scrollrightdown->SetColor(dimColor);
  mModel_textarrowtop->SetColor(dimColor);
  mModel_textarrowbottom->SetColor(dimColor);
  mSlidergroup_slider->SetColor(gpTweakGuiColors->GetPauseItemAmberColor());
  mTablegroup_double->SetColor(gpTweakGuiColors->GetPauseItemAmberColor());
  mTablegroup_triple->SetColor(gpTweakGuiColors->GetPauseItemAmberColor());
  UpdateSideTable(mTablegroup_double);
  UpdateSideTable(mTablegroup_triple);
  UpdateSideTable(mTablegroup_leftlog);
  UpdateSideTable(mTablegroup_rightlog);
  mSlidergroup_slider->SetVisibility(false, kTM_Children);
  mTablegroup_double->SetIsVisible(false);
  mTablegroup_triple->SetIsVisible(false);
  mTablegroup_double->SetVertical(false);
  mTablegroup_triple->SetVertical(false);
  mTablegroup_leftlog->SetMenuAdvanceCallback(
      TFunctor1FromMethod< CPauseScreenBase, CGuiTableGroup* const >::Make(
          *this, &CPauseScreenBase::OnLeftTableAdvance));
  mTablegroup_leftlog->SetMenuSelectionChangeCallback(
      TFunctor2FromMethod< CPauseScreenBase, CGuiTableGroup* const, const int >::Make(
          *this, &CPauseScreenBase::OnTableSelectionChange));
  mTablegroup_rightlog->SetMenuAdvanceCallback(
      TFunctor1FromMethod< CPauseScreenBase, CGuiTableGroup* const >::Make(
          *this, &CPauseScreenBase::OnRightTableAdvance));
  mTablegroup_rightlog->SetMenuSelectionChangeCallback(
      TFunctor2FromMethod< CPauseScreenBase, CGuiTableGroup* const, const int >::Make(
          *this, &CPauseScreenBase::OnTableSelectionChange));
  mTablegroup_rightlog->SetMenuCancelCallback(
      TFunctor1FromMethod< CPauseScreenBase, CGuiTableGroup* const >::Make(
          *this, &CPauseScreenBase::OnRightTableCancel));
  mSlidergroup_slider->SetSelectionChangedCallback(
      TFunctor2< CGuiSliderGroup* const, const float >());
  mTablegroup_double->SetMenuSelectionChangeCallback(
      TFunctor2< CGuiTableGroup* const, const int >());
  mTablegroup_triple->SetMenuSelectionChangeCallback(
      TFunctor2< CGuiTableGroup* const, const int >());
}

bool CPauseScreenBase::IsReady() {
  if (mReady) {
    return true;
  }
  mReady = VReady();
  if (mReady) {
    VActivate();
    ChangeMode(kM_LeftTable);
    UpdateSideTable(mTablegroup_leftlog);
    UpdateRightTable();
    return true;
  }
  return false;
}

void CPauseScreenBase::Update(float dt, CRandom16& rand, CArchitectureQueue& queue) {
  mCanDraw = true;
  mFrame.Update(dt);
  mAlpha = rstl::min_val(1.f, 2.f * dt + mAlpha);
  int rightCount = GetRightTableCount();
  bool pulseRightUp = mMode == kM_RightTable && mFirstViewRightSel > 0;
  bool pulseRightDown = mMode == kM_RightTable && mFirstViewRightSel + 5 < rightCount;
  float rightUpT = pulseRightUp ? CGraphics::GetSecondsMod900() : 0.f;
  float rightDownT = pulseRightDown ? CGraphics::GetSecondsMod900() : 0.f;
  float rightUpAlpha =
      CMath::Clamp(0.f, 0.5f * (1.f + CMath::FastSinR(5.f * rightUpT - M_PIF / 2.f)), 1.f);
  float rightDownAlpha =
      CMath::Clamp(0.f, 0.5f * (1.f + CMath::FastSinR(5.f * rightDownT - M_PIF / 2.f)), 1.f);
  CColor lowColor = gpTweakGuiColors->GetPauseItemAmberColor().WithAlphaOf(0.2f);
  mModel_scrollrightup->SetColor(
      CColor::Lerp(lowColor, gpTweakGuiColors->GetPauseItemAmberColor(), rightUpAlpha));
  mModel_scrollrightdown->SetColor(
      CColor::Lerp(lowColor, gpTweakGuiColors->GetPauseItemAmberColor(), rightDownAlpha));
  float textUpT = mPulseTextArrowTop ? CGraphics::GetSecondsMod900() : 0.f;
  float textDownT = mPulseTextArrowBottom ? CGraphics::GetSecondsMod900() : 0.f;
  float textUpAlpha =
      CMath::Clamp(0.f, 0.5f * (1.f + CMath::FastSinR(5.f * textUpT - M_PIF / 2.f)), 1.f);
  float textDownAlpha =
      CMath::Clamp(0.f, 0.5f * (1.f + CMath::FastSinR(5.f * textDownT - M_PIF / 2.f)), 1.f);
  CColor textLowColor = gpTweakGuiColors->GetPauseItemAmberColor().WithAlphaOf(0.2f);
  mModel_textarrowtop->SetColor(
      CColor::Lerp(textLowColor, gpTweakGuiColors->GetPauseItemAmberColor(), textUpAlpha));
  mModel_textarrowbottom->SetColor(
      CColor::Lerp(textLowColor, gpTweakGuiColors->GetPauseItemAmberColor(), textDownAlpha));
}

void CPauseScreenBase::Touch() {}

void CPauseScreenBase::Draw(float transInterp, float totalAlpha, float yOff) const {
  mBasewidget_pivot->SetColor(CColor::White().WithAlphaOf(transInterp * mAlpha));
  mBasewidget_bgframe->SetColor(CColor::White().WithAlphaOf(totalAlpha));
  mFrame.Draw(CGuiWidgetDrawParms(1.f, CVector3f(0.f, 15.f * yOff, 0.f)));
}

void CPauseScreenBase::ProcessInput(const CFinalInput& input) {
  mHandledInput = false;
  mFrame.ProcessUserInput(input);
}

void CPauseScreenBase::UpdateSideTable(CGuiTableGroup* table) {
  if (table == nullptr) {
    return;
  }
  const CColor selColor(0xffffffff);
  const CColor deselColor(uchar(255), uchar(255), uchar(255), uchar(127));
  bool tableActive = true;
  if (table == mTablegroup_rightlog && mMode != kM_RightTable) {
    tableActive = false;
  }
  table->SetColors(selColor, deselColor);
  if (table == mTablegroup_rightlog) {
    int sel = mRightSel - mFirstViewRightSel;
    int selInView = mRightSel % 5;
    mModel_righthighlight->SetO2PTransform(
        mModel_righthighlight->GetTransform() *
        CTransform4f::Translate(0.f, 0.f, mHighlightPitch * sel));
    mModel_righthighlight->SetVisibility(mMode == kM_RightTable, kTM_Children);
    if (IsRightLogDynamic()) {
      UpdateRightLogHighlight(tableActive, selInView, selColor, deselColor);
    } else {
      for (int i = 0; i < mModel_titles.size(); ++i) {
        mModel_titles[i]->SetColor(i == selInView && tableActive ? selColor : deselColor);
      }
    }
  } else {
    int sel = mTablegroup_leftlog->GetUserSelection();
    mModel_lefthighlight->SetO2PTransform(
        mModel_lefthighlight->GetTransform() *
        CTransform4f::Translate(0.f, 0.f, mHighlightPitch * sel));
    for (int i = 0; i < mModel_categories.size(); ++i) {
      mModel_categories[i]->SetColor(i == sel ? selColor : deselColor);
    }
  }
}

void CPauseScreenBase::ChangeMode(EMode mode) {
  if (mode == mMode) {
    return;
  }
  EMode oldMode = mMode;
  CColor color = gpTweakGuiColors->GetPauseItemAmberColor();
  CColor dimColor = color.WithAlphaOf(0.5f);
  switch (mMode) {
  case kM_LeftTable:
    mBasewidget_leftlog->SetColor(dimColor);
    mTablegroup_leftlog->SetIsActive(false);
    break;
  case kM_Invalid:
  case kM_RightTable:
    if (IsRightLogDynamic()) {
      UpdateRightLogColors(false, color, dimColor);
    } else {
      mBasewidget_rightlog->SetColor(dimColor);
    }
    mTablegroup_rightlog->SetIsActive(false);
    break;
  case kM_TextScroll:
    CSfxManager::SfxStart(0x597, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
    break;
  default:
    break;
  }
  mMode = mode;
  switch (mMode) {
  case kM_LeftTable:
    if (oldMode == kM_RightTable) {
      CSfxManager::SfxStart(0x597, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
    }
    mBasewidget_leftlog->SetColor(color);
    mTablegroup_leftlog->SetIsActive(true);
    UpdateSideTable(mTablegroup_leftlog);
    mFirstViewRightSel = 0;
    mRightSel = 0;
    mTablegroup_rightlog->SetUserSelection(1);
    UpdateSideTable(mTablegroup_rightlog);
    break;
  case kM_RightTable:
    if (IsRightLogDynamic()) {
      UpdateRightLogColors(true, color, dimColor);
    } else {
      mBasewidget_rightlog->SetColor(color);
    }
    mTablegroup_rightlog->SetIsActive(true);
    UpdateSideTable(mTablegroup_rightlog);
    break;
  case kM_TextScroll:
    mBasewidget_leftlog->SetColor(dimColor);
    if (IsRightLogDynamic()) {
      UpdateRightLogColors(false, color, dimColor);
    } else {
      mBasewidget_rightlog->SetColor(dimColor);
    }
    mTablegroup_leftlog->SetIsActive(false);
    mTablegroup_rightlog->SetIsActive(false);
    break;
  default:
    break;
  }
  ChangedMode(oldMode);
}

void CPauseScreenBase::ChangedMode(EMode oldMode) {}

void CPauseScreenBase::UpdateRightTable() {
  mFirstViewRightSel = 0;
  mRightSel = 0;
  mTablegroup_rightlog->SetUserSelection(1);
  UpdateSideTable(mTablegroup_rightlog);
}

bool CPauseScreenBase::ShouldLeftTableAdvance() { return true; }

void CPauseScreenBase::OnLeftTableAdvance(CGuiTableGroup* caller) {
  if (ShouldLeftTableAdvance()) {
    ChangeMode(kM_RightTable);
    mHandledInput = true;
    CSfxManager::SfxStart(0x598, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
  }
}

bool CPauseScreenBase::ShouldRightTableAdvance() { return true; }

void CPauseScreenBase::OnRightTableAdvance(CGuiTableGroup* caller) {
  if (ShouldRightTableAdvance() && !mHandledInput) {
    ChangeMode(kM_TextScroll);
    CSfxManager::SfxStart(0x598, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
  }
}

void CPauseScreenBase::OnTableSelectionChange(CGuiTableGroup* caller, int oldSel) {
  UpdateSideTable(caller);
  if (caller == mTablegroup_leftlog) {
    CSfxManager::SfxStart(0x59c, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
    UpdateRightTable();
  } else {
    SetRightTableSelection(oldSel, mTablegroup_rightlog->GetUserSelection());
  }
}

void CPauseScreenBase::SetRightTableSelection(int oldSel, int newSel) {
  int rightCount = GetRightTableCount();
  int oldRightSel = mRightSel;
  mRightSel = CMath::Clamp(0, mRightSel + (newSel - oldSel), rightCount - 1);
  if (oldRightSel != mRightSel) {
    CSfxManager::SfxStart(0x59c, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
  }
  if (mRightSel < mFirstViewRightSel) {
    mFirstViewRightSel = mRightSel;
  } else if (mRightSel >= mFirstViewRightSel + 5) {
    mFirstViewRightSel = mRightSel - 4;
  }
  mTablegroup_rightlog->SetUserSelection(mRightSel + 1 - mFirstViewRightSel);
  UpdateSideTable(mTablegroup_rightlog);
  RightTableSelectionChanged(oldSel, newSel);
}

void CPauseScreenBase::RightTableSelectionChanged(int oldSel, int newSel) {}

void CPauseScreenBase::OnRightTableCancel(CGuiTableGroup* caller) { ChangeMode(kM_LeftTable); }

void CPauseScreenBase::TransitioningAway() {}

bool CPauseScreenBase::InputDisabled() const { return false; }

bool CPauseScreenBase::ShouldExitPauseScreen() const { return mExitPauseScreen; }
