#include "Runtime/MP1/CLogBookScreen.hpp"

#include <algorithm>

#include "Runtime/CPlayerState.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GuiSys/CAuiImagePane.hpp"
#include "Runtime/GuiSys/CGuiModel.hpp"
#include "Runtime/GuiSys/CGuiTableGroup.hpp"
#include "Runtime/GuiSys/CGuiTextPane.hpp"
#include "Runtime/MP1/CArtifactDoll.hpp"
#include "Runtime/MP1/MP1.hpp"

namespace urde::MP1 {

CLogBookScreen::CLogBookScreen(const CStateManager& mgr, CGuiFrame& frame, const CStringTable& pauseStrg)
: CPauseScreenBase(mgr, frame, pauseStrg, true) {
  x19c_scanCompletes.resize(5);
  x200_viewScans.resize(5);
  x258_artifactDoll = std::make_unique<CArtifactDoll>();
  CMain::EnsureWorldPaksReady();
  InitializeLogBook();
}

CLogBookScreen::~CLogBookScreen() {
  CArtifactDoll::CompleteArtifactHeadScan(x4_mgr);
  for (CGuiModel* model : x144_model_titles)
    model->SetLocalTransform(model->GetTransform());
  CMain::EnsureWorldPakReady(g_GameState->CurrentWorldAssetId());
}

bool CLogBookScreen::IsScanComplete(CSaveWorld::EScanCategory category, CAssetId scan,
                                    const CPlayerState& playerState) {
  const float time = playerState.GetScanTime(scan);
  if (category == CSaveWorld::EScanCategory::Artifact) {
    return time >= 0.5f;
  } else {
    return time >= 1.f;
  }
}

void CLogBookScreen::InitializeLogBook() {
  for (size_t i = 0; i < x19c_scanCompletes.size(); ++i) {
    x19c_scanCompletes[i].reserve(g_MemoryCardSys->GetScanCategoryCount(CSaveWorld::EScanCategory(i + 1)));
  }

  CPlayerState& playerState = *x4_mgr.GetPlayerState();
  for (const auto& [scanId, scanCategory] : g_MemoryCardSys->GetScanStates()) {
    if (scanCategory == CSaveWorld::EScanCategory::None) {
      continue;
    }

    const bool complete = IsScanComplete(scanCategory, scanId, playerState);
    x19c_scanCompletes[int(scanCategory) - 1].emplace_back(scanId, complete);
  }

  std::sort(x19c_scanCompletes[4].begin(), x19c_scanCompletes[4].end(),
            [](const std::pair<CAssetId, bool>& a, std::pair<CAssetId, bool>& b) {
              return CArtifactDoll::GetArtifactHeadScanIndex(a.first) <
                     CArtifactDoll::GetArtifactHeadScanIndex(b.first);
            });

  auto viewIt = x200_viewScans.begin();
  for (const std::vector<std::pair<CAssetId, bool>>& category : x19c_scanCompletes) {
    const size_t viewScanCount = std::min(category.size(), size_t(5));
    auto& viewScans = *viewIt++;
    viewScans.reserve(viewScanCount);

    for (size_t i = 0; i < viewScanCount; ++i) {
      viewScans.emplace_back(g_SimplePool->GetObj({FOURCC('SCAN'), category[i].first}), TLockedToken<CStringTable>{});
    }
  }
}

void CLogBookScreen::UpdateRightTitles() {
  const std::vector<std::pair<CAssetId, bool>>& category = x19c_scanCompletes[x70_tablegroup_leftlog->GetUserSelection()];
  for (size_t i = 0; i < xd8_textpane_titles.size(); ++i) {
    std::u16string string;
    const auto scanIndex = size_t(x18_firstViewRightSel) + i;

    if (scanIndex < x1f0_curViewScans.size()) {
      const auto& scan = x1f0_curViewScans[scanIndex];

      if (scan.second && scan.second.IsLoaded()) {
        if (category[scanIndex].second) {
          if (scan.second->GetStringCount() > 1) {
            string = scan.second->GetString(1);
          } else {
            string = u"No Title!";
          }
        } else {
          string = u"??????";
        }
      }

      if (string.empty()) {
        string = u"........";
      }
    }

    xd8_textpane_titles[i]->TextSupport().SetText(string);
  }

  const int rightSelMod = x18_firstViewRightSel % 5;
  const int rightSelRem = 5 - rightSelMod;
  for (size_t i = 0; i < x144_model_titles.size(); ++i) {
    const float zOff = float(((int(i) >= rightSelMod) ? rightSelRem - 5 : rightSelRem)) * x38_highlightPitch;
    x144_model_titles[i]->SetLocalTransform(zeus::CTransform::Translate(0.f, 0.f, zOff) *
                                            x144_model_titles[i]->GetTransform());
  }
}

void CLogBookScreen::PumpArticleLoad() {
  x260_24_loaded = true;
  for (auto& category : x200_viewScans) {
    for (auto& [scanInfo, stringTable] : category) {
      if (scanInfo.IsLoaded()) {
        if (!stringTable) {
          stringTable = g_SimplePool->GetObj({FOURCC('STRG'), scanInfo->GetStringTableId()});
          x260_24_loaded = false;
        }
      } else {
        x260_24_loaded = false;
      }
    }
  }

  int rem = 6;
  for (auto& [scanInfo, stringTable] : x1f0_curViewScans) {
    if (scanInfo.IsLoaded()) {
      if (!stringTable) {
        stringTable = g_SimplePool->GetObj({FOURCC('STRG'), scanInfo->GetStringTableId()});
        stringTable.Lock();
        --rem;
      }
    } else if (scanInfo.IsLocked()) {
      --rem;
    }
    if (rem == 0)
      break;
  }

  if (x1f0_curViewScans.size()) {
    int articleIdx = x18_firstViewRightSel;
    while (rem > 0) {
      x1f0_curViewScans[articleIdx].first.Lock();
      articleIdx = NextSurroundingArticleIndex(articleIdx);
      if (articleIdx == -1)
        break;
      --rem;
    }
  }

  for (const auto& [scanInfo, stringTable] : x1f0_curViewScans) {
    if (!scanInfo.IsLoaded()) {
      continue;
    }
    if (!stringTable || !stringTable.IsLoaded()) {
      continue;
    }

    UpdateRightTitles();
    UpdateBodyText();
  }
}

bool CLogBookScreen::IsScanCategoryReady(CSaveWorld::EScanCategory category) const {
  const CPlayerState& playerState = *x4_mgr.GetPlayerState();
  const auto& scanState = g_MemoryCardSys->GetScanStates();

  return std::any_of(scanState.cbegin(), scanState.cend(), [category, &playerState](const auto& state) {
    if (state.second != category) {
      return false;
    }

    return IsScanComplete(state.second, state.first, playerState);
  });
}

void CLogBookScreen::UpdateBodyText() {
  if (x10_mode != EMode::TextScroll) {
    x174_textpane_body->TextSupport().SetText(u"");
    return;
  }

  const TCachedToken<CStringTable>& str = x1f0_curViewScans[x1c_rightSel].second;
  if (!str || !str.IsLoaded()) {
    return;
  }

  std::u16string accumStr = str->GetString(0);
  if (str->GetStringCount() > 2) {
    accumStr += u"\n\n";
    accumStr += str->GetString(2);
  }

  if (IsArtifactCategorySelected()) {
    const int headIdx = GetSelectedArtifactHeadScanIndex();
    if (headIdx >= 0 && g_GameState->GetPlayerState()->HasPowerUp(CPlayerState::EItemType(headIdx + 29))) {
      if (g_Main->IsUSA() && !g_Main->IsTrilogy()) {
        accumStr = std::u16string(u"\n\n\n\n\n\n").append(g_MainStringTable->GetString(105));
      } else {
        accumStr = std::u16string(u"\n\n\n\n\n\n").append(g_MainStringTable->GetString(107));
      }
    }
  }

  x174_textpane_body->TextSupport().SetText(accumStr, true);
}

void CLogBookScreen::UpdateBodyImagesAndText() {
  const CScannableObjectInfo* scan = x1f0_curViewScans[x1c_rightSel].first.GetObj();
  for (CAuiImagePane* pane : xf0_imagePanes) {
    pane->SetTextureID0(-1, g_SimplePool);
    pane->SetAnimationParms(zeus::skZero2f, 0.f, 0.f);
  }

  for (size_t i = 0; i < CScannableObjectInfo::NumBuckets; ++i) {
    const CScannableObjectInfo::SBucket& bucket = scan->GetBucket(i);
    if (bucket.x8_imagePos == UINT32_MAX) {
      continue;
    }
    CAuiImagePane* pane = xf0_imagePanes[bucket.x8_imagePos];
    if (bucket.x14_interval > 0.f) {
      pane->SetAnimationParms(zeus::CVector2f(bucket.xc_size.x, bucket.xc_size.y), bucket.x14_interval,
                              bucket.x18_fadeDuration);
    }
    pane->SetTextureID0(bucket.x0_texture, g_SimplePool);
    pane->SetFlashFactor(0.f);
  }

  x260_26_exitTextScroll = false;
  UpdateBodyText();
}

int CLogBookScreen::NextSurroundingArticleIndex(int cur) const {
  if (cur < x18_firstViewRightSel) {
    const int tmp = x18_firstViewRightSel + (x18_firstViewRightSel - cur + 6);

    if (tmp >= int(x1f0_curViewScans.size())) {
      return cur - 1;
    }

    return tmp;
  }

  if (cur < x18_firstViewRightSel + 6) {
    if (cur + 1 < int(x1f0_curViewScans.size())) {
      return cur + 1;
    }

    if (x18_firstViewRightSel == 0) {
      return -1;
    }

    return x18_firstViewRightSel - 1;
  }

  const int tmp = x18_firstViewRightSel - (cur - (x18_firstViewRightSel + 5));
  if (tmp >= 0) {
    return tmp;
  }

  if (cur >= int(x1f0_curViewScans.size()) - 1) {
    return -1;
  }

  return cur + 1;
}

bool CLogBookScreen::IsArtifactCategorySelected() const { return x70_tablegroup_leftlog->GetUserSelection() == 4; }

int CLogBookScreen::GetSelectedArtifactHeadScanIndex() const {
  const auto& category = x19c_scanCompletes[x70_tablegroup_leftlog->GetUserSelection()];

  if (x1c_rightSel < int(category.size())) {
    return CArtifactDoll::GetArtifactHeadScanIndex(category[x1c_rightSel].first);
  }

  return -1;
}

bool CLogBookScreen::InputDisabled() const { return x25c_leavePauseState == ELeavePauseState::LeavingPause; }

void CLogBookScreen::TransitioningAway() { x25c_leavePauseState = ELeavePauseState::LeavingPause; }

void CLogBookScreen::Update(float dt, CRandom16& rand, CArchitectureQueue& archQueue) {
  CPauseScreenBase::Update(dt, rand, archQueue);
  x258_artifactDoll->Update(dt, x4_mgr);
  PumpArticleLoad();

  if (x10_mode == EMode::TextScroll) {
    if (x260_25_inTextScroll)
      x254_viewInterp = std::min(x254_viewInterp + 4.f * dt, 1.f);
    else
      x254_viewInterp = std::max(0.f, x254_viewInterp - 4.f * dt);

    if (x254_viewInterp == 0.f && x25c_leavePauseState == ELeavePauseState::InPause)
      ChangeMode(EMode::RightTable);
  } else {
    x254_viewInterp = std::max(0.f, x254_viewInterp - 4.f * dt);
  }

  zeus::CColor color(1.f, x254_viewInterp);
  x74_basewidget_leftguages->SetColor(color);
  x88_basewidget_rightguages->SetColor(color);

  zeus::CColor invColor(1.f, 1.f - x254_viewInterp);
  x70_tablegroup_leftlog->SetColor(invColor);
  x84_tablegroup_rightlog->SetColor(invColor);
  x17c_model_textalpha->SetColor(invColor);
  x174_textpane_body->SetColor(color);

  for (CAuiImagePane* pane : xf0_imagePanes)
    pane->SetDeResFactor(1.f - x254_viewInterp);

  if (x25c_leavePauseState == ELeavePauseState::LeavingPause && x254_viewInterp == 0.f)
    x25c_leavePauseState = ELeavePauseState::LeftPause;
}

void CLogBookScreen::Touch() {
  CPauseScreenBase::Touch();
  x258_artifactDoll->Touch();
}

void CLogBookScreen::ProcessControllerInput(const CFinalInput& input) {
  x260_25_inTextScroll = false;
  if (x25c_leavePauseState == ELeavePauseState::LeftPause)
    return;

  if (x10_mode == EMode::TextScroll) {
    int oldPage = x174_textpane_body->TextSupport().GetPageCounter();
    int newPage = oldPage;
    int pageCount = x174_textpane_body->TextSupport().GetTotalPageCount();
    bool lastPage = (pageCount - 1) == oldPage;
    if (pageCount != -1) {
      if (input.PLAUp() || m_bodyUpClicked)
        newPage = std::max(oldPage - 1, 0);
      else if (input.PLADown() || m_bodyDownClicked ||
      ((input.PA() || input.PSpecialKey(boo::ESpecialKey::Enter) || m_bodyClicked) && !lastPage))
        newPage = std::min(oldPage + 1, pageCount - 1);
      x174_textpane_body->TextSupport().SetPage(newPage);
      if (oldPage != newPage)
        CSfxManager::SfxStart(SFXui_pause_screen_next_page, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
      x198_28_pulseTextArrowTop = newPage > 0;
      x198_29_pulseTextArrowBottom = !lastPage;
    } else {
      x198_29_pulseTextArrowBottom = false;
      x198_28_pulseTextArrowTop = false;
    }

    if (!x260_26_exitTextScroll)
      x260_26_exitTextScroll = input.PB() || input.PSpecialKey(boo::ESpecialKey::Esc) ||
        ((input.PA() || input.PSpecialKey(boo::ESpecialKey::Enter) || m_bodyClicked) && lastPage);

    if (g_tweakGui->GetLatchArticleText())
      x260_25_inTextScroll = !x260_26_exitTextScroll;
    else
      x260_25_inTextScroll = input.DA();
  } else {
    x198_29_pulseTextArrowBottom = false;
    x198_28_pulseTextArrowTop = false;
  }

  if (x25c_leavePauseState == ELeavePauseState::LeavingPause)
    x260_25_inTextScroll = false;

  CPauseScreenBase::ProcessMouseInput(input, 0.f);
  CPauseScreenBase::ProcessControllerInput(input);
}

void CLogBookScreen::Draw(float transInterp, float totalAlpha, float yOff) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CInventoryScreen::Draw", zeus::skPurple);
  CPauseScreenBase::Draw(transInterp, totalAlpha, yOff);
  bool artifactSel = x10_mode == EMode::RightTable && IsArtifactCategorySelected();
  x258_artifactDoll->Draw(transInterp * (1.f - x254_viewInterp), x4_mgr, artifactSel,
                          GetSelectedArtifactHeadScanIndex());
}

bool CLogBookScreen::VReady() const { return true; }

void CLogBookScreen::VActivate() {
  for (int i = 0; i < int(xa8_textpane_categories.size()); ++i) {
    if (IsScanCategoryReady(CSaveWorld::EScanCategory(i + 1))) {
      xa8_textpane_categories[i]->TextSupport().SetText(xc_pauseStrg.GetString(i + 1));
    } else {
      xa8_textpane_categories[i]->TextSupport().SetText(u"??????");
      x70_tablegroup_leftlog->GetWorkerWidget(i)->SetIsSelectable(false);
    }
  }

  x178_textpane_title->TextSupport().SetText(xc_pauseStrg.GetString(0));

#if 0
    for (int i=5 ; i<5 ; ++i)
        x70_tablegroup_leftlog->GetWorkerWidget(i)->SetIsSelectable(false);
#endif
}

void CLogBookScreen::RightTableSelectionChanged(int oldSel, int newSel) { UpdateRightTitles(); }

void CLogBookScreen::ChangedMode(EMode oldMode) {
  if (oldMode == EMode::TextScroll) {
    x74_basewidget_leftguages->SetVisibility(false, ETraversalMode::Children);
    x88_basewidget_rightguages->SetVisibility(false, ETraversalMode::Children);
    UpdateBodyText();
    x174_textpane_body->TextSupport().SetPage(0);
  } else if (x10_mode == EMode::TextScroll) {
    x74_basewidget_leftguages->SetVisibility(true, ETraversalMode::Children);
    x88_basewidget_rightguages->SetVisibility(true, ETraversalMode::Children);
    x260_25_inTextScroll = true;
    UpdateBodyImagesAndText();
  }
}

void CLogBookScreen::UpdateRightTable() {
  CPauseScreenBase::UpdateRightTable();

  const auto& category = x19c_scanCompletes[x70_tablegroup_leftlog->GetUserSelection()];
  x1f0_curViewScans.clear();
  x1f0_curViewScans.reserve(category.size());
  for (const std::pair<CAssetId, bool>& scan : category) {
    x1f0_curViewScans.emplace_back(g_SimplePool->GetObj({FOURCC('SCAN'), scan.first}), TLockedToken<CStringTable>{});
  }

  PumpArticleLoad();
  UpdateRightTitles();
}

bool CLogBookScreen::ShouldLeftTableAdvance() const {
  if (!x260_24_loaded || x1f0_curViewScans.empty())
    return false;
  return IsScanCategoryReady(CSaveWorld::EScanCategory(x70_tablegroup_leftlog->GetUserSelection() + 1));
}

bool CLogBookScreen::ShouldRightTableAdvance() const {
  const auto& [info, stringTable] = x1f0_curViewScans[x1c_rightSel];
  return info.IsLoaded() && stringTable.IsLoaded();
}

u32 CLogBookScreen::GetRightTableCount() const { return x1f0_curViewScans.size(); }

} // namespace urde::MP1
