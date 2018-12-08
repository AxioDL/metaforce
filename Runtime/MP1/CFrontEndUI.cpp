#include "CFrontEndUI.hpp"
#include "CArchitectureMessage.hpp"
#include "CArchitectureQueue.hpp"
#include "CDvdFile.hpp"
#include "CSimplePool.hpp"
#include "GameGlobalObjects.hpp"
#include "MP1.hpp"
#include "CSlideShow.hpp"
#include "Audio/CSfxManager.hpp"
#include "Graphics/CMoviePlayer.hpp"
#include "CSaveGameScreen.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CStringTable.hpp"
#include "GuiSys/CGuiTableGroup.hpp"
#include "GuiSys/CGuiSliderGroup.hpp"
#include "GuiSys/CGuiModel.hpp"
#include "CGameState.hpp"
#include "CDependencyGroup.hpp"
#include "Audio/CAudioGroupSet.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "CNESEmulator.hpp"
#include "CQuitGameScreen.hpp"
#include "Input/RumbleFxTable.hpp"
#include <ctime>

namespace urde::MP1
{

/* Music volume constants */
static const float FE1_VOL = 0.7421875f;
static const float FE2_VOL = 0.7421875f;

/* L/R Stereo transition cues */
static const u16 FETransitionBackSFX[3][2] =
{
    {SFXfnt_transfore_00L, SFXfnt_transfore_00R},
    {SFXfnt_transfore_01L, SFXfnt_transfore_01R},
    {SFXfnt_transfore_02L, SFXfnt_transfore_02R}
};

static const u16 FETransitionForwardSFX[3][2] =
{
    {SFXfnt_transback_00L, SFXfnt_transback_00R},
    {SFXfnt_transback_01L, SFXfnt_transback_01R},
    {SFXfnt_transback_02L, SFXfnt_transback_02R}
};

struct FEMovie
{
    const char* path;
    bool loop;
};

static const FEMovie FEMovies[] =
{
    {"Video/00_first_start.thp", false},
    {"Video/01_startloop.thp", true},
    {"Video/02_start_fileselect_A.thp", false},
    {"Video/03_fileselectloop.thp", true},
    {"Video/04_fileselect_playgame_A.thp", false},
    {"Video/06_fileselect_GBA.thp", false},
    {"Video/07_GBAloop.thp", true},
    {"Video/08_GBA_fileselect.thp", false},
    {"Video/08_GBA_fileselect.thp", false},
};

SObjectTag g_DefaultWorldTag = {FOURCC('MLVL'), 0x158efe17};

void CFrontEndUI::PlayAdvanceSfx()
{
    CSfxManager::SfxStart(SFXfnt_advance_L, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
    CSfxManager::SfxStart(SFXfnt_advance_R, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
}

CFrontEndUI::SNewFileSelectFrame::SNewFileSelectFrame(CSaveGameScreen* sui, u32 rnd, CFrontEndUITouchBar& touchBar)
: x0_rnd(rnd), x4_saveUI(sui), m_touchBar(touchBar)
{
    x10_frme = g_SimplePool->GetObj("FRME_NewFileSelect");
}

void CFrontEndUI::SNewFileSelectFrame::FinishedLoading()
{
    x1c_loadedFrame->SetAspectConstraint(1.78f);

    x20_tablegroup_fileselect = static_cast<CGuiTableGroup*>(x1c_loadedFrame->FindWidget("tablegroup_fileselect"));
    x24_model_erase = static_cast<CGuiModel*>(x1c_loadedFrame->FindWidget("model_erase"));
    xf8_model_erase_position = x24_model_erase->GetLocalPosition();
    x28_textpane_erase = FindTextPanePair(x1c_loadedFrame, "textpane_erase");
    x38_textpane_gba = FindTextPanePair(x1c_loadedFrame, "textpane_gba");
    x30_textpane_cheats = FindTextPanePair(x1c_loadedFrame, "textpane_cheats");
    x48_textpane_popupadvance = FindTextPanePair(x1c_loadedFrame, "textpane_popupadvance");
    x50_textpane_popupcancel = FindTextPanePair(x1c_loadedFrame, "textpane_popupcancel");
    x58_textpane_popupextra = FindTextPanePair(x1c_loadedFrame, "textpane_popupextra");
    x40_tablegroup_popup = static_cast<CGuiTableGroup*>(x1c_loadedFrame->FindWidget("tablegroup_popup"));
    x44_model_dash7 = static_cast<CGuiModel*>(x1c_loadedFrame->FindWidget("model_dash7"));
    x60_textpane_cancel = static_cast<CGuiTextPane*>(x1c_loadedFrame->FindWidget("textpane_cancel"));
    FindAndSetPairText(x1c_loadedFrame, "textpane_title", g_MainStringTable->GetString(97));
    CGuiTextPane* proceed = static_cast<CGuiTextPane*>(x1c_loadedFrame->FindWidget("textpane_proceed"));
    if (proceed)
        proceed->TextSupport().SetText(g_MainStringTable->GetString(85));
    x40_tablegroup_popup->SetIsVisible(false);
    x40_tablegroup_popup->SetIsActive(false);
    x40_tablegroup_popup->SetVertical(false);
    CGuiWidget* worker = x40_tablegroup_popup->GetWorkerWidget(2);
    worker->SetIsSelectable(false);
    worker->SetVisibility(false, ETraversalMode::Children);

    x20_tablegroup_fileselect->SetMenuAdvanceCallback(
        std::bind(&SNewFileSelectFrame::DoFileMenuAdvance, this, std::placeholders::_1));
    x20_tablegroup_fileselect->SetMenuSelectionChangeCallback(
        std::bind(&SNewFileSelectFrame::DoSelectionChange, this, std::placeholders::_1, std::placeholders::_2));
    x20_tablegroup_fileselect->SetMenuCancelCallback(
        std::bind(&SNewFileSelectFrame::DoFileMenuCancel, this, std::placeholders::_1));

    x40_tablegroup_popup->SetMenuAdvanceCallback(
        std::bind(&SNewFileSelectFrame::DoPopupAdvance, this, std::placeholders::_1));
    x40_tablegroup_popup->SetMenuSelectionChangeCallback(
        std::bind(&SNewFileSelectFrame::DoSelectionChange, this, std::placeholders::_1, std::placeholders::_2));
    x40_tablegroup_popup->SetMenuCancelCallback(
        std::bind(&SNewFileSelectFrame::DoPopupCancel, this, std::placeholders::_1));

    for (int i=0 ; i<3 ; ++i)
        x64_fileSelections[i] = FindFileSelectOption(x1c_loadedFrame, i);

    x104_rowPitch = (x64_fileSelections[1].x0_base->GetLocalPosition() -
                     x64_fileSelections[0].x0_base->GetLocalPosition()).z();
}

bool CFrontEndUI::SNewFileSelectFrame::PumpLoad()
{
    if (x1c_loadedFrame)
        return true;
    if (x10_frme.IsLoaded())
    {
        if (x10_frme->GetIsFinishedLoading())
        {
            x1c_loadedFrame = x10_frme.GetObj();
            FinishedLoading();
            return true;
        }
    }
    return false;
}

bool CFrontEndUI::SNewFileSelectFrame::IsTextDoneAnimating() const
{
    if (x64_fileSelections[0].x28_curField != 4)
        return false;
    if (x64_fileSelections[1].x28_curField != 4)
        return false;
    if (x64_fileSelections[2].x28_curField != 4)
        return false;
    if (!x28_textpane_erase.x0_panes[0]->GetTextSupport().IsAnimationDone())
        return false;
    return x38_textpane_gba.x0_panes[0]->GetTextSupport().IsAnimationDone();
}

void CFrontEndUI::SNewFileSelectFrame::Update(float dt)
{
    bool saveReady = x4_saveUI->GetUIType() == CSaveGameScreen::EUIType::SaveReady;
    if (saveReady != x10c_saveReady)
    {
        if (saveReady)
        {
            ClearFrameContents();
        }
        else if (x8_subMenu != ESubMenu::Root)
        {
            ResetFrame();
            DeactivateEraseGamePopup();
            DeactivateNewGamePopup();
            x8_subMenu = ESubMenu::Root;
        }
        x10c_saveReady = saveReady;
    }
    if (x10c_saveReady)
        SetupFrameContents();

    x1c_loadedFrame->Update(dt);
}

CFrontEndUI::SNewFileSelectFrame::EAction
CFrontEndUI::SNewFileSelectFrame::ProcessUserInput(const CFinalInput& input, CFrontEndUITouchBar::EAction tbAction)
{
    xc_action = EAction::None;

    if (x8_subMenu != ESubMenu::EraseGamePopup)
        x4_saveUI->ProcessUserInput(input);

    if (IsTextDoneAnimating())
        x108_curTime = std::min(0.5f, x108_curTime + input.DeltaTime());

    if (x108_curTime < 0.5f)
        return xc_action;

    if (x10c_saveReady)
    {
        x1c_loadedFrame->ProcessUserInput(input);
        if (x8_subMenu == ESubMenu::Root || x8_subMenu == ESubMenu::EraseGame)
        {
            if (m_touchBar.GetPhase() != CFrontEndUITouchBar::EPhase::FileSelect)
                HandleActiveChange(x20_tablegroup_fileselect);

            if (tbAction >= CFrontEndUITouchBar::EAction::FileA &&
                tbAction <= CFrontEndUITouchBar::EAction::ImageGallery)
            {
                x20_tablegroup_fileselect->SetUserSelection(int(tbAction) - int(CFrontEndUITouchBar::EAction::FileA));
                HandleActiveChange(x20_tablegroup_fileselect);
                DoFileMenuAdvance(x20_tablegroup_fileselect);
            }
            else if (tbAction == CFrontEndUITouchBar::EAction::Back)
            {
                DoFileMenuCancel(x20_tablegroup_fileselect);
            }
        }
        else if (x8_subMenu == ESubMenu::EraseGamePopup)
        {
            if (m_touchBar.GetPhase() != CFrontEndUITouchBar::EPhase::EraseBack)
                HandleActiveChange(x40_tablegroup_popup);

            if (tbAction != CFrontEndUITouchBar::EAction::None)
            {
                if (tbAction == CFrontEndUITouchBar::EAction::Confirm)
                    x40_tablegroup_popup->SetUserSelection(1);
                else
                    x40_tablegroup_popup->SetUserSelection(0);
                HandleActiveChange(x40_tablegroup_popup);
                DoPopupAdvance(x40_tablegroup_popup);
            }
        }
        else if (x8_subMenu == ESubMenu::NewGamePopup)
        {
            if (m_touchBar.GetPhase() != CFrontEndUITouchBar::EPhase::StartOptions)
                HandleActiveChange(x40_tablegroup_popup);

            if (tbAction == CFrontEndUITouchBar::EAction::Back)
            {
                DoPopupCancel(x40_tablegroup_popup);
            }
            else if (tbAction != CFrontEndUITouchBar::EAction::None)
            {
                if (tbAction == CFrontEndUITouchBar::EAction::Options)
                    x40_tablegroup_popup->SetUserSelection(1);
                else if (tbAction == CFrontEndUITouchBar::EAction::Start ||
                         tbAction == CFrontEndUITouchBar::EAction::Hard)
                    x40_tablegroup_popup->SetUserSelection(0);
                else if (tbAction == CFrontEndUITouchBar::EAction::Normal)
                    x40_tablegroup_popup->SetUserSelection(2);
                HandleActiveChange(x40_tablegroup_popup);
                DoPopupAdvance(x40_tablegroup_popup);
            }
        }
    }

    if (x10d_needsEraseToggle)
    {
        if (x40_tablegroup_popup->GetIsActive())
            DeactivateEraseGamePopup();
        else
            ActivateEraseGamePopup();
        x10d_needsEraseToggle = false;
    }

    if (x10e_needsNewToggle)
    {
        if (x40_tablegroup_popup->GetIsActive())
            DeactivateNewGamePopup();
        else
            ActivateNewGamePopup();
        x10e_needsNewToggle = false;
    }

    return xc_action;
}

void CFrontEndUI::SNewFileSelectFrame::Draw() const
{
    if (x1c_loadedFrame && x10c_saveReady)
        x1c_loadedFrame->Draw(CGuiWidgetDrawParms::Default);
}

void CFrontEndUI::SNewFileSelectFrame::HandleActiveChange(CGuiTableGroup* active)
{
    if (!active)
        return;

    active->SetColors(zeus::CColor::skWhite,
                      zeus::CColor{0.627450f, 0.627450f, 0.627450f, 0.784313f});

    if (active == x20_tablegroup_fileselect)
    {
        x24_model_erase->SetLocalTransform(zeus::CTransform::Translate(
            zeus::CVector3f{0.f, 0.f, active->GetUserSelection() * x104_rowPitch} + xf8_model_erase_position));

        /* Set Touch Bar contents here */
        CFrontEndUITouchBar::SFileSelectDetail tbDetails[3] = {};
        for (int i=0 ; i<3 ; ++i)
        {
            if (const CGameState::GameFileStateInfo* data = x4_saveUI->GetGameData(i))
            {
                tbDetails[i].state = data->x20_hardMode ? CFrontEndUITouchBar::EFileState::Hard :
                                                          CFrontEndUITouchBar::EFileState::Normal;
                tbDetails[i].percent = data->x18_itemPercent;
            }
        }
        m_touchBar.SetFileSelectPhase(tbDetails, x8_subMenu == ESubMenu::EraseGame,
                                      CSlideShow::SlideShowGalleryFlags());
    }
    else if (active == x40_tablegroup_popup)
    {
        if (x8_subMenu == ESubMenu::EraseGamePopup)
            m_touchBar.SetPhase(CFrontEndUITouchBar::EPhase::EraseBack);
        else if (x8_subMenu == ESubMenu::NewGamePopup)
            m_touchBar.SetStartOptionsPhase(g_GameState->SystemOptions().GetPlayerBeatNormalMode());
        else
            m_touchBar.SetPhase(CFrontEndUITouchBar::EPhase::None);
    }

    if (x8_subMenu == ESubMenu::Root || x8_subMenu == ESubMenu::NewGamePopup)
        x24_model_erase->SetIsVisible(false);
    else
        x24_model_erase->SetIsVisible(true);
}

void CFrontEndUI::SNewFileSelectFrame::DeactivateEraseGamePopup()
{
    x40_tablegroup_popup->SetIsActive(false);
    x40_tablegroup_popup->SetIsVisible(false);
    x20_tablegroup_fileselect->SetIsActive(true);

    HandleActiveChange(x20_tablegroup_fileselect);

    x64_fileSelections[x20_tablegroup_fileselect->GetUserSelection()].
        x0_base->SetColor(zeus::CColor::skWhite);
}

void CFrontEndUI::SNewFileSelectFrame::ActivateEraseGamePopup()
{
    x40_tablegroup_popup->SetIsActive(true);
    x40_tablegroup_popup->SetIsVisible(true);
    x40_tablegroup_popup->SetLocalTransform(
        zeus::CTransform::Translate(0.f, 0.f, x20_tablegroup_fileselect->GetUserSelection() * x104_rowPitch) *
            x40_tablegroup_popup->GetTransform());
    x20_tablegroup_fileselect->SetIsActive(false);

    x8_subMenu = ESubMenu::EraseGamePopup;
    HandleActiveChange(x40_tablegroup_popup);

    x48_textpane_popupadvance.SetPairText(g_MainStringTable->GetString(95));
    x50_textpane_popupcancel.SetPairText(g_MainStringTable->GetString(38));

    x64_fileSelections[x20_tablegroup_fileselect->GetUserSelection()].
        x0_base->SetColor(zeus::CColor{1.f, 1.f, 1.f, 0.f});
    x44_model_dash7->SetVisibility(false, ETraversalMode::Children);
}

void CFrontEndUI::SNewFileSelectFrame::DeactivateNewGamePopup()
{
    x40_tablegroup_popup->SetIsActive(false);
    x40_tablegroup_popup->SetIsVisible(false);
    x20_tablegroup_fileselect->SetIsActive(true);

    CGuiWidget* worker = x40_tablegroup_popup->GetWorkerWidget(2);
    worker->SetIsSelectable(false);
    worker->SetVisibility(false, ETraversalMode::Children);

    x44_model_dash7->SetVisibility(false, ETraversalMode::Children);

    HandleActiveChange(x20_tablegroup_fileselect);

    x64_fileSelections[x20_tablegroup_fileselect->GetUserSelection()].
        x0_base->SetColor(zeus::CColor::skWhite);
    x60_textpane_cancel->TextSupport().SetText(u"");
}

void CFrontEndUI::SNewFileSelectFrame::ActivateNewGamePopup()
{
    x40_tablegroup_popup->SetIsActive(true);
    x40_tablegroup_popup->SetIsVisible(true);
    x40_tablegroup_popup->SetUserSelection(0);
    x40_tablegroup_popup->SetLocalTransform(
        zeus::CTransform::Translate(0.f, 0.f, x20_tablegroup_fileselect->GetUserSelection() * x104_rowPitch) *
            x40_tablegroup_popup->GetTransform());
    x20_tablegroup_fileselect->SetIsActive(false);

    x8_subMenu = ESubMenu::NewGamePopup;
    HandleActiveChange(x40_tablegroup_popup);
    x64_fileSelections[x20_tablegroup_fileselect->GetUserSelection()].
        x0_base->SetColor(zeus::CColor{1.f, 1.f, 1.f, 0.f});

    PlayAdvanceSfx();

    if (g_GameState->SystemOptions().GetPlayerBeatNormalMode())
    {
        x48_textpane_popupadvance.SetPairText(g_MainStringTable->GetString(102));
        x50_textpane_popupcancel.SetPairText(g_MainStringTable->GetString(94));
        x58_textpane_popupextra.SetPairText(g_MainStringTable->GetString(101));
        CGuiWidget* worker = x40_tablegroup_popup->GetWorkerWidget(2);
        worker->SetIsSelectable(true);
        worker->SetVisibility(true, ETraversalMode::Children);
        x44_model_dash7->SetVisibility(true, ETraversalMode::Children);
    }
    else
    {
        x48_textpane_popupadvance.SetPairText(g_MainStringTable->GetString(67));
        x50_textpane_popupcancel.SetPairText(g_MainStringTable->GetString(94));
        x44_model_dash7->SetVisibility(false, ETraversalMode::Children);
    }
    x60_textpane_cancel->TextSupport().SetText(g_MainStringTable->GetString(82));
}

void CFrontEndUI::SNewFileSelectFrame::ResetFrame()
{
    x8_subMenu = ESubMenu::Root;

    x38_textpane_gba.x0_panes[0]->SetIsSelectable(true);
    x38_textpane_gba.x0_panes[0]->TextSupport().SetFontColor(zeus::CColor::skWhite);

    x30_textpane_cheats.x0_panes[0]->SetIsSelectable(true);
    x30_textpane_cheats.x0_panes[0]->TextSupport().SetFontColor(zeus::CColor::skWhite);

    ClearFrameContents();

    for (int i=2 ; i>=0 ; --i)
        x20_tablegroup_fileselect->GetWorkerWidget(i)->SetIsSelectable(true);
    x60_textpane_cancel->TextSupport().SetText(u"");
}

void CFrontEndUI::SNewFileSelectFrame::ActivateErase()
{
    x8_subMenu = ESubMenu::EraseGame;
    x28_textpane_erase.x0_panes[0]->SetIsSelectable(false);
    zeus::CColor color = zeus::CColor::skGrey;
    color.a() = 0.5f;
    x28_textpane_erase.x0_panes[0]->TextSupport().SetFontColor(color);
    x38_textpane_gba.x0_panes[0]->TextSupport().SetFontColor(color);
    x30_textpane_cheats.x0_panes[0]->TextSupport().SetFontColor(color);
    x38_textpane_gba.x0_panes[0]->SetIsSelectable(false);
    x30_textpane_cheats.x0_panes[0]->SetIsSelectable(false);

    for (int i=2 ; i>=0 ; --i)
    {
        SFileMenuOption& fileOpt = x64_fileSelections[i];
        if (x4_saveUI->GetGameData(i))
        {
            fileOpt.x0_base->SetIsSelectable(true);
            x20_tablegroup_fileselect->SetUserSelection(i);
        }
        else
        {
            fileOpt.x0_base->SetIsSelectable(false);
        }
    }

    x60_textpane_cancel->TextSupport().SetText(g_MainStringTable->GetString(82));
    HandleActiveChange(x20_tablegroup_fileselect);
}

void CFrontEndUI::SNewFileSelectFrame::ClearFrameContents()
{
    x108_curTime = 0.f;
    bool hasSave = false;
    for (int i=0 ; i<3 ; ++i)
    {
        if (x4_saveUI->GetGameData(i))
            hasSave = true;
        SFileMenuOption& option = x64_fileSelections[i];
        option.x2c_chRate = SFileMenuOption::ComputeRandom();
        option.x28_curField = -1;
        for (int j=0 ; j<4 ; ++j)
            option.x4_textpanes[j].SetPairText(u"");
    }

    StartTextAnimating(x28_textpane_erase.x0_panes[0], g_MainStringTable->GetString(38), 60.f);
    StartTextAnimating(x38_textpane_gba.x0_panes[0], g_MainStringTable->GetString(37), 60.f);
    StartTextAnimating(x30_textpane_cheats.x0_panes[0], g_MainStringTable->GetString(96), 60.f);

    StartTextAnimating(x28_textpane_erase.x0_panes[1], g_MainStringTable->GetString(38), 60.f);
    StartTextAnimating(x38_textpane_gba.x0_panes[1], g_MainStringTable->GetString(37), 60.f);
    StartTextAnimating(x30_textpane_cheats.x0_panes[1], g_MainStringTable->GetString(96), 60.f);

    if (hasSave)
    {
        x28_textpane_erase.x0_panes[0]->SetIsSelectable(true);
        x28_textpane_erase.x0_panes[0]->TextSupport().SetFontColor(zeus::CColor::skWhite);
    }
    else
    {
        x28_textpane_erase.x0_panes[0]->SetIsSelectable(false);
        zeus::CColor color = zeus::CColor::skGrey;
        color.a() = 0.5f;
        x28_textpane_erase.x0_panes[0]->TextSupport().SetFontColor(color);
    }

    x20_tablegroup_fileselect->SetUserSelection(0);
    CGuiTextPane* cheats = static_cast<CGuiTextPane*>(x20_tablegroup_fileselect->GetWorkerWidget(5));
    if (CSlideShow::SlideShowGalleryFlags())
    {
        cheats->SetIsSelectable(true);
        x30_textpane_cheats.x0_panes[0]->TextSupport().SetFontColor(zeus::CColor::skWhite);
    }
    else
    {
        cheats->SetIsSelectable(false);
        zeus::CColor color = zeus::CColor::skGrey;
        color.a() = 0.5f;
        x30_textpane_cheats.x0_panes[0]->TextSupport().SetFontColor(color);
    }

    HandleActiveChange(x20_tablegroup_fileselect);
}

void CFrontEndUI::SNewFileSelectFrame::SetupFrameContents()
{
    for (int i=0 ; i<3 ; ++i)
    {
        SFileMenuOption& option = x64_fileSelections[i];
        if (option.x28_curField == 4)
            continue;
        SGuiTextPair* pair = (option.x28_curField == -1) ? nullptr : &option.x4_textpanes[option.x28_curField];
        if (!pair ||
            pair->x0_panes[0]->GetTextSupport().GetNumCharsPrinted() >=
            pair->x0_panes[0]->GetTextSupport().GetNumCharsTotal())
        {
            if (++option.x28_curField < 4)
            {
                std::u16string str;
                SGuiTextPair& populatePair = option.x4_textpanes[option.x28_curField];
                const CGameState::GameFileStateInfo* data = x4_saveUI->GetGameData(i);

                switch (option.x28_curField)
                {
                case 0:
                    // Completion percent
                    if (data)
                    {
                        std::u16string fileStr = g_MainStringTable->GetString(data->x20_hardMode ? 106 : 39);
                        str = fileStr + hecl::Char16Format(L"  %02d%%", data->x18_itemPercent);
                        break;
                    }
                    str = g_MainStringTable->GetString(36);
                    break;

                case 1:
                    // World name
                    if (data)
                    {
                        if (g_MemoryCardSys->HasSaveWorldMemory(data->x8_mlvlId))
                        {
                            const CSaveWorldMemory& wldMem = g_MemoryCardSys->GetSaveWorldMemory(data->x8_mlvlId);
                            str = wldMem.GetFrontEndName();
                        }
                        break;
                    }
                    str = g_MainStringTable->GetString(51);
                    break;

                case 2:
                    // Formatted time
                    if (data)
                    {
                        auto pt = std::div(data->x0_playTime, 3600);
                        str = hecl::Char16Format(L"%02d:%02d", pt.quot, pt.rem / 60);
                        break;
                    }
                    str = g_MainStringTable->GetString(52);
                    break;

                case 3:
                    // "Elapsed"
                    str = g_MainStringTable->GetString(data ? 54 : 53);
                    break;

                default: break;
                }

                StartTextAnimating(populatePair.x0_panes[0], str, option.x2c_chRate);
                StartTextAnimating(populatePair.x0_panes[1], str, option.x2c_chRate);
            }
        }
    }
}

void CFrontEndUI::SNewFileSelectFrame::DoPopupCancel(CGuiTableGroup* caller)
{
    if (x8_subMenu == ESubMenu::EraseGamePopup)
    {
        CSfxManager::SfxStart(SFXfnt_back, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        x8_subMenu = ESubMenu::EraseGame;
        x10d_needsEraseToggle = true;
    }
    else
    {
        CSfxManager::SfxStart(SFXfnt_back, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        x8_subMenu = ESubMenu::Root;
        x10e_needsNewToggle = true;
    }
}

void CFrontEndUI::SNewFileSelectFrame::DoPopupAdvance(CGuiTableGroup* caller)
{
    if (x8_subMenu == ESubMenu::EraseGamePopup)
    {
        if (x40_tablegroup_popup->GetUserSelection() == 1)
        {
            x4_saveUI->EraseGame(x20_tablegroup_fileselect->GetUserSelection());
            ResetFrame();
        }
        else
            x8_subMenu = ESubMenu::EraseGame;
        x10d_needsEraseToggle = true;
    }
    else
    {
        if (g_GameState->SystemOptions().GetPlayerBeatNormalMode())
        {
            if (x40_tablegroup_popup->GetUserSelection() == 1)
            {
                PlayAdvanceSfx();
                xc_action = EAction::GameOptions;
                return;
            }
            g_GameState->SetHardMode(!x40_tablegroup_popup->GetUserSelection());
            x4_saveUI->StartGame(x20_tablegroup_fileselect->GetUserSelection());
        }
        else
        {
            if (x40_tablegroup_popup->GetUserSelection() == 1)
            {
                PlayAdvanceSfx();
                xc_action = EAction::GameOptions;
                return;
            }
            x4_saveUI->StartGame(x20_tablegroup_fileselect->GetUserSelection());
        }
    }
}

void CFrontEndUI::SNewFileSelectFrame::DoFileMenuCancel(CGuiTableGroup* caller)
{
    if (x8_subMenu == ESubMenu::EraseGame)
    {
        CSfxManager::SfxStart(SFXfnt_back, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        ResetFrame();
    }
}

void CFrontEndUI::SNewFileSelectFrame::DoSelectionChange(CGuiTableGroup* caller, int oldSel)
{
    HandleActiveChange(caller);
    CSfxManager::SfxStart(SFXfnt_selection_change, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
}

void CFrontEndUI::SNewFileSelectFrame::DoFileMenuAdvance(CGuiTableGroup* caller)
{
    int userSel = x20_tablegroup_fileselect->GetUserSelection();
    if (userSel < 3)
    {
        if (x8_subMenu == ESubMenu::EraseGame)
        {
            if (x4_saveUI->GetGameData(userSel))
            {
                PlayAdvanceSfx();
                x10d_needsEraseToggle = true;
            }
        }
        else
        {
            if (x4_saveUI->GetGameData(userSel))
            {
                m_touchBar.SetPhase(CFrontEndUITouchBar::EPhase::None);
                x4_saveUI->StartGame(userSel);
            }
            else
                x10e_needsNewToggle = true;
        }
    }
    else if (userSel == 3)
    {
        PlayAdvanceSfx();
        ActivateErase();
    }
    else if (userSel == 4)
    {
        xc_action = EAction::FusionBonus;
    }
    else if (userSel == 5)
    {
        xc_action = EAction::SlideShow;
    }
}

CFrontEndUI::SFileMenuOption CFrontEndUI::SNewFileSelectFrame::FindFileSelectOption(CGuiFrame* frame, int idx)
{
    SFileMenuOption ret;
    ret.x0_base = frame->FindWidget(hecl::Format("basewidget_file%d", idx).c_str());
    ret.x4_textpanes[0] = FindTextPanePair(frame, hecl::Format("textpane_filename%d", idx).c_str());
    ret.x4_textpanes[1] = FindTextPanePair(frame, hecl::Format("textpane_world%d", idx).c_str());
    ret.x4_textpanes[2] = FindTextPanePair(frame, hecl::Format("textpane_playtime%d", idx).c_str());
    ret.x4_textpanes[3] = FindTextPanePair(frame, hecl::Format("textpane_date%d", idx).c_str());
    return ret;
}

void CFrontEndUI::SNewFileSelectFrame::StartTextAnimating(CGuiTextPane* text, std::u16string_view str, float chRate)
{
    text->TextSupport().SetText(str);
    text->TextSupport().SetTypeWriteEffectOptions(true, 0.1f, chRate);
}

CFrontEndUI::SFusionBonusFrame::SFusionBonusFrame(CFrontEndUITouchBar& touchBar)
: m_touchBar(touchBar)
{
    x4_gbaSupport = std::make_unique<CGBASupport>();
    xc_gbaScreen = g_SimplePool->GetObj("FRME_GBAScreen");
    x18_gbaLink = g_SimplePool->GetObj("FRME_GBALink");
}

void CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::SetUIText(EUIType tp)
{
    int instructions = -1;
    int yes = -1;
    int no = -1;

    bool cableVisible = false;
    bool circleGcVisible = false;
    bool circleGbaVisible = false;
    bool circleStartVisible = false;
    bool pakoutVisible = false;
    bool gbaScreenVisible = false;
    bool connectVisible = false;

    switch (tp)
    {
    case EUIType::InsertPak:
        instructions = 73; // Insert Game Pak
        no = 82;
        yes = 83;
        pakoutVisible = true;
        circleGbaVisible = true;
        break;
    case EUIType::ConnectSocket:
        instructions = 68; // Connect socket
        no = 82;
        yes = 83;
        cableVisible = true;
        circleGcVisible = true;
        circleGbaVisible = true;
        break;
    case EUIType::PressStartAndSelect:
        instructions = 74; // Hold start and select
        no = 82;
        yes = 83;
        cableVisible = true;
        circleStartVisible = true;
        gbaScreenVisible = true;
        break;
    case EUIType::BeginLink:
        instructions = 75; // Begin link?
        no = 82;
        yes = 83;
        cableVisible = true;
        gbaScreenVisible = true;
        break;
    case EUIType::TurnOffGBA:
        instructions = 76; // Turn off GBA
        no = 82;
        yes = 83;
        cableVisible = true;
        gbaScreenVisible = true;
        circleStartVisible = true;
        break;
    case EUIType::Linking:
        x4_gbaSupport->StartLink();
        instructions = 72; // Linking
        cableVisible = true;
        gbaScreenVisible = true;
        connectVisible = true;
        break;
    case EUIType::LinkFailed:
        instructions = 69; // Link failed
        no = 82;
        yes = 83;
        cableVisible = true;
        circleGcVisible = true;
        circleGbaVisible = true;
        circleStartVisible = true;
        gbaScreenVisible = true;
        break;
    case EUIType::LinkCompleteOrLinking:
        yes = 83;
        instructions = x40_linkInProgress + 71; // Complete or linking
        cableVisible = true;
        gbaScreenVisible = true;
        break;
    case EUIType::Complete:
    case EUIType::Cancelled:
    default:
        break;
    }

    std::u16string instructionsStr;
    if (instructions != -1)
        instructionsStr = g_MainStringTable->GetString(instructions);
    xc_textpane_instructions.SetPairText(instructionsStr);

    std::u16string yesStr;
    if (yes != -1)
        yesStr = g_MainStringTable->GetString(yes);
    x14_textpane_yes->TextSupport().SetText(yesStr);

    std::u16string noStr;
    if (no != -1)
        noStr = g_MainStringTable->GetString(no);
    x18_textpane_no->TextSupport().SetText(noStr);

    x1c_model_gc->SetVisibility(true, ETraversalMode::Children);
    x20_model_gba->SetVisibility(true, ETraversalMode::Children);
    x24_model_cable->SetVisibility(cableVisible, ETraversalMode::Children);
    x28_model_circlegcport->SetVisibility(circleGcVisible, ETraversalMode::Children);
    x2c_model_circlegbaport->SetVisibility(circleGbaVisible, ETraversalMode::Children);
    x30_model_circlestartselect->SetVisibility(circleStartVisible, ETraversalMode::Children);
    x34_model_pakout->SetVisibility(pakoutVisible, ETraversalMode::Children);
    x38_model_gbascreen->SetVisibility(gbaScreenVisible, ETraversalMode::Children);
    x3c_model_connect->SetVisibility(connectVisible, ETraversalMode::Children);

    x0_uiType = tp;
}

static const CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::EUIType NextLinkUI[] =
{
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::EUIType::ConnectSocket,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::EUIType::PressStartAndSelect,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::EUIType::BeginLink,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::EUIType::Linking,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::EUIType::Empty,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::EUIType::TurnOffGBA,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::EUIType::Complete,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::EUIType::InsertPak,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::EUIType::Empty,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::EUIType::Empty
};

static const CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::EUIType PrevLinkUI[] =
{
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::EUIType::Cancelled,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::EUIType::Cancelled,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::EUIType::Cancelled,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::EUIType::Cancelled,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::EUIType::Empty,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::EUIType::Cancelled,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::EUIType::Empty,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::EUIType::Cancelled,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::EUIType::Empty,
    CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::EUIType::Empty
};

CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::EAction
CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::ProcessUserInput(const CFinalInput &input, bool linkInProgress,
                                                                CFrontEndUITouchBar::EAction tbAction)
{
    if (linkInProgress != x40_linkInProgress)
    {
        x40_linkInProgress = linkInProgress;
        SetUIText(x0_uiType);
    }

    switch (x0_uiType)
    {
    case EUIType::InsertPak:
    case EUIType::ConnectSocket:
    case EUIType::PressStartAndSelect:
    case EUIType::BeginLink:
    case EUIType::LinkFailed:
    case EUIType::LinkCompleteOrLinking:
    case EUIType::TurnOffGBA:
        if (input.PA() || tbAction == CFrontEndUITouchBar::EAction::Confirm)
        {
            PlayAdvanceSfx();
            SetUIText(NextLinkUI[int(x0_uiType)]);
        }
        else if (input.PB() || tbAction == CFrontEndUITouchBar::EAction::Back)
        {
            EUIType prevUi = PrevLinkUI[int(x0_uiType)];
            if (prevUi == EUIType::Empty)
                break;
            CSfxManager::SfxStart(SFXfnt_back, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
            SetUIText(prevUi);
        }
        break;
    case EUIType::Linking:
        if (x4_gbaSupport->GetPhase() == CGBASupport::EPhase::Complete)
        {
            if (x4_gbaSupport->IsFusionLinked())
                g_GameState->SystemOptions().SetPlayerLinkedFusion(true);
            if (x4_gbaSupport->IsFusionBeat())
                g_GameState->SystemOptions().SetPlayerBeatFusion(true);
            if (x4_gbaSupport->IsFusionLinked())
            {
                PlayAdvanceSfx();
                SetUIText(EUIType::LinkCompleteOrLinking);
            }
            else
            {
                CSfxManager::SfxStart(SFXfnt_back, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
                SetUIText(EUIType::LinkFailed);
            }
        }
        else if (x4_gbaSupport->GetPhase() == CGBASupport::EPhase::Failed)
        {
            CSfxManager::SfxStart(SFXfnt_back, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
            SetUIText(EUIType::LinkFailed);
        }
        break;
    case EUIType::Complete:
        return EAction::Complete;
    case EUIType::Cancelled:
        return EAction::Cancelled;
    default: break;
    }

    return EAction::None;
}

void CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::Update(float dt)
{
    x4_gbaSupport->Update(dt);
    x8_frme->Update(dt);
}

void CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::FinishedLoading()
{
    x8_frme->SetAspectConstraint(1.78f);

    xc_textpane_instructions = FindTextPanePair(x8_frme, "textpane_instructions");
    x14_textpane_yes = static_cast<CGuiTextPane*>(x8_frme->FindWidget("textpane_yes"));
    x18_textpane_no = static_cast<CGuiTextPane*>(x8_frme->FindWidget("textpane_no"));
    x1c_model_gc = static_cast<CGuiModel*>(x8_frme->FindWidget("model_gc"));
    x20_model_gba = static_cast<CGuiModel*>(x8_frme->FindWidget("model_gba"));
    x24_model_cable = static_cast<CGuiModel*>(x8_frme->FindWidget("model_cable"));
    x28_model_circlegcport = static_cast<CGuiModel*>(x8_frme->FindWidget("model_circlegcport"));
    x2c_model_circlegbaport = static_cast<CGuiModel*>(x8_frme->FindWidget("model_circlegbaport"));
    x30_model_circlestartselect = static_cast<CGuiModel*>(x8_frme->FindWidget("model_circlestartselect"));
    x34_model_pakout = static_cast<CGuiModel*>(x8_frme->FindWidget("model_pakout"));
    x38_model_gbascreen = static_cast<CGuiModel*>(x8_frme->FindWidget("model_gbascreen"));
    x3c_model_connect = static_cast<CGuiModel*>(x8_frme->FindWidget("model_connect"));
    SetUIText(EUIType::InsertPak);
}

void CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::Draw()
{
    x8_frme->Draw(CGuiWidgetDrawParms::Default);
}

CFrontEndUI::SFusionBonusFrame::SGBALinkFrame::SGBALinkFrame(CGuiFrame* linkFrame,
                                                             CGBASupport* support,
                                                             bool linkInProgress)
: x4_gbaSupport(support), x8_frme(linkFrame), x40_linkInProgress(linkInProgress)
{
    support->InitializeSupport();
    FinishedLoading();
}

void CFrontEndUI::SFusionBonusFrame::FinishedLoading()
{
    x24_loadedFrame->SetAspectConstraint(1.78f);

    x28_tablegroup_options = static_cast<CGuiTableGroup*>(x24_loadedFrame->FindWidget("tablegroup_options"));
    x2c_tablegroup_fusionsuit = static_cast<CGuiTableGroup*>(x24_loadedFrame->FindWidget("tablegroup_fusionsuit"));
    x30_textpane_instructions = FindTextPanePair(x24_loadedFrame, "textpane_instructions");

    FindAndSetPairText(x24_loadedFrame, "textpane_nes", g_MainStringTable->GetString(66));
    FindAndSetPairText(x24_loadedFrame, "textpane_fusionsuit", g_MainStringTable->GetString(63));
    FindAndSetPairText(x24_loadedFrame, "textpane_fusionsuitno", g_MainStringTable->GetString(65));
    FindAndSetPairText(x24_loadedFrame, "textpane_fusionsuityes", g_MainStringTable->GetString(64));
    FindAndSetPairText(x24_loadedFrame, "textpane_title", g_MainStringTable->GetString(100));

    static_cast<CGuiTextPane*>(x24_loadedFrame->FindWidget("textpane_proceed"))->
        TextSupport().SetText(g_MainStringTable->GetString(85));
    static_cast<CGuiTextPane*>(x24_loadedFrame->FindWidget("textpane_cancel"))->
        TextSupport().SetText(g_MainStringTable->GetString(82));

    x2c_tablegroup_fusionsuit->SetIsActive(false);
    x2c_tablegroup_fusionsuit->SetIsVisible(false);
    x2c_tablegroup_fusionsuit->SetVertical(false);
    x2c_tablegroup_fusionsuit->SetUserSelection(g_GameState->SystemOptions().GetPlayerFusionSuitActive());

    SetTableColors(x28_tablegroup_options);
    SetTableColors(x2c_tablegroup_fusionsuit);

    x28_tablegroup_options->SetMenuAdvanceCallback(
        std::bind(&SFusionBonusFrame::DoAdvance, this, std::placeholders::_1));
    x28_tablegroup_options->SetMenuSelectionChangeCallback(
        std::bind(&SFusionBonusFrame::DoSelectionChange, this, std::placeholders::_1, std::placeholders::_2));
    x28_tablegroup_options->SetMenuCancelCallback(
        std::bind(&SFusionBonusFrame::DoCancel, this, std::placeholders::_1));
    x2c_tablegroup_fusionsuit->SetMenuSelectionChangeCallback(
        std::bind(&SFusionBonusFrame::DoSelectionChange, this, std::placeholders::_1, std::placeholders::_2));
}

bool CFrontEndUI::SFusionBonusFrame::PumpLoad()
{
    if (x24_loadedFrame)
        return true;
    if (!xc_gbaScreen.IsLoaded())
        return false;
    if (!x18_gbaLink.IsLoaded())
        return false;
    if (!x4_gbaSupport->IsReady())
        return false;
    if (!xc_gbaScreen->GetIsFinishedLoading())
        return false;
    x24_loadedFrame = xc_gbaScreen.GetObj();
    FinishedLoading();
    return true;
}

void CFrontEndUI::SFusionBonusFrame::SetTableColors(CGuiTableGroup* tbgp) const
{
    tbgp->SetColors(zeus::CColor::skWhite,
                    zeus::CColor{0.627450f, 0.627450f, 0.627450f, 0.784313f});
}

void CFrontEndUI::SFusionBonusFrame::Update(float dt, CSaveGameScreen* saveUI)
{
    bool doDraw = !saveUI || saveUI->GetUIType() == CSaveGameScreen::EUIType::SaveReady;

    if (doDraw != x38_lastDoDraw)
    {
        x38_lastDoDraw = doDraw;
        ResetCompletionFlags();
    }

    if (x0_gbaLinkFrame)
        x0_gbaLinkFrame->Update(dt);
    else if (x24_loadedFrame)
        x24_loadedFrame->Update(dt);

    bool showFusionSuit = (g_GameState->SystemOptions().GetPlayerLinkedFusion() &&
                           g_GameState->SystemOptions().GetPlayerBeatNormalMode()) || m_gbaOverride;
    bool showFusionSuitProceed = showFusionSuit && x28_tablegroup_options->GetUserSelection() == 1;
    x2c_tablegroup_fusionsuit->SetIsActive(showFusionSuitProceed);
    x2c_tablegroup_fusionsuit->SetIsVisible(showFusionSuitProceed);
    x24_loadedFrame->FindWidget("textpane_proceed")->SetIsVisible(showFusionSuitProceed);

    std::u16string instructionStr;
    x30_textpane_instructions.x0_panes[0]->TextSupport().SetFontColor(zeus::CColor::skWhite);
    if (x28_tablegroup_options->GetUserSelection() == 1)
    {
        /* Fusion Suit */
        if (x3a_mpNotComplete)
            instructionStr = g_MainStringTable->GetString(80); // MP not complete
        else if (!showFusionSuit)
            instructionStr = g_MainStringTable->GetString(78); // To enable fusion suit
    }
    else
    {
        /* NES Metroid */
        if (x39_fusionNotComplete)
            instructionStr = g_MainStringTable->GetString(79); // You have not completed fusion
        else if (!g_GameState->SystemOptions().GetPlayerBeatFusion())
            instructionStr = g_MainStringTable->GetString(77); // To play NES Metroid
    }

    x30_textpane_instructions.SetPairText(instructionStr);
}

CFrontEndUI::SFusionBonusFrame::EAction
CFrontEndUI::SFusionBonusFrame::ProcessUserInput(const CFinalInput& input, CSaveGameScreen* sui,
                                                 CFrontEndUITouchBar::EAction tbAction)
{
    x8_action = EAction::None;

    if (sui)
        sui->ProcessUserInput(input);

    if (x38_lastDoDraw)
    {
        if (x0_gbaLinkFrame)
        {
            if (m_touchBar.GetPhase() != CFrontEndUITouchBar::EPhase::ProceedBack)
                m_touchBar.SetPhase(CFrontEndUITouchBar::EPhase::ProceedBack);

            SGBALinkFrame::EAction action = x0_gbaLinkFrame->ProcessUserInput(input, sui, tbAction);
            if (action != SGBALinkFrame::EAction::None)
            {
                x0_gbaLinkFrame.reset();
                if (action == SGBALinkFrame::EAction::Complete)
                {
                    if (x28_tablegroup_options->GetUserSelection() == 0 &&
                        !g_GameState->SystemOptions().GetPlayerBeatFusion())
                        x39_fusionNotComplete = true;
                    else if (sui)
                        sui->SaveNESState();
                }
            }
        }
        else if (x24_loadedFrame)
        {
            CFinalInput useInput = input;
            if (input.PZ())
            {
                useInput.x2d_b28_PA = true;
                m_gbaOverride = true;
            }

            bool showFusionSuit = (g_GameState->SystemOptions().GetPlayerLinkedFusion() &&
                                   g_GameState->SystemOptions().GetPlayerBeatNormalMode()) || m_gbaOverride;
            if (m_touchBar.GetPhase() != CFrontEndUITouchBar::EPhase::FusionBonus)
            {
                m_touchBar.SetFusionBonusPhase(showFusionSuit &&
                                               g_GameState->SystemOptions().GetPlayerFusionSuitActive());
            }

            x24_loadedFrame->ProcessUserInput(useInput);

            switch (tbAction)
            {
            case CFrontEndUITouchBar::EAction::NESMetroid:
                x28_tablegroup_options->SetUserSelection(0);
                ResetCompletionFlags();
                SetTableColors(x28_tablegroup_options);
                DoAdvance(x28_tablegroup_options);
                break;
            case CFrontEndUITouchBar::EAction::FusionSuit:
                x28_tablegroup_options->SetUserSelection(1);
                ResetCompletionFlags();
                SetTableColors(x28_tablegroup_options);
                if (showFusionSuit)
                {
                    if (x2c_tablegroup_fusionsuit->GetUserSelection() == 1)
                    {
                        x2c_tablegroup_fusionsuit->SetUserSelection(0);
                        DoSelectionChange(x2c_tablegroup_fusionsuit, 0);
                    }
                    else
                    {
                        x2c_tablegroup_fusionsuit->SetUserSelection(1);
                        DoSelectionChange(x2c_tablegroup_fusionsuit, 1);
                    }
                }
                else
                {
                    DoAdvance(x28_tablegroup_options);
                }
                break;
            case CFrontEndUITouchBar::EAction::Back:
                DoCancel(x28_tablegroup_options);
                break;
            default: break;
            }
        }
    }

    return x8_action;
}

void CFrontEndUI::SFusionBonusFrame::Draw() const
{
    if (!x38_lastDoDraw)
        return;
    if (x0_gbaLinkFrame)
        x0_gbaLinkFrame->Draw();
    else if (x24_loadedFrame)
        x24_loadedFrame->Draw(CGuiWidgetDrawParms::Default);
}

void CFrontEndUI::SFusionBonusFrame::DoCancel(CGuiTableGroup* caller)
{
    x8_action = EAction::GoBack;
    x28_tablegroup_options->SetUserSelection(0);
    x2c_tablegroup_fusionsuit->SetIsActive(false);
    x30_textpane_instructions.SetPairText(u"");
    SetTableColors(x28_tablegroup_options);
}

void CFrontEndUI::SFusionBonusFrame::DoSelectionChange(CGuiTableGroup* caller, int oldSel)
{
    if (caller == x28_tablegroup_options)
    {
        CSfxManager::SfxStart(SFXfnt_selection_change, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        ResetCompletionFlags();
    }
    else
    {
        CSfxManager::SfxStart(SFXfnt_enum_change, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        bool fusionActive = x2c_tablegroup_fusionsuit->GetUserSelection() == 1;
        g_GameState->SystemOptions().SetPlayerFusionSuitActive(fusionActive);
        g_GameState->GetPlayerState()->SetIsFusionEnabled(fusionActive);
        m_touchBar.SetFusionBonusPhase(g_GameState->SystemOptions().GetPlayerFusionSuitActive());
    }
    SetTableColors(caller);
}

void CFrontEndUI::SFusionBonusFrame::DoAdvance(CGuiTableGroup* caller)
{
    switch (x28_tablegroup_options->GetUserSelection())
    {
    case 1:
        /* Fusion Suit */
        if (x3a_mpNotComplete || m_gbaOverride)
        {
            x3a_mpNotComplete = false;
            PlayAdvanceSfx();
        }
        else if (g_GameState->SystemOptions().GetPlayerBeatNormalMode())
        {
            if (g_GameState->SystemOptions().GetPlayerLinkedFusion())
                return;
            x0_gbaLinkFrame = std::make_unique<SGBALinkFrame>(x18_gbaLink.GetObj(), x4_gbaSupport.get(), false);
            PlayAdvanceSfx();
        }
        else
        {
            x3a_mpNotComplete = true;
            CSfxManager::SfxStart(SFXfnt_back, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        }
        break;
    case 0:
        /* NES Metroid */
        if (x39_fusionNotComplete && !m_gbaOverride)
        {
            x39_fusionNotComplete = false;
            PlayAdvanceSfx();
        }
        else if (g_GameState->SystemOptions().GetPlayerBeatFusion() || m_gbaOverride)
        {
            //x8_action = EAction::None;
            CSfxManager::SfxStart(SFXfnt_back, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
            x8_action = EAction::PlayNESMetroid;
        }
        else
        {
            x0_gbaLinkFrame = std::make_unique<SGBALinkFrame>(x18_gbaLink.GetObj(), x4_gbaSupport.get(), false);
            PlayAdvanceSfx();
        }
        break;
    default: break;
    }
}

void CFrontEndUI::SGuiTextPair::SetPairText(std::u16string_view str)
{
    x0_panes[0]->TextSupport().SetText(str);
    x0_panes[1]->TextSupport().SetText(str);
}

CFrontEndUI::SGuiTextPair CFrontEndUI::FindTextPanePair(CGuiFrame* frame, const char* name)
{
    SGuiTextPair ret;
    ret.x0_panes[0] = static_cast<CGuiTextPane*>(frame->FindWidget(name));
    ret.x0_panes[1] = static_cast<CGuiTextPane*>(frame->FindWidget(hecl::Format("%sb", name).c_str()));
    return ret;
}

void CFrontEndUI::FindAndSetPairText(CGuiFrame* frame, const char* name, std::u16string_view str)
{
    CGuiTextPane* w1 = static_cast<CGuiTextPane*>(frame->FindWidget(name));
    w1->TextSupport().SetText(str);
    CGuiTextPane* w2 = static_cast<CGuiTextPane*>(frame->FindWidget(hecl::Format("%sb", name).c_str()));
    w2->TextSupport().SetText(str);
}

void CFrontEndUI::SFrontEndFrame::FinishedLoading()
{
    x14_loadedFrme->SetAspectConstraint(1.78f);

    x18_tablegroup_mainmenu = static_cast<CGuiTableGroup*>(x14_loadedFrme->FindWidget("tablegroup_mainmenu"));
    x1c_gbaPair = FindTextPanePair(x14_loadedFrme, "textpane_gba");
    x1c_gbaPair.SetPairText(g_MainStringTable->GetString(37));
    x24_cheatPair = FindTextPanePair(x14_loadedFrme, "textpane_cheats");
    x24_cheatPair.SetPairText(g_MainStringTable->GetString(96));

    FindAndSetPairText(x14_loadedFrme, "textpane_start", g_MainStringTable->GetString(67));
    FindAndSetPairText(x14_loadedFrme, "textpane_options", g_MainStringTable->GetString(94));
    FindAndSetPairText(x14_loadedFrme, "textpane_title", g_MainStringTable->GetString(98));

    CGuiTextPane* proceed = static_cast<CGuiTextPane*>(x14_loadedFrme->FindWidget("textpane_proceed"));
    if (proceed)
        proceed->TextSupport().SetText(g_MainStringTable->GetString(85));

    x18_tablegroup_mainmenu->SetMenuAdvanceCallback(
        std::bind(&SFrontEndFrame::DoAdvance, this, std::placeholders::_1));
    x18_tablegroup_mainmenu->SetMenuSelectionChangeCallback(
        std::bind(&SFrontEndFrame::DoSelectionChange, this, std::placeholders::_1, std::placeholders::_2));
    x18_tablegroup_mainmenu->SetMenuCancelCallback(
        std::bind(&SFrontEndFrame::DoCancel, this, std::placeholders::_1));

    HandleActiveChange(x18_tablegroup_mainmenu);
}

bool CFrontEndUI::SFrontEndFrame::PumpLoad()
{
    if (x14_loadedFrme)
        return true;
    if (x8_frme.IsLoaded())
    {
        if (CGuiFrame* frme = x8_frme.GetObj())
        {
            if (frme->GetIsFinishedLoading())
            {
                x14_loadedFrme = frme;
                FinishedLoading();
                return true;
            }
        }
    }
    return false;
}

void CFrontEndUI::SFrontEndFrame::Update(float dt)
{
    CGuiTextPane* imageGallery =
        static_cast<CGuiTextPane*>(x18_tablegroup_mainmenu->GetWorkerWidget(3));

    if (CSlideShow::SlideShowGalleryFlags())
    {
        imageGallery->SetIsSelectable(true);
        x24_cheatPair.x0_panes[0]->TextSupport().SetFontColor(zeus::CColor::skWhite);
    }
    else
    {
        imageGallery->SetIsSelectable(false);
        zeus::CColor color = zeus::CColor::skGrey;
        color.a() = 0.5f;
        x24_cheatPair.x0_panes[0]->TextSupport().SetFontColor(color);
    }

    x14_loadedFrme->Update(dt);
}

CFrontEndUI::SFrontEndFrame::EAction
CFrontEndUI::SFrontEndFrame::ProcessUserInput(const CFinalInput& input,
                                              CFrontEndUITouchBar::EAction tbAction)
{
    if (m_touchBar.GetPhase() != CFrontEndUITouchBar::EPhase::NoCardSelect)
        m_touchBar.SetNoCardSelectPhase(CSlideShow::SlideShowGalleryFlags());

    x4_action = EAction::None;
    x14_loadedFrme->ProcessUserInput(input);

    switch (tbAction)
    {
    case CFrontEndUITouchBar::EAction::Start:
        x18_tablegroup_mainmenu->SetUserSelection(0);
        HandleActiveChange(x18_tablegroup_mainmenu);
        DoAdvance(x18_tablegroup_mainmenu);
        break;
    case CFrontEndUITouchBar::EAction::FusionBonus:
        x18_tablegroup_mainmenu->SetUserSelection(1);
        HandleActiveChange(x18_tablegroup_mainmenu);
        DoAdvance(x18_tablegroup_mainmenu);
        break;
    case CFrontEndUITouchBar::EAction::Options:
        x18_tablegroup_mainmenu->SetUserSelection(2);
        HandleActiveChange(x18_tablegroup_mainmenu);
        DoAdvance(x18_tablegroup_mainmenu);
        break;
    case CFrontEndUITouchBar::EAction::ImageGallery:
        x18_tablegroup_mainmenu->SetUserSelection(3);
        HandleActiveChange(x18_tablegroup_mainmenu);
        DoAdvance(x18_tablegroup_mainmenu);
        break;
    default: break;
    }

    return x4_action;
}

void CFrontEndUI::SFrontEndFrame::Draw() const
{
    x14_loadedFrme->Draw(CGuiWidgetDrawParms::Default);
}

void CFrontEndUI::SFrontEndFrame::HandleActiveChange(CGuiTableGroup* active)
{
    active->SetColors(zeus::CColor::skWhite,
                      zeus::CColor{0.627450f, 0.627450f, 0.627450f, 0.784313f});
}

void CFrontEndUI::SFrontEndFrame::DoCancel(CGuiTableGroup* caller)
{
    /* Intentionally empty */
}

void CFrontEndUI::SFrontEndFrame::DoSelectionChange(CGuiTableGroup* caller, int oldSel)
{
    CSfxManager::SfxStart(SFXfnt_selection_change, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
    HandleActiveChange(caller);
}

void CFrontEndUI::SFrontEndFrame::DoAdvance(CGuiTableGroup* caller)
{
    switch (x18_tablegroup_mainmenu->GetUserSelection())
    {
    case 0:
        CSfxManager::SfxStart(FETransitionForwardSFX[x0_rnd][0], 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        CSfxManager::SfxStart(FETransitionForwardSFX[x0_rnd][1], 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        x4_action = EAction::StartGame;
        break;
    case 1:
        x4_action = EAction::FusionBonus;
        break;
    case 2:
        PlayAdvanceSfx();
        x4_action = EAction::GameOptions;
        break;
    case 3:
        PlayAdvanceSfx();
        x4_action = EAction::SlideShow;
        break;
    default: break;
    }
}

CFrontEndUI::SFrontEndFrame::SFrontEndFrame(u32 rnd, CFrontEndUITouchBar& touchBar)
: x0_rnd(rnd), m_touchBar(touchBar)
{
    x8_frme = g_SimplePool->GetObj("FRME_FrontEndPL");
}

CFrontEndUI::SNesEmulatorFrame::SNesEmulatorFrame()
{
    x4_nesEmu = std::make_unique<CNESEmulator>();

    const SObjectTag* deface = g_ResFactory->GetResourceIdByName("FONT_Deface14B");
    CGuiTextProperties props(false, true, EJustification::Left,
                             EVerticalJustification::Center);
    xc_textSupport = std::make_unique<CGuiTextSupport>(deface->id, props, zeus::CColor::skWhite,
                                                       zeus::CColor::skBlack, zeus::CColor::skWhite,
                                                       0, 0, g_SimplePool, CGuiWidget::EGuiModelDrawFlags::Alpha);
    xc_textSupport->SetText(g_MainStringTable->GetString(103));
    xc_textSupport->AutoSetExtent();
    xc_textSupport->ClearRenderBuffer();
}

void CFrontEndUI::SNesEmulatorFrame::SetMode(EMode mode)
{
    switch (mode)
    {
    case EMode::Emulator:
        x8_quitScreen.reset();
        break;
    case EMode::SaveProgress:
        x8_quitScreen = std::make_unique<CQuitGameScreen>(EQuitType::SaveProgress);
        break;
    case EMode::ContinuePlaying:
        x8_quitScreen = std::make_unique<CQuitGameScreen>(EQuitType::ContinuePlaying);
        break;
    case EMode::QuitNESMetroid:
        x8_quitScreen = std::make_unique<CQuitGameScreen>(EQuitType::QuitNESMetroid);
        break;
    default: break;
    }
    x0_mode = mode;
}

void CFrontEndUI::SNesEmulatorFrame::ProcessUserInput(const CFinalInput& input, CSaveGameScreen* sui)
{
    bool processInput = true;
    if (sui && sui->GetUIType() != CSaveGameScreen::EUIType::SaveReady)
        processInput = false;
    if (sui)
        sui->ProcessUserInput(input);
    if (!processInput)
        return;

    switch (x0_mode)
    {
    case EMode::Emulator:
        x4_nesEmu->ProcessUserInput(input, 4);
        if (input.ControllerIdx() == 0 && input.PL())
            SetMode(EMode::QuitNESMetroid);
        break;
    case EMode::SaveProgress:
    case EMode::ContinuePlaying:
    case EMode::QuitNESMetroid:
        x8_quitScreen->ProcessUserInput(input);
        break;
    default: break;
    }
}

bool CFrontEndUI::SNesEmulatorFrame::Update(float dt, CSaveGameScreen* saveUi)
{
    bool doUpdate = (saveUi && saveUi->GetUIType() != CSaveGameScreen::EUIType::SaveReady) ? false : true;
    x10_remTime = std::max(x10_remTime - dt, 0.f);

    zeus::CColor geomCol(zeus::CColor::skWhite);
    geomCol.a() = std::min(x10_remTime, 1.f);
    xc_textSupport->SetGeometryColor(geomCol);
    if (xc_textSupport->GetIsTextSupportFinishedLoading())
    {
        xc_textSupport->AutoSetExtent();
        xc_textSupport->ClearRenderBuffer();
    }

    if (!doUpdate)
        return false;

    switch (x0_mode)
    {
    case EMode::Emulator:
    {
        x4_nesEmu->Update();
        if (!x4_nesEmu->IsGameOver())
            x14_emulationSuspended = false;
        if (x4_nesEmu->IsGameOver() && !x14_emulationSuspended)
        {
            x14_emulationSuspended = true;
            if (saveUi && !saveUi->IsSavingDisabled())
            {
                SetMode(EMode::SaveProgress);
                break;
            }
            SetMode(EMode::ContinuePlaying);
            break;
        }
        if (x4_nesEmu->GetPasswordEntryState() == CNESEmulator::EPasswordEntryState::NotEntered && saveUi)
            x4_nesEmu->LoadPassword(g_GameState->SystemOptions().GetNESState());
        break;
    }

    case EMode::SaveProgress:
    {
        if (saveUi)
        {
            EQuitAction action = x8_quitScreen->Update(dt);
            if (action == EQuitAction::Yes)
            {
                memmove(g_GameState->SystemOptions().GetNESState(), x4_nesEmu->GetPassword(), 18);
                saveUi->SaveNESState();
                SetMode(EMode::ContinuePlaying);
            }
            else if (action == EQuitAction::No)
                SetMode(EMode::ContinuePlaying);
        }
        else
            SetMode(EMode::ContinuePlaying);
        break;
    }

    case EMode::ContinuePlaying:
    {
        EQuitAction action = x8_quitScreen->Update(dt);
        if (action == EQuitAction::Yes)
            SetMode(EMode::Emulator);
        else if (action == EQuitAction::No)
            return true;
        break;
    }

    case EMode::QuitNESMetroid:
    {
        EQuitAction action = x8_quitScreen->Update(dt);
        if (action == EQuitAction::Yes)
            return true;
        else if (action == EQuitAction::No)
            SetMode(EMode::Emulator);
        break;
    }

    default: break;
    }

    return false;
}

void CFrontEndUI::SNesEmulatorFrame::Draw(CSaveGameScreen* saveUi) const
{
    zeus::CColor mulColor = zeus::CColor::skWhite;
    bool blackout = saveUi && saveUi->GetUIType() != CSaveGameScreen::EUIType::SaveReady;

    if (blackout)
        mulColor = zeus::CColor::skBlack;
    else if (x8_quitScreen)
        mulColor = zeus::CColor{0.376470f, 0.376470f, 0.376470f, 1.f};

    x4_nesEmu->Draw(mulColor, x15_enableFiltering);
    if (!blackout && x8_quitScreen)
        x8_quitScreen->Draw();

    if (x10_remTime >= 7.5f)
        return;
    if (x10_remTime <= 0.f)
        return;
    if (xc_textSupport->GetIsTextSupportFinishedLoading())
    {
        float aspect = g_Viewport.x8_width / float(g_Viewport.xc_height) / 1.33f;
        CGraphics::SetOrtho(-320.f * aspect, 320.f * aspect, 240.f, -240.f, -4096.f, 4096.f);
        CGraphics::SetViewPointMatrix(zeus::CTransform::Identity());
        CGraphics::SetModelMatrix(zeus::CTransform::Translate(-220.f, 0.f, -200.f));
        xc_textSupport->Render();
    }
}

CFrontEndUI::SOptionsFrontEndFrame::SOptionsFrontEndFrame()
{
    x4_frme = g_SimplePool->GetObj("FRME_OptionsFrontEnd");
    x10_pauseScreen = g_SimplePool->GetObj("STRG_PauseScreen");
    x134_24_visible = true;
}

void CFrontEndUI::SOptionsFrontEndFrame::DoSliderChange(CGuiSliderGroup* caller, float value)
{
    if (x28_tablegroup_rightmenu->GetIsActive())
    {
        int leftSel = x24_tablegroup_leftmenu->GetUserSelection();
        int rightSel = x28_tablegroup_rightmenu->GetUserSelection();
        const auto& optionCategory = GameOptionsRegistry[leftSel];
        const SGameOption& option = optionCategory.second[rightSel];
        CGameOptions::SetOption(option.option, caller->GetGurVal());
        m_touchBarValueDirty = true;
    }
}

void CFrontEndUI::SOptionsFrontEndFrame::DoMenuCancel(CGuiTableGroup* caller)
{
    if (x28_tablegroup_rightmenu == caller)
    {
        DeactivateRightMenu();
        x24_tablegroup_leftmenu->SetIsActive(true);
        x28_tablegroup_rightmenu->SetIsActive(false);
        x28_tablegroup_rightmenu->SetUserSelection(0);
        SetTableColors(x28_tablegroup_rightmenu);
        CSfxManager::SfxStart(SFXfnt_back, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
    }
}

void CFrontEndUI::SOptionsFrontEndFrame::DoMenuSelectionChange(CGuiTableGroup* caller, int oldSel)
{
    SetTableColors(caller);
    if (x24_tablegroup_leftmenu == caller)
    {
        SetRightUIText();
        CSfxManager::SfxStart(SFXfnt_selection_change, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
    }
    else if (x28_tablegroup_rightmenu == caller)
    {
        HandleRightSelectionChange();
        CSfxManager::SfxStart(SFXfnt_selection_change, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
    }
    else if (x2c_tablegroup_double == caller || x30_tablegroup_triple == caller)
    {
        if (x28_tablegroup_rightmenu->GetIsActive())
        {
            int leftSel = x24_tablegroup_leftmenu->GetUserSelection();
            int rightSel = x28_tablegroup_rightmenu->GetUserSelection();
            const auto& optionCategory = GameOptionsRegistry[leftSel];
            const SGameOption& option = optionCategory.second[rightSel];
            CGameOptions::SetOption(option.option, caller->GetUserSelection());
            m_touchBarValueDirty = true;
            CSfxManager::SfxStart(SFXfnt_enum_change, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);

            if (option.option == EGameOption::Rumble && caller->GetUserSelection() > 0)
            {
                x40_rumbleGen.HardStopAll();
                x40_rumbleGen.Rumble(RumbleFxTable[int(ERumbleFxId::PlayerBump)], 1.f,
                                     ERumblePriority::One, EIOPort::Zero);
            }
        }
    }
}

void CFrontEndUI::SOptionsFrontEndFrame::DoLeftMenuAdvance(CGuiTableGroup* caller)
{
    if (caller == x24_tablegroup_leftmenu)
    {
        HandleRightSelectionChange();
        x28_tablegroup_rightmenu->SetUserSelection(0);
        x24_tablegroup_leftmenu->SetIsActive(false);
        x28_tablegroup_rightmenu->SetIsActive(true);
        CSfxManager::SfxStart(SFXfnt_advance_L, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        CSfxManager::SfxStart(SFXfnt_advance_R, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
    }
}

void CFrontEndUI::SOptionsFrontEndFrame::DeactivateRightMenu()
{
    x2c_tablegroup_double->SetIsActive(false);
    x30_tablegroup_triple->SetIsActive(false);
    x34_slidergroup_slider->SetIsActive(false);
    x2c_tablegroup_double->SetVisibility(false, ETraversalMode::Children);
    x30_tablegroup_triple->SetVisibility(false, ETraversalMode::Children);
    x34_slidergroup_slider->SetVisibility(false, ETraversalMode::Children);
}

void CFrontEndUI::SOptionsFrontEndFrame::HandleRightSelectionChange()
{
    DeactivateRightMenu();
    int leftSel = x24_tablegroup_leftmenu->GetUserSelection();
    int rightSel = x28_tablegroup_rightmenu->GetUserSelection();
    const auto& optionCategory = GameOptionsRegistry[leftSel];
    const SGameOption& option = optionCategory.second[rightSel];

    switch (option.type)
    {
    case EOptionType::Float:
        x34_slidergroup_slider->SetIsActive(true);
        x34_slidergroup_slider->SetVisibility(true, ETraversalMode::Children);
        x34_slidergroup_slider->SetMinVal(option.minVal);
        x34_slidergroup_slider->SetMaxVal(option.maxVal);
        x34_slidergroup_slider->SetIncrement(option.increment);
        x34_slidergroup_slider->SetCurVal(CGameOptions::GetOption(option.option));
        x34_slidergroup_slider->SetLocalTransform(
            zeus::CTransform::Translate(0.f, 0.f, rightSel * x38_rowPitch) *
            x34_slidergroup_slider->GetTransform());
        break;

    case EOptionType::DoubleEnum:
        x2c_tablegroup_double->SetUserSelection(CGameOptions::GetOption(option.option));
        x2c_tablegroup_double->SetIsVisible(true);
        x2c_tablegroup_double->SetIsActive(true);
        x2c_tablegroup_double->SetLocalTransform(
            zeus::CTransform::Translate(0.f, 0.f, rightSel * x38_rowPitch) *
            x2c_tablegroup_double->GetTransform());
        SetTableColors(x2c_tablegroup_double);
        break;

    case EOptionType::TripleEnum:
        x30_tablegroup_triple->SetUserSelection(CGameOptions::GetOption(option.option));
        x30_tablegroup_triple->SetIsVisible(true);
        x30_tablegroup_triple->SetIsActive(true);
        x30_tablegroup_triple->SetLocalTransform(
            zeus::CTransform::Translate(0.f, 0.f, rightSel * x38_rowPitch) *
            x30_tablegroup_triple->GetTransform());
        SetTableColors(x30_tablegroup_triple);
        break;

    default: break;
    }
}

void CFrontEndUI::SOptionsFrontEndFrame::SetRightUIText()
{
    int userSel = x24_tablegroup_leftmenu->GetUserSelection();
    const std::pair<int, const SGameOption*>& options = GameOptionsRegistry[userSel];

    for (int i=0 ; i<5 ; ++i)
    {
        char name[36];
        snprintf(name, 36, "textpane_right%d", i);
        if (i < options.first)
        {
            FindTextPanePair(x1c_loadedFrame, name).SetPairText(
                x20_loadedPauseStrg->GetString(options.second[i].stringId));
            x28_tablegroup_rightmenu->GetWorkerWidget(i)->SetIsSelectable(true);
        }
        else
        {
            FindTextPanePair(x1c_loadedFrame, name).SetPairText(u"");
            x28_tablegroup_rightmenu->GetWorkerWidget(i)->SetIsSelectable(false);
        }
    }
}

void CFrontEndUI::SOptionsFrontEndFrame::SetTableColors(CGuiTableGroup* tbgp) const
{
    tbgp->SetColors(zeus::CColor::skWhite,
                    zeus::CColor{0.627450f, 0.627450f, 0.627450f, 0.784313f});
}

void CFrontEndUI::SOptionsFrontEndFrame::FinishedLoading()
{
    x1c_loadedFrame->SetAspectConstraint(1.78f);

    x24_tablegroup_leftmenu = static_cast<CGuiTableGroup*>(x1c_loadedFrame->FindWidget("tablegroup_leftmenu"));
    x28_tablegroup_rightmenu = static_cast<CGuiTableGroup*>(x1c_loadedFrame->FindWidget("tablegroup_rightmenu"));
    x2c_tablegroup_double = static_cast<CGuiTableGroup*>(x1c_loadedFrame->FindWidget("tablegroup_double"));
    x30_tablegroup_triple = static_cast<CGuiTableGroup*>(x1c_loadedFrame->FindWidget("tablegroup_triple"));
    x34_slidergroup_slider = static_cast<CGuiSliderGroup*>(x1c_loadedFrame->FindWidget("slidergroup_slider"));

    x24_tablegroup_leftmenu->SetMenuAdvanceCallback(
        std::bind(&SOptionsFrontEndFrame::DoLeftMenuAdvance, this, std::placeholders::_1));
    x24_tablegroup_leftmenu->SetMenuSelectionChangeCallback(
        std::bind(&SOptionsFrontEndFrame::DoMenuSelectionChange, this, std::placeholders::_1, std::placeholders::_2));

    x38_rowPitch = x24_tablegroup_leftmenu->GetWorkerWidget(1)->GetIdlePosition().z() -
                   x24_tablegroup_leftmenu->GetWorkerWidget(0)->GetIdlePosition().z();

    x28_tablegroup_rightmenu->SetMenuSelectionChangeCallback(
        std::bind(&SOptionsFrontEndFrame::DoMenuSelectionChange, this, std::placeholders::_1, std::placeholders::_2));
    x28_tablegroup_rightmenu->SetMenuCancelCallback(
        std::bind(&SOptionsFrontEndFrame::DoMenuCancel, this, std::placeholders::_1));

    x2c_tablegroup_double->SetMenuSelectionChangeCallback(
        std::bind(&SOptionsFrontEndFrame::DoMenuSelectionChange, this, std::placeholders::_1, std::placeholders::_2));
    x2c_tablegroup_double->SetMenuCancelCallback(
        std::bind(&SOptionsFrontEndFrame::DoMenuCancel, this, std::placeholders::_1));

    x30_tablegroup_triple->SetMenuSelectionChangeCallback(
        std::bind(&SOptionsFrontEndFrame::DoMenuSelectionChange, this, std::placeholders::_1, std::placeholders::_2));
    x30_tablegroup_triple->SetMenuCancelCallback(
        std::bind(&SOptionsFrontEndFrame::DoMenuCancel, this, std::placeholders::_1));

    x34_slidergroup_slider->SetSelectionChangedCallback(
        std::bind(&SOptionsFrontEndFrame::DoSliderChange, this, std::placeholders::_1, std::placeholders::_2));

    FindTextPanePair(x1c_loadedFrame, "textpane_double0").SetPairText(x20_loadedPauseStrg->GetString(95)); // Off
    FindTextPanePair(x1c_loadedFrame, "textpane_double1").SetPairText(x20_loadedPauseStrg->GetString(94)); // On
    FindTextPanePair(x1c_loadedFrame, "textpane_triple0").SetPairText(x20_loadedPauseStrg->GetString(96)); // Mono
    FindTextPanePair(x1c_loadedFrame, "textpane_triple1").SetPairText(x20_loadedPauseStrg->GetString(97)); // Stereo
    FindTextPanePair(x1c_loadedFrame, "textpane_triple2").SetPairText(x20_loadedPauseStrg->GetString(98)); // Dolby

    FindTextPanePair(x1c_loadedFrame, "textpane_title").SetPairText(g_MainStringTable->GetString(99)); // OPTIONS

    if (CGuiTextPane* proceed = static_cast<CGuiTextPane*>(x1c_loadedFrame->FindWidget("textpane_proceed")))
        proceed->TextSupport().SetText(g_MainStringTable->GetString(85));

    if (CGuiTextPane* cancel = static_cast<CGuiTextPane*>(x1c_loadedFrame->FindWidget("textpane_cancel")))
        cancel->TextSupport().SetText(g_MainStringTable->GetString(82));

    // Visor, Display, Sound, Controller
    for (int i=0 ; i<4 ;++i)
    {
        char name[36];
        snprintf(name, 36, "textpane_filename%d", i);
        FindTextPanePair(x1c_loadedFrame, name).SetPairText(x20_loadedPauseStrg->GetString(16+i));
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

bool CFrontEndUI::SOptionsFrontEndFrame::PumpLoad()
{
    if (x1c_loadedFrame)
        return true;
    if (!x4_frme.IsLoaded())
        return false;
    if (!x10_pauseScreen.IsLoaded())
        return false;
    if (!x4_frme->GetIsFinishedLoading())
        return false;
    x1c_loadedFrame = x4_frme.GetObj();
    x20_loadedPauseStrg = x10_pauseScreen.GetObj();
    m_touchBar = NewGameOptionsTouchBar();
    FinishedLoading();
    return true;
}

bool CFrontEndUI::SOptionsFrontEndFrame::ProcessUserInput(const CFinalInput& input, CSaveGameScreen* sui)
{
    x134_25_exitOptions = false;
    if (sui)
        sui->ProcessUserInput(input);
    if (x1c_loadedFrame && x134_24_visible)
    {
        if (input.PB() && x24_tablegroup_leftmenu->GetIsActive())
        {
            x134_25_exitOptions = true;
            CSfxManager::SfxStart(SFXfnt_back, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        }
        else
        {
            x1c_loadedFrame->ProcessUserInput(input);
            int leftSel = x24_tablegroup_leftmenu->GetUserSelection();
            int rightSel = x28_tablegroup_rightmenu->GetUserSelection();
            CGameOptions::TryRestoreDefaults(input, leftSel, rightSel, true, false);

            CGameOptionsTouchBar::EAction tbAction = m_touchBar->PopAction();
            if (x28_tablegroup_rightmenu->GetIsActive())
            {
                if (tbAction == CGameOptionsTouchBar::EAction::Advance && !m_touchBarInValue)
                {
                    int value;
                    m_touchBar->GetSelection(leftSel, rightSel, value);
                    x28_tablegroup_rightmenu->SetUserSelection(rightSel);
                    SetTableColors(x28_tablegroup_rightmenu);
                    HandleRightSelectionChange();
                    const auto& optionCategory = GameOptionsRegistry[leftSel];
                    const SGameOption& option = optionCategory.second[rightSel];
                    if (option.type != EOptionType::RestoreDefaults)
                    {
                        m_touchBarInValue = true;
                        m_touchBarValueDirty = true;
                    }
                    else
                    {
                        CGameOptions::TryRestoreDefaults(input, leftSel, rightSel, true, true);
                    }
                }
                else if (tbAction == CGameOptionsTouchBar::EAction::Back)
                {
                    if (m_touchBarInValue)
                        m_touchBarInValue = false;
                    else
                        DoMenuCancel(x28_tablegroup_rightmenu);
                }
                else if (tbAction == CGameOptionsTouchBar::EAction::ValueChange)
                {
                    int value;
                    m_touchBar->GetSelection(leftSel, rightSel, value);
                    const auto& optionCategory = GameOptionsRegistry[leftSel];
                    const SGameOption& option = optionCategory.second[rightSel];
                    CGameOptions::SetOption(option.option, value);
                    if (option.type != EOptionType::Float)
                        m_touchBarValueDirty = true;
                    HandleRightSelectionChange();
                }
                else
                {
                    if (m_touchBarInValue)
                    {
                        if (m_touchBarValueDirty)
                        {
                            const auto& optionCategory = GameOptionsRegistry[leftSel];
                            const SGameOption& option = optionCategory.second[rightSel];
                            int value = CGameOptions::GetOption(option.option);
                            m_touchBar->SetSelection(leftSel, rightSel, value);
                            m_touchBarValueDirty = false;
                        }
                    }
                    else
                    {
                        m_touchBar->SetSelection(leftSel, -1, -1);
                    }
                }
            }
            else
            {
                if (tbAction == CGameOptionsTouchBar::EAction::Advance)
                {
                    int value;
                    m_touchBar->GetSelection(leftSel, rightSel, value);
                    x24_tablegroup_leftmenu->SetUserSelection(leftSel);
                    SetTableColors(x24_tablegroup_leftmenu);
                    SetRightUIText();
                    DoLeftMenuAdvance(x24_tablegroup_leftmenu);
                }
                if (tbAction == CGameOptionsTouchBar::EAction::Back)
                {
                    x134_25_exitOptions = true;
                    CSfxManager::SfxStart(SFXfnt_back, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
                }
                else
                {
                    m_touchBar->SetSelection(-1, -1, -1);
                }
            }
        }
    }
    return !x134_25_exitOptions;
}

void CFrontEndUI::SOptionsFrontEndFrame::Update(float dt, CSaveGameScreen* sui)
{
    x40_rumbleGen.Update(dt);
    x134_24_visible = !sui || sui->GetUIType() == CSaveGameScreen::EUIType::SaveReady;

    if (!PumpLoad())
        return;

    x0_uiAlpha = std::min(1.f, x0_uiAlpha + dt);
    x1c_loadedFrame->Update(dt);

    bool isSliding = x34_slidergroup_slider->GetState() != CGuiSliderGroup::EState::None;
    if (x3c_sliderSfx.operator bool() != isSliding)
    {
        if (isSliding)
        {
            x3c_sliderSfx = CSfxManager::SfxStart(SFXui_frontend_options_slider_change_lp,
                1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        }
        else
        {
            CSfxManager::SfxStop(x3c_sliderSfx);
            x3c_sliderSfx.reset();
        }
    }
}

void CFrontEndUI::SOptionsFrontEndFrame::Draw() const
{
    if (x1c_loadedFrame && x134_24_visible)
    {
        CGuiWidgetDrawParms params(x0_uiAlpha, zeus::CVector3f::skZero);
        x1c_loadedFrame->Draw(params);
    }
}

CFrontEndUI::CFrontEndUI()
: CIOWin("FrontEndUI")
{
    CMain* m = static_cast<CMain*>(g_Main);

    CRandom16 r(time(nullptr));
    x18_rndA = r.Range(0, 2);
    x1c_rndB = r.Range(0, 2);

    x20_depsGroup = g_SimplePool->GetObj("FrontEnd_DGRP");
    x38_pressStart = g_SimplePool->GetObj("TXTR_PressStart");
    x44_frontendAudioGrp = g_SimplePool->GetObj("FrontEnd_AGSC");

    xdc_saveUI = std::make_unique<CSaveGameScreen>(ESaveContext::FrontEnd, g_GameState->GetCardSerial());

    m->ResetGameState();
    g_GameState->SetCurrentWorldId(g_ResFactory->TranslateOriginalToNew(g_DefaultWorldTag.id));
    g_GameState->GameOptions().ResetToDefaults();
    g_GameState->WriteBackupBuf();

    for (int i=0 ; CDvdFile::FileExists(GetAttractMovieFileName(i).c_str()) ; ++i)
        ++xc0_attractCount;

    m_touchBar = NewFrontEndUITouchBar();
    m_touchBar->SetPhase(CFrontEndUITouchBar::EPhase::None);
}

void CFrontEndUI::StartSlideShow(CArchitectureQueue& queue)
{
    xf4_curAudio->StopMixing();
    queue.Push(MakeMsg::CreateCreateIOWin(EArchMsgTarget::IOWinManager, 12, 11,
                                          std::make_shared<CSlideShow>()));
}

std::string CFrontEndUI::GetAttractMovieFileName(int idx)
{
    return hecl::Format("Video/attract%d.thp", idx);
}

std::string CFrontEndUI::GetNextAttractMovieFileName()
{
    std::string ret = GetAttractMovieFileName(xbc_nextAttract);
    xbc_nextAttract = (xbc_nextAttract + 1) % xc0_attractCount;
    return ret;
}

void CFrontEndUI::SetCurrentMovie(EMenuMovie movie)
{
    if (xb8_curMovie == movie)
        return;
    StopAttractMovie();

    if (xb8_curMovie != EMenuMovie::Stopped)
    {
        xcc_curMoviePtr->SetPlayMode(CMoviePlayer::EPlayMode::Stopped);
        xcc_curMoviePtr->Rewind();
    }

    xb8_curMovie = movie;

    if (xb8_curMovie != EMenuMovie::Stopped)
    {
        xcc_curMoviePtr = x70_menuMovies[int(xb8_curMovie)].get();
        xcc_curMoviePtr->SetPlayMode(CMoviePlayer::EPlayMode::Playing);
    }
    else
        xcc_curMoviePtr = nullptr;
}

void CFrontEndUI::StopAttractMovie()
{
    if (!xc4_attractMovie)
        return;
    xc4_attractMovie.reset();
    xcc_curMoviePtr = nullptr;
}

void CFrontEndUI::StartAttractMovie()
{
    if (xc4_attractMovie)
        return;
    SetCurrentMovie(EMenuMovie::Stopped);
    xc4_attractMovie = std::make_unique<CMoviePlayer>(GetNextAttractMovieFileName().c_str(), 0.f, false, true);
    xcc_curMoviePtr = xc4_attractMovie.get();
}

void CFrontEndUI::StartStateTransition(EScreen screen)
{
    switch (x50_curScreen)
    {
    case EScreen::Title:
        if (screen != EScreen::FileSelect)
            break;
        SetCurrentMovie(EMenuMovie::StartFileSelectA);
        SetFadeBlackTimer(xcc_curMoviePtr->GetTotalSeconds());
        break;
    case EScreen::FileSelect:
        if (screen == EScreen::ToPlayGame)
        {
            SetCurrentMovie(EMenuMovie::FileSelectPlayGameA);
            SetFadeBlackTimer(xcc_curMoviePtr->GetTotalSeconds());
        }
        else if (screen == EScreen::FusionBonus)
        {
            SetCurrentMovie(EMenuMovie::FileSelectGBA);
            SetFadeBlackTimer(xcc_curMoviePtr->GetTotalSeconds());
            CSfxManager::SfxStart(SFXfnt_tofusion_L, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
            CSfxManager::SfxStart(SFXfnt_tofusion_R, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        }
        break;
    case EScreen::FusionBonus:
        if (screen == EScreen::ToPlayGame)
        {
            SetCurrentMovie(EMenuMovie::GBAFileSelectB);
            SetFadeBlackTimer(xcc_curMoviePtr->GetTotalSeconds());
        }
        else if (screen == EScreen::FileSelect)
        {
            SetCurrentMovie(EMenuMovie::GBAFileSelectA);
            SetFadeBlackTimer(xcc_curMoviePtr->GetTotalSeconds());
            CSfxManager::SfxStart(SFXfnt_fromfusion_L, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
            CSfxManager::SfxStart(SFXfnt_fromfusion_R, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        }
    default: break;
    }

    switch (screen)
    {
    case EScreen::OpenCredits:
    case EScreen::Title:
        SetCurrentMovie(EMenuMovie::FirstStart);
        SetFadeBlackTimer(xcc_curMoviePtr->GetTotalSeconds());
        break;
    case EScreen::AttractMovie:
        StartAttractMovie();
        SetFadeBlackWithMovie();
    default: break;
    }

    x54_nextScreen = screen;
}

void CFrontEndUI::CompleteStateTransition()
{
    EScreen oldScreen = x50_curScreen;
    x50_curScreen = x54_nextScreen;

    switch (x50_curScreen)
    {
    case EScreen::AttractMovie:
        x54_nextScreen = EScreen::OpenCredits;
        x50_curScreen = EScreen::OpenCredits;
        xd0_playerSkipToTitle = false;
        StartStateTransition(EScreen::Title);
        break;

    case EScreen::Title:
        SetCurrentMovie(EMenuMovie::StartLoop);
        SetFadeBlackTimer(30.f);
        break;

    case EScreen::FileSelect:
        SetCurrentMovie(EMenuMovie::FileSelectLoop);
        if (oldScreen == EScreen::Title)
        {
            xf4_curAudio->StopMixing();
            xf4_curAudio = xd8_audio2.get();
            xf4_curAudio->StartMixing();
        }
        if (xdc_saveUI)
            xdc_saveUI->ResetCardDriver();
        break;

    case EScreen::FusionBonus:
        SetCurrentMovie(EMenuMovie::GBALoop);
        break;

    case EScreen::ToPlayGame:
        x14_phase = EPhase::ExitFrontEnd;
        break;

    default: break;
    }
}

void CFrontEndUI::HandleDebugMenuReturnValue(CGameDebug::EReturnValue val, CArchitectureQueue& queue)
{}

void CFrontEndUI::Draw() const
{
    if (x14_phase < EPhase::DisplayFrontEnd)
        return;

    if (xec_emuFrme)
    {
        xec_emuFrme->Draw(xdc_saveUI.get());
    }
    else
    {
        //g_Renderer->SetDepthReadWrite(false, false);
        g_Renderer->SetViewportOrtho(false, -4096.f, 4096.f);

        /* Correct movie aspect ratio */
        float vpAspectRatio = g_Viewport.x8_width / float(g_Viewport.xc_height);
        float hPad, vPad;
        if (vpAspectRatio >= 1.78f)
        {
            hPad = 1.78f / vpAspectRatio;
            vPad = 1.78f / 1.33f;
        }
        else
        {
            hPad = 1.f;
            vPad = vpAspectRatio / 1.33f;
        }

        if (xcc_curMoviePtr && xcc_curMoviePtr->GetIsFullyCached())
        {
            /* Render movie */
            xcc_curMoviePtr->SetFrame({-hPad, vPad, 0.f}, {-hPad, -vPad, 0.f}, {hPad, -vPad, 0.f}, {hPad, vPad, 0.f});
            xcc_curMoviePtr->DrawFrame();
        }

        if (x50_curScreen == EScreen::FileSelect && x54_nextScreen == EScreen::FileSelect)
        {
            /* Render active FileSelect UI */
            if (xf0_optionsFrme)
                xf0_optionsFrme->Draw();
            else if (xe0_frontendCardFrme)
                xe0_frontendCardFrme->Draw();
            else
                xe8_frontendNoCardFrme->Draw();
        }
        else if (x50_curScreen == EScreen::FusionBonus && x54_nextScreen == EScreen::FusionBonus)
        {
            /* Render Fusion bonus UI */
            xe4_fusionBonusFrme->Draw();
        }

        if (x64_pressStartAlpha > 0.f && x38_pressStart.IsLoaded() && m_pressStartQuad)
        {
            /* Render "Press Start" */
            zeus::CRectangle rect(0.5f - x38_pressStart->GetWidth() / 2.f / 640.f * hPad,
                                  0.5f + (x38_pressStart->GetHeight() / 2.f - 240.f + 72.f) / 480.f * vPad,
                                  x38_pressStart->GetWidth() / 640.f * hPad,
                                  x38_pressStart->GetHeight() / 480.f * vPad);
            zeus::CColor color = zeus::CColor::skWhite;
            color.a() = x64_pressStartAlpha;
            const_cast<CTexturedQuadFilterAlpha&>(*m_pressStartQuad).draw(color, 1.f, rect);
        }

        if (xc0_attractCount > 0)
        {
            /* Render fade-to-black into attract movie */
            if (((x50_curScreen == EScreen::Title && x54_nextScreen == EScreen::Title) ||
                 x54_nextScreen == EScreen::AttractMovie) && x58_fadeBlackTimer < 1.f)
            {
                /* To black */
                zeus::CColor color = zeus::CColor::skBlack;
                color.a() = 1.f - x58_fadeBlackTimer;
                const_cast<CColoredQuadFilter&>(m_fadeToBlack).draw(color);
            }
        }

        if (xd0_playerSkipToTitle)
        {
            /* Render fade-through-black into title if player skips */
            if (x50_curScreen == EScreen::OpenCredits && x54_nextScreen == EScreen::Title)
            {
                /* To black */
                zeus::CColor color = zeus::CColor::skBlack;
                color.a() = zeus::clamp(0.f, 1.f - x58_fadeBlackTimer, 1.f);
                const_cast<CColoredQuadFilter&>(m_fadeToBlack).draw(color);
            }
            else if (x50_curScreen == EScreen::Title && x54_nextScreen == EScreen::Title)
            {
                /* From black with 30-sec skip to title */
                zeus::CColor color = zeus::CColor::skBlack;
                color.a() = 1.f - zeus::clamp(0.f, 30.f - x58_fadeBlackTimer, 1.f);
                const_cast<CColoredQuadFilter&>(m_fadeToBlack).draw(color);
            }
        }

        if (xdc_saveUI)
        {
            /* Render memory card feedback strings */
            if ((CanShowSaveUI() && !xdc_saveUI->IsHiddenFromFrontEnd()) ||
                ((x50_curScreen == EScreen::FileSelect && x54_nextScreen == EScreen::FileSelect) ||
                 (x50_curScreen == EScreen::FusionBonus && x54_nextScreen == EScreen::FusionBonus)))
                xdc_saveUI->Draw();
        }
    }
}

void CFrontEndUI::UpdateMovies(float dt)
{
    if (xcc_curMoviePtr && x5c_fadeBlackWithMovie)
    {
        /* Set fade-to-black timer to match attract movie */
        x5c_fadeBlackWithMovie = false;
        x58_fadeBlackTimer = xcc_curMoviePtr->GetTotalSeconds();
    }

    /* Advance playing menu movies */
    for (auto& movie : x70_menuMovies)
        if (movie)
            movie->Update(dt);

    /* Advance attract movie */
    if (xc4_attractMovie)
        xc4_attractMovie->Update(dt);
}

void CFrontEndUI::FinishedLoadingDepsGroup()
{
    /* Transfer DGRP tokens into FrontEnd and lock */
    const CDependencyGroup* dgrp = x20_depsGroup.GetObj();
    x2c_deps.reserve(dgrp->GetObjectTagVector().size());
    for (const SObjectTag& tag : dgrp->GetObjectTagVector())
    {
        x2c_deps.push_back(g_SimplePool->GetObj(tag));
        x2c_deps.back().Lock();
    }
    x44_frontendAudioGrp.Lock();
}

bool CFrontEndUI::PumpLoad()
{
    /* Poll all tokens for load completion */
    for (CToken& tok : x2c_deps)
        if (!tok.IsLoaded())
            return false;
    if (!x44_frontendAudioGrp.IsLoaded())
        return false;

    /* Ready to construct texture quads */
    m_pressStartQuad.emplace(EFilterType::Add, x38_pressStart);

    return true;
}

bool CFrontEndUI::PumpMovieLoad()
{
    /* Prepare all FrontEnd movies and pause each */
    if (xd1_moviesLoaded)
        return true;
    for (int i=0 ; i<9 ; ++i)
    {
        if (!x70_menuMovies[i])
        {
            const FEMovie& movie = FEMovies[i];
            std::string path = movie.path;
            if (i == int(EMenuMovie::StartFileSelectA))
            {
                auto pos = path.find("A.thp");
                if (pos != std::string::npos)
                    path[pos] = 'A' + x18_rndA;
            }
            else if (i == int(EMenuMovie::FileSelectPlayGameA))
            {
                auto pos = path.find("A.thp");
                if (pos != std::string::npos)
                    path[pos] = 'A' + x1c_rndB;
            }
            x70_menuMovies[i] = std::make_unique<CMoviePlayer>(path.c_str(), 0.05f, movie.loop, false);
            x70_menuMovies[i]->SetPlayMode(CMoviePlayer::EPlayMode::Stopped);
            return false;
        }
    }
    xd1_moviesLoaded = true;
    return true;
}

void CFrontEndUI::ProcessUserInput(const CFinalInput& input, CArchitectureQueue& queue)
{
    if (static_cast<CMain*>(g_Main)->GetCardBusy())
        return;
    if (input.ControllerIdx() > 1)
        return;

    if (xec_emuFrme)
    {
        /* NES emulator pre-empts user input if active */
        xec_emuFrme->ProcessUserInput(input, xdc_saveUI.get());
        return;
    }

    /* Controllers other than first shall not pass */
    if (x14_phase != EPhase::DisplayFrontEnd || input.ControllerIdx() != 0)
        return;

    /* Pop most recent action from Touch Bar */
    CFrontEndUITouchBar::EAction touchBarAction = m_touchBar->PopAction();

    if (x50_curScreen != x54_nextScreen)
    {
        if (x54_nextScreen == EScreen::AttractMovie && (input.PStart() || input.PA() ||
                                                        touchBarAction == CFrontEndUITouchBar::EAction::Start))
        {
            /* Player wants to return to opening credits from attract movie */
            SetFadeBlackTimer(std::min(1.f, x58_fadeBlackTimer));
            PlayAdvanceSfx();
            return;
        }

        if (input.PA() || input.PStart() || touchBarAction == CFrontEndUITouchBar::EAction::Start)
        {
            if (x50_curScreen == EScreen::OpenCredits && x54_nextScreen == EScreen::Title &&
                x58_fadeBlackTimer > 1.f)
            {
                /* Player is too impatient to view opening credits */
                xd0_playerSkipToTitle = true;
                SetFadeBlackTimer(1.f);
                return;
            }
        }
    }
    else
    {
        if (x50_curScreen == EScreen::Title)
        {
            if (input.PStart() || input.PA() || touchBarAction == CFrontEndUITouchBar::EAction::Start)
            {
                if (x58_fadeBlackTimer < 30.f - g_tweakGame->GetPressStartDelay())
                {
                    /* Proceed to file select UI */
                    CSfxManager::SfxStart(FETransitionBackSFX[x18_rndA][0], 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
                    CSfxManager::SfxStart(FETransitionBackSFX[x18_rndA][1], 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
                    m_touchBar->SetPhase(CFrontEndUITouchBar::EPhase::None);
                    StartStateTransition(EScreen::FileSelect);
                    return;
                }
            }
        }
        else if (x50_curScreen == EScreen::FileSelect && x54_nextScreen == EScreen::FileSelect)
        {
            if (xf0_optionsFrme)
            {
                /* Control options UI */
                if (xf0_optionsFrme->ProcessUserInput(input, xdc_saveUI.get()))
                    return;
                /* Exit options UI */
                m_touchBar->SetPhase(CFrontEndUITouchBar::EPhase::None);
                xf0_optionsFrme.reset();
                return;
            }
            else if (xe0_frontendCardFrme)
            {
                /* Control FrontEnd with memory card */
                switch (xe0_frontendCardFrme->ProcessUserInput(input, touchBarAction))
                {
                case SNewFileSelectFrame::EAction::FusionBonus:
                    m_touchBar->SetPhase(CFrontEndUITouchBar::EPhase::None);
                    StartStateTransition(EScreen::FusionBonus);
                    return;
                case SNewFileSelectFrame::EAction::GameOptions:
                    m_touchBar->SetPhase(CFrontEndUITouchBar::EPhase::None);
                    xf0_optionsFrme = std::make_unique<SOptionsFrontEndFrame>();
                    return;
                case SNewFileSelectFrame::EAction::SlideShow:
                    m_touchBar->SetPhase(CFrontEndUITouchBar::EPhase::None);
                    xd2_deferSlideShow = true;
                    StartSlideShow(queue);
                    return;
                default: return;
                }
            }
            else
            {
                /* Control FrontEnd without memory card */
                switch (xe8_frontendNoCardFrme->ProcessUserInput(input, touchBarAction))
                {
                case SFrontEndFrame::EAction::FusionBonus:
                    m_touchBar->SetPhase(CFrontEndUITouchBar::EPhase::None);
                    StartStateTransition(EScreen::FusionBonus);
                    return;
                case SFrontEndFrame::EAction::GameOptions:
                    m_touchBar->SetPhase(CFrontEndUITouchBar::EPhase::None);
                    xf0_optionsFrme = std::make_unique<SOptionsFrontEndFrame>();
                    return;
                case SFrontEndFrame::EAction::StartGame:
                    TransitionToGame();
                    return;
                case SFrontEndFrame::EAction::SlideShow:
                    m_touchBar->SetPhase(CFrontEndUITouchBar::EPhase::None);
                    xd2_deferSlideShow = true;
                    StartSlideShow(queue);
                    return;
                default: return;
                }
            }
        }
        else if (x50_curScreen == EScreen::FusionBonus && x54_nextScreen == EScreen::FusionBonus)
        {
            /* Control Fusion bonus UI */
            switch (xe4_fusionBonusFrme->ProcessUserInput(input, xdc_saveUI.get(), touchBarAction))
            {
            case SFusionBonusFrame::EAction::GoBack:
                m_touchBar->SetPhase(CFrontEndUITouchBar::EPhase::None);
                StartStateTransition(EScreen::FileSelect);
                return;
            case SFusionBonusFrame::EAction::PlayNESMetroid:
                m_touchBar->SetPhase(CFrontEndUITouchBar::EPhase::None);
                xf4_curAudio->StopMixing();
                xec_emuFrme = std::make_unique<SNesEmulatorFrame>();
                if (xdc_saveUI)
                    xdc_saveUI->SetInGame(true);
                return;
            default: return;
            }
        }
    }
}

void CFrontEndUI::TransitionToGame()
{
    if (x14_phase >= EPhase::ToPlayGame)
        return;

    const u16* sfx = FETransitionForwardSFX[x1c_rndB];
    CSfxManager::SfxStart(sfx[0], 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
    CSfxManager::SfxStart(sfx[1], 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);

    x14_phase = EPhase::ToPlayGame;
    m_touchBar->SetPhase(CFrontEndUITouchBar::EPhase::None);
    StartStateTransition(EScreen::ToPlayGame);
}

void CFrontEndUI::UpdateMusicVolume()
{
    float volMul = (xf4_curAudio == xd4_audio1.get()) ? FE1_VOL : FE2_VOL;
    if (xf4_curAudio)
    {
        float vol = volMul * x68_musicVol * (g_GameState->GameOptions().GetMusicVolume() / 127.f);
        xf4_curAudio->SetVolume(vol);
    }
}

static const float AudioFadeTimeA[] =
{
    0.44f, 5.41f, 3.41f
};

static const float AudioFadeTimeB[] =
{
    4.2f, 6.1f, 6.1f
};

CIOWin::EMessageReturn CFrontEndUI::Update(float dt, CArchitectureQueue& queue)
{
    if (xdc_saveUI && x50_curScreen >= EScreen::FileSelect)
    {
        switch (xdc_saveUI->Update(dt))
        {
        case EMessageReturn::Exit:
            /* Memory card operation complete, transition to game */
            TransitionToGame();
            break;
        case EMessageReturn::RemoveIOWinAndExit:
        case EMessageReturn::RemoveIOWin:
            /* No memory card available, fallback to non-save UI */
            xe0_frontendCardFrme.reset();
            xdc_saveUI.reset();
        default: break;
        }
    }

    /* Set music fade volume */
    UpdateMusicVolume();

    switch (x14_phase)
    {
    case EPhase::LoadDepsGroup:
        /* Poll DGRP load */
        if (!x20_depsGroup.IsLoaded())
            return EMessageReturn::Exit;
        FinishedLoadingDepsGroup();
        x20_depsGroup.Unlock();
        x14_phase = EPhase::LoadDeps;

    case EPhase::LoadDeps:
        /* Poll loading DGRP resources */
        if (PumpLoad())
        {
            xe0_frontendCardFrme = std::make_unique<SNewFileSelectFrame>(xdc_saveUI.get(), x1c_rndB, *m_touchBar);
            xe4_fusionBonusFrme = std::make_unique<SFusionBonusFrame>(*m_touchBar);
            xe8_frontendNoCardFrme = std::make_unique<SFrontEndFrame>(x1c_rndB, *m_touchBar);
            x38_pressStart.GetObj();
            CAudioSys::AddAudioGroup(x44_frontendAudioGrp->GetAudioGroupData());
            xd4_audio1 = std::make_unique<CStaticAudioPlayer>("Audio/frontend_1.rsf", 416480, 1973664);
            xd8_audio2 = std::make_unique<CStaticAudioPlayer>("Audio/frontend_2.rsf", 273556, 1636980);
            x14_phase = EPhase::LoadFrames;
        }
        if (x14_phase == EPhase::LoadDeps)
            return EMessageReturn::Exit;

    case EPhase::LoadFrames:
        /* Poll loading music and FRME resources */
        if (!xd4_audio1->IsReady() || !xd8_audio2->IsReady() ||
            !xe0_frontendCardFrme->PumpLoad() || !xe4_fusionBonusFrme->PumpLoad() ||
            !xe8_frontendNoCardFrme->PumpLoad() || !xdc_saveUI->PumpLoad())
            return EMessageReturn::Exit;
        xf4_curAudio = xd4_audio1.get();
        xf4_curAudio->StartMixing();
        x14_phase = EPhase::LoadMovies;

    case EPhase::LoadMovies:
    {
        /* Poll loading movies */
        bool moviesReady = true;
        if (PumpMovieLoad())
        {
            /* Prime first frame of movies */
            UpdateMovies(dt);
            for (int i=0 ; i<9 ; ++i)
            {
                if (!x70_menuMovies[i]->GetIsFullyCached())
                {
                    moviesReady = false;
                    break;
                }
            }
        }
        else
            moviesReady = false;

        if (moviesReady)
        {
            /* Ready to display FrontEnd */
            x14_phase = EPhase::DisplayFrontEnd;
            m_touchBar->SetPhase(CFrontEndUITouchBar::EPhase::PressStart);
            StartStateTransition(EScreen::Title);
        }
        else
            return EMessageReturn::Exit;
    }

    case EPhase::DisplayFrontEnd:
    case EPhase::ToPlayGame:
        /* Displaying frontend to user */
        if (xec_emuFrme)
        {
            /* Update just the emulator if active */
            if (xec_emuFrme->Update(dt, xdc_saveUI.get()))
            {
                /* Exit emulator */
                xec_emuFrme.reset();
                if (xdc_saveUI)
                    xdc_saveUI->SetInGame(false);
                xf4_curAudio->StartMixing();
            }
            break;
        }

        if (xd2_deferSlideShow)
        {
            /* Start mixing slideshow music */
            xd2_deferSlideShow = false;
            xf4_curAudio->StartMixing();
            if (xdc_saveUI)
                xdc_saveUI->ResetCardDriver();
        }

        if (x50_curScreen == EScreen::FileSelect && x54_nextScreen == EScreen::FileSelect)
        {
            /* Main FrontEnd UI tree active */
            if (xf0_optionsFrme)
            {
                bool optionsActive = true;
                if (xdc_saveUI && xdc_saveUI->GetUIType() != CSaveGameScreen::EUIType::SaveReady)
                    optionsActive = false;

                if (optionsActive)
                {
                    /* Update options UI */
                    xf0_optionsFrme->Update(dt, xdc_saveUI.get());
                }
                else
                {
                    /* Save triggered; exit options UI here */
                    xf0_optionsFrme.reset();
                }
            }
            else if (xe0_frontendCardFrme)
            {
                /* Update FrontEnd with memory card UI */
                xe0_frontendCardFrme->Update(dt);
            }
            else
            {
                /* Update FrontEnd without memory card UI */
                xe8_frontendNoCardFrme->Update(dt);
            }
        }
        else if (x50_curScreen == EScreen::FusionBonus && x54_nextScreen == EScreen::FusionBonus)
        {
            /* Update Fusion bonus UI */
            xe4_fusionBonusFrme->Update(dt, xdc_saveUI.get());
        }

        if (x50_curScreen != x54_nextScreen && xcc_curMoviePtr &&
            (xcc_curMoviePtr->GetIsMovieFinishedPlaying() || xcc_curMoviePtr->IsLooping()))
        {
            /* Movie-based transition complete */
            CompleteStateTransition();
        }

        if (x58_fadeBlackTimer > 0.f && !x5c_fadeBlackWithMovie)
        {
            SetFadeBlackTimer(std::max(0.f, x58_fadeBlackTimer - dt));
            if (x58_fadeBlackTimer == 0.f)
            {
                if (x50_curScreen == EScreen::Title && x54_nextScreen == EScreen::Title)
                {
                    if (xc0_attractCount > 0)
                    {
                        /* Screen black, start attract movie */
                        StartStateTransition(EScreen::AttractMovie);
                    }
                }
                else if (x54_nextScreen == EScreen::AttractMovie)
                {
                    /* Attract movie done, play open credits again */
                    CompleteStateTransition();
                }
                else if (x50_curScreen != x54_nextScreen)
                {
                    /* Fade-based transition complete */
                    CompleteStateTransition();
                }
            }
        }

        /* Advance active movies */
        UpdateMovies(dt);

        if (x50_curScreen == EScreen::Title && x54_nextScreen == EScreen::Title)
        {
            /* Update press-start pulsing */
            if (x58_fadeBlackTimer < 30.f - g_tweakGame->GetPressStartDelay())
            {
                x60_pressStartTime = std::fmod(x60_pressStartTime + dt, 1.f);
                if (x60_pressStartTime < 0.5f)
                    x64_pressStartAlpha = x60_pressStartTime / 0.5f;
                else
                    x64_pressStartAlpha = (1.f - x60_pressStartTime) / 0.5f;
            }
        }
        else
        {
            /* Clear press-start pulsing */
            x60_pressStartTime = 0.f;
            x64_pressStartAlpha = 0.f;
        }

        if (x50_curScreen == EScreen::Title && x54_nextScreen == EScreen::FileSelect)
        {
            /* Fade out title music */
            x68_musicVol = 1.f - zeus::clamp(0.f, (xcc_curMoviePtr->GetPlayedSeconds() -
                                                   AudioFadeTimeA[x18_rndA]) / 2.5f, 1.f);
        }
        else if (x54_nextScreen == EScreen::ToPlayGame)
        {
            /* Fade out menu music */
            float delay = AudioFadeTimeB[x1c_rndB];
            x68_musicVol = 1.f - zeus::clamp(0.f, (xcc_curMoviePtr->GetPlayedSeconds() - delay) /
                                                  (xcc_curMoviePtr->GetTotalSeconds() - delay), 1.f);
        }
        else
        {
            /* Full music volume */
            x68_musicVol = 1.f;
        }

        return EMessageReturn::Exit;

    case EPhase::ExitFrontEnd:
        /* Remove FrontEnd IOWin and begin updating next IOWin */
        return EMessageReturn::RemoveIOWin;

    default: break;
    }

    return EMessageReturn::Exit;
}

CIOWin::EMessageReturn CFrontEndUI::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue)
{
    switch (msg.GetType())
    {
    case EArchMsgType::UserInput:
    {
        /* Forward user events */
        const CArchMsgParmUserInput& input = MakeMsg::GetParmUserInput(msg);
        ProcessUserInput(input.x4_parm, queue);
        break;
    }
    case EArchMsgType::TimerTick:
    {
        /* Forward frame events */
        float dt = MakeMsg::GetParmTimerTick(msg).x4_parm;
        return Update(dt, queue);
    }
    case EArchMsgType::QuitGameplay:
    {
        /* Immediately exit FrontEnd */
        x14_phase = EPhase::ExitFrontEnd;
        break;
    }
    default: break;
    }
    return EMessageReturn::Normal;
}

}
