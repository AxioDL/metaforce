#include "MetroidPrime/CLogBookScreen.hpp"

#include "GuiSys/CAuiImagePane.hpp"
#include "GuiSys/CGuiModel.hpp"
#include "GuiSys/CGuiTableGroup.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/CArtifactDoll.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CMemoryCard.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Factories/CScannableObjectInfo.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"

namespace {
struct CArtifactScanSorter {
  bool operator()(rstl::pair< CAssetId, bool >& a, rstl::pair< CAssetId, bool >& b) const {
    return CArtifactDoll::GetArtifactHeadScanIndex(a.first) <
           CArtifactDoll::GetArtifactHeadScanIndex(b.first);
  }
};

CArtifactScanSorter skArtifactScanSorter;
} // namespace

CLogBookScreen::CLogBookScreen(const CStateManager& mgr, CGuiFrame& frame,
                               const CStringTable& pauseStrg)
: CPauseScreenBase(mgr, frame, pauseStrg)
, mScanCompletes(5, CScanCategory())
, mViewScans(5, CArticleList())
, mViewInterp(0.f)
, mArtifactDoll(rs_new CArtifactDoll())
, mLeavePauseState(kLPS_InPause)
, mLoaded(false)
, mInTextScroll(false)
, mExitTextScroll(false) {
  CMain::EnsureWorldPaksReady();
  InitializeLogBook();
}

CLogBookScreen::~CLogBookScreen() {
  mArtifactDoll->CompleteArtifactHeadScan(mMgr);

  for (int i = 0; i < mModel_titles.size(); ++i) {
    CGuiModel* model = mModel_titles[i];
    model->SetO2PTransform(model->GetTransform());
  }

  CMain::EnsureWorldPakReady(gpGameState->CurrentWorldAssetId());
}

void CLogBookScreen::Update(float dt, CRandom16& rand, CArchitectureQueue& queue) {
  CPauseScreenBase::Update(dt, rand, queue);
  mArtifactDoll->Update(dt, mMgr);
  PumpArticleLoad();

  if (mMode == kM_TextScroll) {
    if (mInTextScroll) {
      mViewInterp = rstl::min_val(1.f, mViewInterp + 4.f * dt);
    } else {
      mViewInterp = rstl::max_val(0.f, mViewInterp - 4.f * dt);
    }

    CColor color = CColor::White().WithAlphaOf(mViewInterp);
    mBasewidget_leftguages->SetColor(color);
    mBasewidget_rightguages->SetColor(color);

    CColor invColor = CColor::White().WithAlphaOf(1.f - mViewInterp);
    mTablegroup_leftlog->SetColor(invColor);
    mTablegroup_rightlog->SetColor(invColor);
    mModel_textalpha->SetColor(invColor);
    mTextpane_body->SetColor(color);

    for (int i = 0; i < mImagePanes.size(); ++i) {
      mImagePanes[i]->SetDeResFactor(1.f - mViewInterp);
    }

    if (mViewInterp == 0.f && mLeavePauseState == kLPS_InPause) {
      ChangeMode(kM_RightTable);
    }
  }

  if (mLeavePauseState == kLPS_LeavingPause && mViewInterp == 0.f) {
    mLeavePauseState = kLPS_LeftPause;
  }
}

void CLogBookScreen::Touch() {
  CPauseScreenBase::Touch();
  mArtifactDoll->Touch();
}

bool CLogBookScreen::IsArtifactCategorySelected() const {
  return mTablegroup_leftlog->GetUserSelection() + 1 == CWorldSaveGameInfo::kSC_Artifact;
}

int CLogBookScreen::GetSelectedArtifactHeadScanIndex() const {
  const CScanCategory& category = mScanCompletes[mTablegroup_leftlog->GetUserSelection()];
  if (mRightSel < category.size()) {
    return CArtifactDoll::GetArtifactHeadScanIndex(category[mRightSel].first);
  }

  return -1;
}

void CLogBookScreen::Draw(float transInterp, float totalAlpha, float yOff) const {
  CPauseScreenBase::Draw(transInterp, totalAlpha, yOff);

  bool artifactSel = mMode == kM_RightTable && IsArtifactCategorySelected();
  int headIdx = GetSelectedArtifactHeadScanIndex();
  mArtifactDoll->Draw(transInterp * (1.f - mViewInterp), mMgr, artifactSel, headIdx);
}

void CLogBookScreen::ProcessInput(const CFinalInput& input) {
  mInTextScroll = false;
  if (mLeavePauseState == kLPS_LeftPause) {
    return;
  }

  if (mMode == kM_TextScroll) {
    int oldPage = mTextpane_body->TextSupport().GetPageCounter();
    int newPage = oldPage;
    int pageCount = mTextpane_body->TextSupport().GetTotalPageCount();
    bool lastPage = oldPage == pageCount - 1;

    if (pageCount != -1) {
      if (input.PLAUp()) {
        newPage = rstl::max_val(0, oldPage - 1);
      } else if (input.PLADown() || (input.PA() && !lastPage)) {
        newPage = rstl::min_val(pageCount - 1, oldPage + 1);
      }

      mTextpane_body->TextSupport().SetPage(newPage);
      if (oldPage != newPage) {
        CSfxManager::SfxStart(0x5a4, 0x7f, 0x40, false);
      }

      mPulseTextArrowTop = newPage > 0;
      mPulseTextArrowBottom = !lastPage;
    } else {
      mPulseTextArrowTop = mPulseTextArrowBottom = false;
    }

    if (!mExitTextScroll) {
      mExitTextScroll = input.PB() || ((input.PA() && lastPage) ? true : false);
    }

    mInTextScroll = gpTweakGui->GetLatchArticleText() ? !mExitTextScroll : input.DA();
  } else {
    mPulseTextArrowTop = mPulseTextArrowBottom = false;
  }

  if (mLeavePauseState == kLPS_LeavingPause) {
    mInTextScroll = false;
  }

  CPauseScreenBase::ProcessInput(input);
}

bool CLogBookScreen::VReady() const { return true; }

void CLogBookScreen::VActivate() {
  for (int i = 0; i < 5; ++i) {
    CGuiTextPane* category = mTextpane_categories[i];
    if (IsScanCategoryReady(static_cast< CWorldSaveGameInfo::EScanCategory >(i + 1))) {
      category->TextSupport().SetText(mPauseStrg.GetString(i + 1));
    } else {
      category->TextSupport().SetText(rstl::wstring_l(L"??????"));
      mTablegroup_leftlog->GetWorkerWidget(i)->SetIsSelectable(false);
    }
  }

  mTextpane_title->TextSupport().SetText(mPauseStrg.GetString(0));

  for (int i = 5; i < 5; ++i) {
    mTablegroup_leftlog->GetWorkerWidget(i)->SetIsSelectable(false);
  }
}

void CLogBookScreen::ChangedMode(EMode oldMode) {
  if (oldMode == kM_TextScroll) {
    mBasewidget_leftguages->SetVisibility(false, kTM_Children);
    mBasewidget_rightguages->SetVisibility(false, kTM_Children);
    UpdateBodyText();
    mTextpane_body->TextSupport().SetPage(0);
  } else if (mMode == kM_TextScroll) {
    mBasewidget_leftguages->SetVisibility(true, kTM_Children);
    mBasewidget_rightguages->SetVisibility(true, kTM_Children);
    mInTextScroll = true;
    UpdateBodyImagesAndText();
  }
}

void CLogBookScreen::UpdateRightTable() {
  CPauseScreenBase::UpdateRightTable();

  const CScanCategory& category = mScanCompletes[mTablegroup_leftlog->GetUserSelection()];
  int count = category.size();
  mCurViewScans = CArticleList();
  mCurViewScans.reserve(count);

  for (AUTO(it, category.begin()); it != category.end(); ++it) {
    TCachedToken< CScannableObjectInfo > scan = gpSimplePool->GetObj(SObjectTag('SCAN', it->first));
    mCurViewScans.push_back(SArticle(scan, rstl::optional_object_null()));
  }

  PumpArticleLoad();
  UpdateRightTitles();
}

bool CLogBookScreen::ShouldLeftTableAdvance() {
  if (!mLoaded || mCurViewScans.empty()) {
    return false;
  }

  return IsScanCategoryReady(static_cast< CWorldSaveGameInfo::EScanCategory >(
      mTablegroup_leftlog->GetUserSelection() + 1));
}

bool CLogBookScreen::ShouldRightTableAdvance() {
  SArticle& article = mCurViewScans[mRightSel];
  if (!article.first.TryCache()) {
    return false;
  }
  if (!article.second) {
    return false;
  }
  if (!article.second->TryCache()) {
    return false;
  }

  if (!mHandledInput &&
      mScanCompletes[mTablegroup_leftlog->GetUserSelection()][mRightSel].second) {
    return true;
  }
  return false;
}

uint CLogBookScreen::GetRightTableCount() const { return mCurViewScans.size(); }

void CLogBookScreen::RightTableSelectionChanged(int oldSel, int newSel) { UpdateRightTitles(); }

void CLogBookScreen::InitializeLogBook() {
  const CStateManager& mgr = mMgr;
  for (int i = 0; i < 5; ++i) {
    mScanCompletes[i].reserve(gpMemoryCard->GetScanCategoryCount(
        static_cast< CWorldSaveGameInfo::EScanCategory >(i + 1)));
  }

  const rstl::vector< CMemoryCard::ScanState >& states = gpMemoryCard->GetScanStates();
  const CPlayerState& playerState = *mgr.GetPlayerState();
  for (AUTO(it, states.begin()); it != states.end(); ++it) {
    if (it->second != CWorldSaveGameInfo::kSC_None) {
      CAssetId scan = it->first;
      CScanCategory& category = mScanCompletes[it->second - 1];
      bool complete = IsScanComplete(static_cast< CWorldSaveGameInfo::EScanCategory >(it->second),
                                     scan, playerState);
      category.push_back(SScanComplete(scan, complete));
    }
  }

  rstl::sort(mScanCompletes[4].begin(), mScanCompletes[4].end(), skArtifactScanSorter);

  for (int i = 0; i < mScanCompletes.size(); ++i) {
    const CScanCategory& category = mScanCompletes[i];
    int count = rstl::min_val(5, category.size());
    CArticleList& articles = mViewScans[i];
    articles.reserve(count);

    for (int j = 0; j < count; ++j) {
      articles.push_back(SArticle(gpSimplePool->GetObj(SObjectTag('SCAN', category[j].first)),
                                  rstl::optional_object_null()));
      articles[j].first.Lock();
    }
  }
}

void CLogBookScreen::PumpArticleLoad() {
  mLoaded = true;
  for (AUTO(category, mViewScans.begin()); category != mViewScans.end(); ++category) {
    for (AUTO(it, category->begin()); it != category->end(); ++it) {
      if (it->first.TryCache()) {
        rstl::optional_object< TCachedToken< CStringTable > >& str = it->second;
        if (str) {
          str->TryCache();
        } else {
          str = TCachedToken< CStringTable >(
              gpSimplePool->GetObj(SObjectTag('STRG', it->first.GetObject()->GetStringTableId())));
          str->Lock();
          mLoaded = false;
        }
      } else {
        mLoaded = false;
      }
    }
  }

  int remaining = 6;
  for (AUTO(it, mCurViewScans.begin()); it != mCurViewScans.end(); ++it) {
    if (it->first.IsLoaded()) {
      it->first.Lock();
      it->first.TryCache();
      rstl::optional_object< TCachedToken< CStringTable > >& str = it->second;
      if (!str) {
        str = TCachedToken< CStringTable >(
            gpSimplePool->GetObj(SObjectTag('STRG', it->first.GetObject()->GetStringTableId())));
        str->Lock();
        --remaining;
      }
    } else if (it->first.IsLocked()) {
      --remaining;
    }

    if (remaining == 0) {
      break;
    }
  }

  int articleIdx = mFirstViewRightSel;
  if (!mCurViewScans.empty()) {
    while (remaining > 0) {
      const int cur = articleIdx;
      if (!mCurViewScans[cur].first.IsLocked()) {
        mCurViewScans[cur].first.Lock();
        --remaining;
      }
      articleIdx = NextSurroundingArticleIndex(cur);
      if (articleIdx == -1) {
        break;
      }
    }
  }

  for (int i = 0; i < mCurViewScans.size(); ++i) {
    if (mCurViewScans[i].first.TryCache()) {
      rstl::optional_object< TCachedToken< CStringTable > >& str = mCurViewScans[i].second;
      if (str && str->TryCache()) {
        UpdateRightTitles();
        UpdateBodyText();
      }
    }
  }
}

int CLogBookScreen::NextSurroundingArticleIndex(int cur) {
  if (cur < mFirstViewRightSel) {
    int next = mFirstViewRightSel - cur + 6;
    next += mFirstViewRightSel;
    return next < mCurViewScans.size() ? next : cur - 1;
  }

  if (cur < mFirstViewRightSel + 6) {
    if (cur + 1 < mCurViewScans.size()) {
      return cur + 1;
    }
    if (mFirstViewRightSel == 0) {
      return -1;
    }
    return mFirstViewRightSel - 1;
  }

  int next = mFirstViewRightSel - (cur - (mFirstViewRightSel + 5));
  if (next >= 0) {
    return next;
  }
  if (cur >= mCurViewScans.size() - 1) {
    return -1;
  }
  return cur + 1;
}

void CLogBookScreen::UpdateBodyImagesAndText() {
  const CScannableObjectInfo* const scan = mCurViewScans[mRightSel].first.GetObject();
  for (int i = 0; i < mImagePanes.size(); ++i) {
    CAuiImagePane* pane = mImagePanes[i];
    pane->SetTextureID0(kInvalidAssetId, gpSimplePool);
    pane->SetAnimationParms(CVector2f::Zero(), 0.f, 0.f);
  }

  for (int i = 0; i < 4; ++i) {
    const CScannableObjectInfo::SBucket& bucket = scan->GetBucket(i);
    if (bucket.mImagePos != CScannableObjectInfo::kPT_Invalid) {
      CAuiImagePane* pane = mImagePanes[bucket.mImagePos];
      if (bucket.mInterval > 0.f) {
        pane->SetAnimationParms(CVector2f(bucket.mSize.GetX(), bucket.mSize.GetY()),
                                bucket.mInterval, bucket.mFadeDuration);
      }
      pane->SetTextureID0(bucket.mTexture, gpSimplePool);
      pane->SetFlashFactor(0.f);
    }
  }

  mExitTextScroll = false;
  UpdateBodyText();
}

void CLogBookScreen::UpdateBodyText() {
  if (mMode != kM_TextScroll) {
    mTextpane_body->TextSupport().SetText(rstl::wstring_l(L""));
    return;
  }

  rstl::optional_object< TCachedToken< CStringTable > >& str =
      mCurViewScans[mRightSel].second;
  if (str && str->TryCache() &&
      static_cast< int >(mTextpane_body->TextSupport().GetText().size()) == 0) {
    const CStringTable* const table = str->GetObject();
    rstl::wstring text = table->GetString(0);
    if (table->GetStringCount() > 2) {
      text.append(L"\n\n", -1);
      text.append(table->GetString(2), -1);
    }

    if (IsArtifactCategorySelected()) {
      int headIdx = GetSelectedArtifactHeadScanIndex();
      if (headIdx >= 0) {
        const CPlayerState& playerState = *gpGameState->GetPlayerState();
        if (playerState.HasPowerUp(
                static_cast< CPlayerState::EItemType >(headIdx + CPlayerState::kIT_Truth))) {
          text = rstl::wstring_l(L"\n\n\n\n\n\n") + gpStringTable->GetString(105);
        }
      }
    }

    mTextpane_body->TextSupport().SetText(text, true);
  }
}

void CLogBookScreen::UpdateRightTitles() {
  const CScanCategory& category = mScanCompletes[mTablegroup_leftlog->GetUserSelection()];
  for (int i = 0; i < mTextpane_titles.size(); ++i) {
    int scanIdx = mFirstViewRightSel + i;
    rstl::wstring text;
    if (scanIdx < mCurViewScans.size()) {
      const rstl::optional_object< TCachedToken< CStringTable > >& str =
          mCurViewScans[scanIdx].second;
      if (str && str->GetObject()) {
        if (category[scanIdx].second) {
          if (str->GetObject()->GetStringCount() > 1) {
            text = str->GetObject()->GetString(1);
          } else {
            text = rstl::wstring_l(L"No Title!");
          }
        } else {
          text = rstl::wstring_l(L"??????");
        }
      }

      if (static_cast< int >(text.size()) == 0) {
        text = rstl::wstring_l(L"........");
      }
    }
    mTextpane_titles[i]->TextSupport().SetText(text);
  }

  int rightSelRem;
  int rightSelMod = mFirstViewRightSel % 5;
  rightSelRem = 5 - rightSelMod;
  for (int i = 0; i < mModel_titles.size(); ++i) {
    CGuiModel* model = mModel_titles[i];
    int row = rightSelRem;
    if (i >= rightSelMod) {
      row -= 5;
    }
    const float zOff = mHighlightPitch * row;
    model->SetO2PTransform(CTransform4f::Translate(0.f, 0.f, zOff) * model->GetTransform());
  }
}

void CLogBookScreen::TransitioningAway() { mLeavePauseState = kLPS_LeavingPause; }

bool CLogBookScreen::InputDisabled() const { return mLeavePauseState == kLPS_LeavingPause; }

bool CLogBookScreen::IsScanCategoryReady(CWorldSaveGameInfo::EScanCategory category) {
  const CPlayerState& playerState = *mMgr.GetPlayerState();
  const rstl::vector< CMemoryCard::ScanState >& states = gpMemoryCard->GetScanStates();
  for (AUTO(it, states.begin()); it != states.end(); ++it) {
    const uint& currentCategory = it->second;
    if (currentCategory == category &&
        IsScanComplete(static_cast< CWorldSaveGameInfo::EScanCategory >(it->second), it->first,
                       playerState)) {
      return true;
    }
  }

  return false;
}

bool CLogBookScreen::IsScanComplete(CWorldSaveGameInfo::EScanCategory category, CAssetId scan,
                                    const CPlayerState& playerState) {
  float time = playerState.GetScanTime(scan);
  if (category == CWorldSaveGameInfo::kSC_Artifact) {
    return time >= 0.5f;
  }
  return time >= 1.f;
}
