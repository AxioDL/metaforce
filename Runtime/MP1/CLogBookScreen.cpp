#include "CLogBookScreen.hpp"
#include "GuiSys/CGuiModel.hpp"
#include "GuiSys/CGuiTableGroup.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "GuiSys/CAuiImagePane.hpp"
#include "MP1.hpp"

namespace urde
{
namespace MP1
{

CLogBookScreen::CLogBookScreen(const CStateManager& mgr, CGuiFrame& frame,
                               const CStringTable& pauseStrg)
: CPauseScreenBase(mgr, frame, pauseStrg)
{
    x19c_scanCompletes.resize(5);
    x200_viewScans.resize(5);
    x258_artifactDoll = std::make_unique<CArtifactDoll>();
    CMain::EnsureWorldPaksReady();
    InitializeLogBook();
}

CLogBookScreen::~CLogBookScreen()
{
    CArtifactDoll::CompleteArtifactHeadScan(x4_mgr);
    for (CGuiModel* model : x144_model_titles)
        model->SetLocalTransform(model->GetTransform());
    CMain::EnsureWorldPakReady(g_GameState->CurrentWorldAssetId());
}

bool CLogBookScreen::IsScanComplete(CSaveWorld::EScanCategory category, ResId scan,
                                    const CPlayerState& playerState)
{
    float time = playerState.GetScanTime(scan);
    if (category == CSaveWorld::EScanCategory::Artifact)
        return time >= 0.5f;
    else
        return time >= 1.f;
}

void CLogBookScreen::InitializeLogBook()
{
    for (int i=0 ; i<5 ; ++i)
        x19c_scanCompletes[i].reserve(g_MemoryCardSys->GetScanCategoryCount(CSaveWorld::EScanCategory(i + 1)));

    CPlayerState& playerState = *x4_mgr.GetPlayerState();
    for (const std::pair<ResId, CSaveWorld::EScanCategory>& scanState : g_MemoryCardSys->GetScanStates())
    {
        if (scanState.second == CSaveWorld::EScanCategory::None)
            continue;
        bool complete = IsScanComplete(scanState.second, scanState.first, playerState);
        x19c_scanCompletes[int(scanState.second) - 1].push_back(std::make_pair(scanState.first, complete));
    }

    std::sort(x19c_scanCompletes[4].begin(), x19c_scanCompletes[4].end(),
    [](const std::pair<ResId, bool>& a, std::pair<ResId, bool>& b)
    {
        return CArtifactDoll::GetArtifactHeadScanIndex(a.first) <
               CArtifactDoll::GetArtifactHeadScanIndex(b.first);
    });

    auto viewIt = x200_viewScans.begin();
    for (std::vector<std::pair<ResId, bool>>& category : x19c_scanCompletes)
    {
        std::vector<std::pair<TLockedToken<CScannableObjectInfo>,
                              TLockedToken<CStringTable>>>& viewScans = *viewIt++;
        size_t viewScanCount = std::min(category.size(), size_t(5));
        viewScans.reserve(viewScanCount);
        for (size_t i=0 ; i<viewScanCount ; ++i)
            viewScans.push_back(std::make_pair(g_SimplePool->GetObj({FOURCC('SCAN'), category[i].first}),
                                               TLockedToken<CStringTable>{}));
    }
}

void CLogBookScreen::UpdateRightTitles()
{
    std::vector<std::pair<ResId, bool>>& category =
        x19c_scanCompletes[x70_tablegroup_leftlog->GetUserSelection()];
    for (int i=0 ; xd8_textpane_titles.size() ; ++i)
    {
        std::u16string string;
        size_t scanIndex = x18_firstViewRightSel + i;
        if (scanIndex < x1f0_curViewScans.size())
        {
            std::pair<TCachedToken<CScannableObjectInfo>,
                      TCachedToken<CStringTable>>& scan = x1f0_curViewScans[scanIndex];
            if (scan.second && scan.second.IsLoaded())
            {
                if (category[scanIndex].second)
                {
                    if (scan.second->GetStringCount() > 1)
                        string = scan.second->GetString(1);
                    else
                        string = u"No Title!";
                }
                else
                {
                    string = u"??????";
                }
            }

            if (string.empty())
                string = u"........";
        }
        xd8_textpane_titles[i]->TextSupport()->SetText(string);
    }

    int rightSelMod = x18_firstViewRightSel % 5;
    int rightSelRem = 5 - rightSelMod;
    for (int i=0 ; i<x144_model_titles.size() ; ++i)
    {
        float zOff = ((i >= rightSelMod) ? rightSelRem - 5 : rightSelRem) * x38_highlightPitch;
        x144_model_titles[i]->SetLocalTransform(
            zeus::CTransform::Translate(0.f, 0.f, zOff) * x144_model_titles[i]->GetTransform());
    }
}

void CLogBookScreen::PumpArticleLoad()
{
    x260_24_loaded = true;
    for (std::vector<std::pair<TLockedToken<CScannableObjectInfo>,
         TLockedToken<CStringTable>>>& category : x200_viewScans)
    {
        for (std::pair<TLockedToken<CScannableObjectInfo>,
             TLockedToken<CStringTable>>& scan : category)
        {
            if (scan.first.IsLoaded())
            {
                if (!scan.second)
                {
                    scan.second = g_SimplePool->GetObj({FOURCC('STRG'), scan.first->GetStringTableId()});
                    x260_24_loaded = false;
                }
            }
            else
            {
                x260_24_loaded = false;
            }
        }
    }

    int rem = 6;
    for (std::pair<TCachedToken<CScannableObjectInfo>,
         TCachedToken<CStringTable>>& scan : x1f0_curViewScans)
    {
        if (scan.first.IsLoaded())
        {
            if (!scan.second)
            {
                scan.second = g_SimplePool->GetObj({FOURCC('STRG'), scan.first->GetStringTableId()});
                scan.second.Lock();
            }
        }
        if (--rem == 0)
            break;
    }

    if (x1f0_curViewScans.size())
    {
        int articleIdx = x18_firstViewRightSel;
        while (rem > 0)
        {
            x1f0_curViewScans[articleIdx].first.Lock();
            articleIdx = NextSurroundingArticleIndex(articleIdx);
            --rem;
        }
    }

    for (std::pair<TCachedToken<CScannableObjectInfo>,
         TCachedToken<CStringTable>>& scan : x1f0_curViewScans)
    {
        if (scan.first.IsLoaded())
        {
            if (scan.second && scan.second.IsLoaded())
            {
                UpdateRightTitles();
                UpdateBodyText();
            }
        }
    }
}

bool CLogBookScreen::IsScanCategoryReady(CSaveWorld::EScanCategory category) const
{
    CPlayerState& playerState = *x4_mgr.GetPlayerState();
    for (const std::pair<ResId, CSaveWorld::EScanCategory>& scanState : g_MemoryCardSys->GetScanStates())
    {
        if (scanState.second != category)
            continue;
        if (IsScanComplete(scanState.second, scanState.first, playerState))
            return true;
    }
    return false;
}

void CLogBookScreen::UpdateBodyText()
{
    if (x10_mode != EMode::TextScroll)
    {
        x174_textpane_body->TextSupport()->SetText(u"");
        return;
    }

    TCachedToken<CStringTable>& str = x1f0_curViewScans[x1c_rightSel].second;
    if (str && str.IsLoaded())
    {
        std::u16string accumStr = str->GetString(0);
        if (str->GetStringCount() > 2)
        {
            accumStr += u"\n\n";
            accumStr += str->GetString(2);
        }

        if (IsArtifactCategorySelected())
        {
            int headIdx = GetSelectedArtifactHeadScanIndex();
            if (headIdx >= 0 && g_GameState->GetPlayerState()->HasPowerUp(CPlayerState::EItemType(headIdx + 29)))
                 accumStr = std::u16string(u"\n\n\n\n\n\n") + g_MainStringTable->GetString(105);
        }

        x174_textpane_body->TextSupport()->SetText(accumStr, true);
    }
}

void CLogBookScreen::UpdateBodyImagesAndText()
{
    const CScannableObjectInfo* scan = x1f0_curViewScans[x1c_rightSel].first.GetObj();
    for (CAuiImagePane* pane : xf0_imagePanes)
    {
        pane->SetTextureID0(-1, g_SimplePool);
        pane->SetAnimationParms(zeus::CVector2f::skZero, 0.f, 0.f);
    }

    for (int i=0 ; i<4 ; ++i)
    {
        const CScannableObjectInfo::SBucket& bucket = scan->GetBucket(i);
        if (bucket.x8_imagePos == -1)
            continue;
        CAuiImagePane* pane = xf0_imagePanes[bucket.x8_imagePos];
        if (bucket.x14_interval > 0.f)
        {
            pane->SetAnimationParms(zeus::CVector2f(bucket.xc_size.x, bucket.xc_size.y),
                                    bucket.x14_interval, bucket.x18_fadeDuration);
        }
        pane->SetTextureID0(bucket.x0_texture, g_SimplePool);
        pane->ResetInterp();
    }

    x260_26_exitTextScroll = false;
    UpdateBodyText();
}

int CLogBookScreen::NextSurroundingArticleIndex(int cur) const
{
    if (cur < x18_firstViewRightSel)
    {
        int tmp = x18_firstViewRightSel + (x18_firstViewRightSel - cur + 6);
        if (tmp >= x1f0_curViewScans.size())
            return cur - 1;
        else
            return tmp;
    }

    if (cur < x18_firstViewRightSel + 6)
    {
        if (cur + 1 < x1f0_curViewScans.size())
            return cur + 1;
        if (x18_firstViewRightSel == 0)
            return -1;
        return x18_firstViewRightSel - 1;
    }

    int tmp = x18_firstViewRightSel - (cur - (x18_firstViewRightSel + 5));
    if (tmp >= 0)
        return tmp;

    if (cur >= x1f0_curViewScans.size() - 1)
        return -1;
    return cur + 1;
}

bool CLogBookScreen::IsArtifactCategorySelected() const
{
    return x70_tablegroup_leftlog->GetUserSelection() == 4;
}

int CLogBookScreen::GetSelectedArtifactHeadScanIndex() const
{
    auto& category = x19c_scanCompletes[x70_tablegroup_leftlog->GetUserSelection()];
    if (x1c_rightSel < category.size())
        return CArtifactDoll::GetArtifactHeadScanIndex(category[x1c_rightSel].first);
    return -1;
}

bool CLogBookScreen::InputDisabled() const
{
    return x25c_leavePauseState == ELeavePauseState::LeavingPause;
}

void CLogBookScreen::TransitioningAway()
{
    x25c_leavePauseState = ELeavePauseState::LeavingPause;
}

void CLogBookScreen::Update(float dt, CRandom16& rand, CArchitectureQueue& archQueue)
{
    CPauseScreenBase::Update(dt, rand, archQueue);
    x258_artifactDoll->Update(dt, x4_mgr);
    PumpArticleLoad();

    if (x10_mode == EMode::TextScroll)
    {
        if (x260_25_inTextScroll)
            x254_viewInterp = std::min(x254_viewInterp + 4.f * dt, 1.f);
        else
            x254_viewInterp = std::max(0.f, x254_viewInterp - 4.f * dt);

        zeus::CColor color(1.f, x254_viewInterp);
        x74_basewidget_leftguages->SetColor(color);
        x88_basewidget_rightguages->SetColor(color);

        zeus::CColor invColor(1.f, 1.f - x254_viewInterp);
        x70_tablegroup_leftlog->SetColor(invColor);
        x84_tablegroup_rightlog->SetColor(invColor);
        x17c_model_textalpha->SetColor(invColor);
        x174_textpane_body->SetColor(invColor);

        for (CAuiImagePane* pane : xf0_imagePanes)
            pane->SetInverseAlpha(1.f - x254_viewInterp);

        if (x254_viewInterp == 0.f && x25c_leavePauseState == ELeavePauseState::InPause)
            ChangeMode(EMode::RightTable);
    }

    if (x25c_leavePauseState == ELeavePauseState::LeavingPause && x254_viewInterp == 0.f)
        x25c_leavePauseState = ELeavePauseState::LeftPause;
}

void CLogBookScreen::Touch()
{
    CPauseScreenBase::Touch();
    x258_artifactDoll->Touch();
}

void CLogBookScreen::ProcessControllerInput(const CFinalInput& input)
{
    x260_25_inTextScroll = false;
    if (x25c_leavePauseState == ELeavePauseState::LeftPause)
        return;

    if (x10_mode == EMode::TextScroll)
    {
        int oldPage = x174_textpane_body->TextSupport()->GetPageCounter();
        int newPage = oldPage;
        int pageCount = x174_textpane_body->TextSupport()->GetTotalPageCount();
        bool lastPage = (pageCount - 1) == oldPage;
        if (pageCount != -1)
        {
            if (input.PLAUp())
                newPage = std::max(oldPage - 1, 0);
            else if (input.PLADown() || (input.PA() && !lastPage))
                newPage = std::min(oldPage + 1, pageCount - 1);
            x174_textpane_body->TextSupport()->SetPage(newPage);
            if (oldPage != newPage)
                CSfxManager::SfxStart(1444, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
            x198_28_pulseTextArrowTop = newPage > 1;
            x198_29_pulseTextArrowBottom = !lastPage;
        }
        else
        {
            x198_29_pulseTextArrowBottom = false;
            x198_28_pulseTextArrowTop = false;
        }

        if (!x260_26_exitTextScroll)
            x260_26_exitTextScroll = input.PB() || (input.PA() && lastPage);

        if (g_tweakGui->GetLatchArticleText())
            x260_25_inTextScroll = !x260_26_exitTextScroll;
        else
            x260_25_inTextScroll = input.DA();
    }
    else
    {
        x198_29_pulseTextArrowBottom = false;
        x198_28_pulseTextArrowTop = false;
    }

    if (x25c_leavePauseState == ELeavePauseState::LeavingPause)
        x260_25_inTextScroll = false;

    CPauseScreenBase::ProcessControllerInput(input);
}

void CLogBookScreen::Draw(float transInterp, float totalAlpha, float yOff)
{
    CPauseScreenBase::Draw(transInterp, totalAlpha, yOff);
    bool artifactSel = x10_mode == EMode::RightTable && IsArtifactCategorySelected();
    x258_artifactDoll->Draw(transInterp * (1.f - x254_viewInterp), x4_mgr, artifactSel,
                            GetSelectedArtifactHeadScanIndex());
}

bool CLogBookScreen::VReady() const
{
    return true;
}

void CLogBookScreen::VActivate()
{
    for (int i=0 ; i<5 ; ++i)
    {
        if (IsScanCategoryReady(CSaveWorld::EScanCategory(i + 1)))
        {
            xa8_textpane_categories[i]->TextSupport()->SetText(xc_pauseStrg.GetString(i + 1));
        }
        else
        {
            xa8_textpane_categories[i]->TextSupport()->SetText(u"??????");
            x70_tablegroup_leftlog->GetWorkerWidget(i)->SetIsSelectable(false);
        }
    }

    x178_textpane_title->TextSupport()->SetText(xc_pauseStrg.GetString(0));

    for (int i=0 ; i<5 ; ++i)
        x70_tablegroup_leftlog->GetWorkerWidget(i)->SetIsSelectable(false);
}

void CLogBookScreen::RightTableSelectionChanged(int selBegin, int selEnd)
{
    UpdateRightTitles();
}

void CLogBookScreen::ChangedMode(EMode oldMode)
{
    if (oldMode == EMode::TextScroll)
    {
        x74_basewidget_leftguages->SetVisibility(false, ETraversalMode::Children);
        x88_basewidget_rightguages->SetVisibility(false, ETraversalMode::Children);
        UpdateBodyText();
        x174_textpane_body->TextSupport()->SetPage(0);
    }
    else if (x10_mode == EMode::TextScroll)
    {
        x74_basewidget_leftguages->SetVisibility(true, ETraversalMode::Children);
        x88_basewidget_rightguages->SetVisibility(true, ETraversalMode::Children);
        x260_25_inTextScroll = true;
        UpdateBodyImagesAndText();
    }
}

void CLogBookScreen::UpdateRightTable()
{
    CPauseScreenBase::UpdateRightTable();
    x1f0_curViewScans.clear();
    std::vector<std::pair<ResId, bool>>& category = x19c_scanCompletes[x70_tablegroup_leftlog->GetUserSelection()];
    x1f0_curViewScans.reserve(category.size());
    for (std::pair<ResId, bool>& scan : category)
        x1f0_curViewScans.push_back(std::make_pair(g_SimplePool->GetObj({FOURCC('SCAN'), scan.first}),
                                                   TLockedToken<CStringTable>{}));

    PumpArticleLoad();
    UpdateRightTitles();
}

bool CLogBookScreen::ShouldLeftTableAdvance() const
{
    if (!x260_24_loaded || x1f0_curViewScans.empty())
        return false;
    return IsScanCategoryReady(CSaveWorld::EScanCategory(x70_tablegroup_leftlog->GetUserSelection() + 1));
}

bool CLogBookScreen::ShouldRightTableAdvance() const
{
    const std::pair<TLockedToken<CScannableObjectInfo>,
                    TLockedToken<CStringTable>>& scan = x1f0_curViewScans[x1c_rightSel];
    return scan.first.IsLoaded() && scan.second.IsLoaded();
}

u32 CLogBookScreen::GetRightTableCount() const
{
    return x1f0_curViewScans.size();
}

}
}
