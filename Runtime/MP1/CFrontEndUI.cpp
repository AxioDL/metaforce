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
#include "GuiSys/CSaveUI.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CStringTable.hpp"
#include "GuiSys/CGuiTableGroup.hpp"
#include "GuiSys/CGuiModel.hpp"
#include "CGameState.hpp"
#include "CDependencyGroup.hpp"
#include "Audio/CAudioGroupSet.hpp"
#include "CNESEmulator.hpp"

namespace urde
{
namespace MP1
{

/* Music volume constants */
#define FE1_VOL 0.7421875f
#define FE2_VOL 0.7421875f

/* L/R Stereo transition cues */
static const u16 FETransitionBackSFX[3][2] =
{
    {1090, 1097},
    {1098, 1099},
    {1100, 1101}
};

static const u16 FETransitionForwardSFX[3][2] =
{
    {1102, 1103},
    {1104, 1105},
    {1106, 1107}
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

CFrontEndUI::SNewFileSelectFrame::SNewFileSelectFrame(CSaveUI* sui, u32 rnd)
: x0_rnd(rnd), x4_saveUI(sui)
{
    x10_frme = g_SimplePool->GetObj("FRME_NewFileSelect");
}

CFrontEndUI::SFileSelectOption CFrontEndUI::FindFileSelectOption(CGuiFrame* frame, int idx)
{
    SFileSelectOption ret;
    ret.x0_base = frame->FindWidget(hecl::Format("basewidget_file%d", idx).c_str());
    ret.x4_textpanes[0] = FindTextPanePair(frame, hecl::Format("textpane_filename%d", idx).c_str());
    ret.x4_textpanes[1] = FindTextPanePair(frame, hecl::Format("textpane_world%d", idx).c_str());
    ret.x4_textpanes[2] = FindTextPanePair(frame, hecl::Format("textpane_playtime%d", idx).c_str());
    ret.x4_textpanes[3] = FindTextPanePair(frame, hecl::Format("textpane_date%d", idx).c_str());
    return ret;
}

void CFrontEndUI::SNewFileSelectFrame::FinishedLoading()
{
    x20_tablegroup_fileselect = static_cast<CGuiTableGroup*>(x1c_loadedFrame->FindWidget("tablegroup_fileselect"));
    x24_model_erase = static_cast<CGuiModel*>(x1c_loadedFrame->FindWidget("model_erase"));
    xf8_model_erase_position = x24_model_erase->GetLocalPosition();
    x28_textpane_erase = FindTextPanePair(x1c_loadedFrame, "textpane_erase");
    x58_textpane_popupextra = FindTextPanePair(x1c_loadedFrame, "textpane_popupextra");
    x40_tablegroup_popup = static_cast<CGuiTableGroup*>(x1c_loadedFrame->FindWidget("tablegroup_popup"));
    x44_model_dash7 = static_cast<CGuiModel*>(x1c_loadedFrame->FindWidget("model_dash7"));
    x60_textpane_cancel = static_cast<CGuiTextPane*>(x1c_loadedFrame->FindWidget("textpane_cancel"));
    FindAndSetPairText(x1c_loadedFrame, "textpane_title", g_MainStringTable->GetString(97));
    CGuiTextPane* proceed = static_cast<CGuiTextPane*>(x1c_loadedFrame->FindWidget("textpane_proceed"));
    if (proceed)
        proceed->TextSupport()->SetText(g_MainStringTable->GetString(85));
    x40_tablegroup_popup->SetIsVisible(false);
    x40_tablegroup_popup->SetIsActive(false);
    x40_tablegroup_popup->SetD1(false);
    CGuiWidget* worker = x40_tablegroup_popup->GetWorkerWidget(2);
    worker->SetB627(false);
    worker->SetVisibility(false, ETraversalMode::Children);

    x20_tablegroup_fileselect->SetMenuAdvanceCallback(
        std::bind(&SNewFileSelectFrame::DoFileselectAdvance, this, std::placeholders::_1));
    x20_tablegroup_fileselect->SetMenuSelectionChangeCallback(
        std::bind(&SNewFileSelectFrame::DoSelectionChange, this, std::placeholders::_1));
    x20_tablegroup_fileselect->SetMenuCancelCallback(
        std::bind(&SNewFileSelectFrame::DoFileselectCancel, this, std::placeholders::_1));

    x40_tablegroup_popup->SetMenuAdvanceCallback(
        std::bind(&SNewFileSelectFrame::DoPopupAdvance, this, std::placeholders::_1));
    x40_tablegroup_popup->SetMenuSelectionChangeCallback(
        std::bind(&SNewFileSelectFrame::DoSelectionChange, this, std::placeholders::_1));
    x40_tablegroup_popup->SetMenuCancelCallback(
        std::bind(&SNewFileSelectFrame::DoPopupCancel, this, std::placeholders::_1));

    for (int i=0 ; i<3 ; ++i)
        x64_fileSelections[i] = FindFileSelectOption(x1c_loadedFrame, i);

    x104_rowPitch = (x64_fileSelections[1].x0_base->GetLocalPosition() - x64_fileSelections[0].x0_base->GetLocalPosition()).z;
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
    if (x64_fileSelections[0].x28_ != 4)
        return false;
    if (x64_fileSelections[1].x28_ != 4)
        return false;
    if (x64_fileSelections[2].x28_ != 4)
        return false;
    if (!x28_textpane_erase.x0_panes[0]->GetTextSupport()->AnimationDone())
        return false;
    return x38_.x0_panes[0]->GetTextSupport()->AnimationDone();
}

CFrontEndUI::SNewFileSelectFrame::EPhase
CFrontEndUI::SNewFileSelectFrame::ProcessUserInput(const CFinalInput& input)
{
    if (x8_ != 2)
        x4_saveUI->ProcessUserInput(input);
    if (IsTextDoneAnimating())
        x108_curTime = std::min(0.5f, x108_curTime + input.DeltaTime());
    if (x108_curTime < 0.5f)
        return xc_phase;
    if (x10c_inputEnable)
        x1c_loadedFrame->ProcessUserInput(input);
    if (x10d_needsToggle)
    {
        if (x40_tablegroup_popup->GetIsActive())
            DeactivatePopup();
        else
            ActivatePopup();
    }
    return xc_phase;
}

void CFrontEndUI::SNewFileSelectFrame::HandleActiveChange(CGuiTableGroup* active)
{
    if (!active)
        return;
    active->SetColors(zeus::CColor::skWhite,
                      zeus::CColor{0.627450f, 0.627450f, 0.627450f, 0.784313f});

    if (active == x20_tablegroup_fileselect)
        x24_model_erase->SetLocalTransform(zeus::CTransform::Translate(
            zeus::CVector3f{0.f, 0.f, active->GetUserSelection() * x104_rowPitch} + xf8_model_erase_position));

    if (x8_ == 0 || x8_ == 3)
        x24_model_erase->SetIsVisible(false);
    else
        x24_model_erase->SetIsVisible(true);
}

void CFrontEndUI::SNewFileSelectFrame::DeactivatePopup()
{
    x40_tablegroup_popup->SetIsActive(false);
    x40_tablegroup_popup->SetIsVisible(false);
    x20_tablegroup_fileselect->SetIsActive(true);
    HandleActiveChange(x20_tablegroup_fileselect);
    x64_fileSelections[x20_tablegroup_fileselect->GetUserSelection()].
        x0_base->SetColor(zeus::CColor::skWhite);
}

void CFrontEndUI::SNewFileSelectFrame::ActivatePopup()
{
    x40_tablegroup_popup->SetIsActive(true);
    x40_tablegroup_popup->SetIsVisible(true);
    x40_tablegroup_popup->SetLocalTransform(
        zeus::CTransform::Translate(0.f, 0.f, x20_tablegroup_fileselect->GetUserSelection() * x104_rowPitch) *
            x40_tablegroup_popup->GetTransform());
    x20_tablegroup_fileselect->SetIsActive(false);
    x8_ = 2;
    HandleActiveChange(x40_tablegroup_popup);
    x48_.SetPairText(g_MainStringTable->GetString(95));
    x50_.SetPairText(g_MainStringTable->GetString(38));
    x64_fileSelections[x20_tablegroup_fileselect->GetUserSelection()].
        x0_base->SetColor(zeus::CColor{1.f, 1.f, 1.f, 0.f});
    x44_model_dash7->SetVisibility(false, ETraversalMode::Children);
}

void CFrontEndUI::SNewFileSelectFrame::DoPopupCancel(const CGuiTableGroup* caller)
{

}

void CFrontEndUI::SNewFileSelectFrame::DoPopupAdvance(const CGuiTableGroup* caller)
{

}

void CFrontEndUI::SNewFileSelectFrame::DoFileselectCancel(const CGuiTableGroup* caller)
{

}

void CFrontEndUI::SNewFileSelectFrame::DoSelectionChange(const CGuiTableGroup* caller)
{

}

void CFrontEndUI::SNewFileSelectFrame::DoFileselectAdvance(const CGuiTableGroup* caller)
{

}

CFrontEndUI::SGBASupportFrame::SGBASupportFrame()
{
    x4_gbaSupport = std::make_unique<CGBASupport>();
    xc_gbaScreen = g_SimplePool->GetObj("FRME_GBAScreen");
    x18_gbaLink = g_SimplePool->GetObj("FRME_GBALink");
}

void CFrontEndUI::SGBASupportFrame::FinishedLoading()
{
    x28_tablegroup_options = static_cast<CGuiTableGroup*>(x24_loadedFrame->FindWidget("tablegroup_options"));
    x2c_tablegroup_fusionsuit = static_cast<CGuiTableGroup*>(x24_loadedFrame->FindWidget("tablegroup_fusionsuit"));
    x30_textpane_instructions = FindTextPanePair(x24_loadedFrame, "textpane_instructions");
    FindAndSetPairText(x24_loadedFrame, "textpane_nes", g_MainStringTable->GetString(66));
    FindAndSetPairText(x24_loadedFrame, "textpane_fusionsuit", g_MainStringTable->GetString(63));
    FindAndSetPairText(x24_loadedFrame, "textpane_fusionsuitno", g_MainStringTable->GetString(65));
    FindAndSetPairText(x24_loadedFrame, "textpane_fusionsuityes", g_MainStringTable->GetString(64));
    FindAndSetPairText(x24_loadedFrame, "textpane_title", g_MainStringTable->GetString(100));
    static_cast<CGuiTextPane*>(x24_loadedFrame->FindWidget("textpane_proceed"))->
        TextSupport()->SetText(g_MainStringTable->GetString(85));
    static_cast<CGuiTextPane*>(x24_loadedFrame->FindWidget("textpane_cancel"))->
        TextSupport()->SetText(g_MainStringTable->GetString(82));
    x2c_tablegroup_fusionsuit->SetIsActive(false);
    x2c_tablegroup_fusionsuit->SetIsVisible(false);
    x2c_tablegroup_fusionsuit->SetD1(false);
    x2c_tablegroup_fusionsuit->SetUserSelection(g_GameState->SystemOptions().PlayerHasFusion());

    SetTableColors(x28_tablegroup_options);
    SetTableColors(x2c_tablegroup_fusionsuit);

    x28_tablegroup_options->SetMenuAdvanceCallback(
        std::bind(&SGBASupportFrame::DoOptionsAdvance, this, std::placeholders::_1));
    x28_tablegroup_options->SetMenuSelectionChangeCallback(
        std::bind(&SGBASupportFrame::DoSelectionChange, this, std::placeholders::_1));
    x28_tablegroup_options->SetMenuCancelCallback(
        std::bind(&SGBASupportFrame::DoOptionsCancel, this, std::placeholders::_1));
    x2c_tablegroup_fusionsuit->SetMenuSelectionChangeCallback(
        std::bind(&SGBASupportFrame::DoSelectionChange, this, std::placeholders::_1));
}

bool CFrontEndUI::SGBASupportFrame::PumpLoad()
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

void CFrontEndUI::SGBASupportFrame::SetTableColors(CGuiTableGroup* tbgp) const
{
    tbgp->SetColors(zeus::CColor::skWhite,
                    zeus::CColor{0.627450f, 0.627450f, 0.627450f, 0.784313f});
}

void CFrontEndUI::SGBASupportFrame::ProcessUserInput(const CFinalInput& input, CSaveUI* sui)
{

}

void CFrontEndUI::SGBASupportFrame::DoOptionsCancel(const CGuiTableGroup* caller)
{

}

void CFrontEndUI::SGBASupportFrame::DoSelectionChange(const CGuiTableGroup* caller)
{

}

void CFrontEndUI::SGBASupportFrame::DoOptionsAdvance(const CGuiTableGroup* caller)
{

}

void CFrontEndUI::SGuiTextPair::SetPairText(const std::wstring& str)
{
    x0_panes[0]->TextSupport()->SetText(str);
    x0_panes[1]->TextSupport()->SetText(str);
}

CFrontEndUI::SGuiTextPair CFrontEndUI::FindTextPanePair(CGuiFrame* frame, const char* name)
{
    SGuiTextPair ret;
    ret.x0_panes[0] = static_cast<CGuiTextPane*>(frame->FindWidget(name));
    ret.x0_panes[1] = static_cast<CGuiTextPane*>(frame->FindWidget(hecl::Format("%sb", name).c_str()));
    return ret;
}

void CFrontEndUI::FindAndSetPairText(CGuiFrame* frame, const char* name, const std::wstring& str)
{
    CGuiTextPane* w1 = static_cast<CGuiTextPane*>(frame->FindWidget(name));
    w1->TextSupport()->SetText(str);
    CGuiTextPane* w2 = static_cast<CGuiTextPane*>(frame->FindWidget(hecl::Format("%sb", name).c_str()));
    w2->TextSupport()->SetText(str);
}

void CFrontEndUI::SFrontEndFrame::FinishedLoading()
{
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
        proceed->TextSupport()->SetText(g_MainStringTable->GetString(85));

    x18_tablegroup_mainmenu->SetMenuAdvanceCallback(
        std::bind(&SFrontEndFrame::DoAdvance, this, std::placeholders::_1));
    x18_tablegroup_mainmenu->SetMenuSelectionChangeCallback(
        std::bind(&SFrontEndFrame::DoSelectionChange, this, std::placeholders::_1));
    x18_tablegroup_mainmenu->SetMenuCancelCallback(
        std::bind(&SFrontEndFrame::DoCancel, this, std::placeholders::_1));
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

void CFrontEndUI::SFrontEndFrame::ProcessUserInput(const CFinalInput& input)
{

}

void CFrontEndUI::SFrontEndFrame::DoCancel(const CGuiTableGroup* caller)
{

}

void CFrontEndUI::SFrontEndFrame::DoSelectionChange(const CGuiTableGroup* caller)
{

}

void CFrontEndUI::SFrontEndFrame::DoAdvance(const CGuiTableGroup* caller)
{

}

CFrontEndUI::SFrontEndFrame::SFrontEndFrame(u32 rnd)
: x0_rnd(rnd)
{
    x8_frme = g_SimplePool->GetObj("FRME_FrontEndPL");
}

CFrontEndUI::SFusionBonusFrame::SFusionBonusFrame()
{
    x4_nesEmu = std::make_unique<CNESEmulator>();

    const SObjectTag* deface = g_ResFactory->GetResourceIdByName("FONT_Deface14B");
    CGuiTextProperties props(false, true, EJustification::Left,
                             EVerticalJustification::Center,
                             ETextDirection::Horizontal);
    xc_textSupport = std::make_unique<CGuiTextSupport>(deface->id, props, zeus::CColor::skWhite,
                                                       zeus::CColor::skBlack, zeus::CColor::skWhite,
                                                       0, 0, g_SimplePool);
}

bool CFrontEndUI::SFusionBonusFrame::DoUpdateWithSaveUI(float dt, CSaveUI* saveUi)
{
    bool flag = (saveUi && saveUi->x10_ != 16) ? false : true;
    x10_remTime = std::max(x10_remTime - dt, 0.f);

    zeus::CColor geomCol(zeus::CColor::skWhite);
    geomCol.a = std::min(x10_remTime, 1.f);
    xc_textSupport->SetGeometryColor(geomCol);
    if (xc_textSupport->GetIsTextSupportFinishedLoading())
    {

    }
}

CFrontEndUI::SOptionsFrontEndFrame::SOptionsFrontEndFrame()
{
    x4_frme = g_SimplePool->GetObj("FRME_OptionsFrontEnd");
    x10_pauseScreen = g_SimplePool->GetObj("STRG_PauseScreen");
}

CFrontEndUI::CFrontEndUI(CArchitectureQueue& queue)
: CIOWin("FrontEndUI")
{
    x18_rndA = std::min(rand() * 3 / RAND_MAX, 2);
    x1c_rndB = std::min(rand() * 3 / RAND_MAX, 2);

    x20_depsGroup = g_SimplePool->GetObj("FrontEnd_DGRP");
    x38_pressStart = g_SimplePool->GetObj("TXTR_PressStart");
    x44_frontendAudioGrp = g_SimplePool->GetObj("FrontEnd_AGSC");

    g_Main->ResetGameState();
    g_GameState->SetCurrentWorldId(g_DefaultWorldTag.id);

    for (int i=0 ; CDvdFile::FileExists(GetAttractMovieFileName(i).c_str()) ; ++i)
        ++xc0_attractCount;
}

void CFrontEndUI::OnSliderSelectionChange(CGuiSliderGroup* grp, float)
{}

void CFrontEndUI::OnCheckBoxSelectionChange(CGuiTableGroup* grp)
{}

void CFrontEndUI::OnOptionSubMenuCancel(CGuiTableGroup* grp)
{}

void CFrontEndUI::OnOptionsMenuCancel(CGuiTableGroup* grp)
{}

void CFrontEndUI::OnNewGameMenuCancel(CGuiTableGroup* grp)
{}

void CFrontEndUI::OnFileMenuCancel(CGuiTableGroup* grp)
{}

void CFrontEndUI::OnGenericMenuSelectionChange(CGuiTableGroup* grp, int, int)
{}

void CFrontEndUI::OnOptionsMenuAdvance(CGuiTableGroup* grp)
{}

void CFrontEndUI::OnNewGameMenuAdvance(CGuiTableGroup* grp)
{}

void CFrontEndUI::OnFileMenuAdvance(CGuiTableGroup* grp)
{}

void CFrontEndUI::OnMainMenuAdvance(CGuiTableGroup* grp)
{}

void CFrontEndUI::StartSlideShow(CArchitectureQueue& queue)
{
    queue.Push(MakeMsg::CreateCreateIOWin(EArchMsgTarget::IOWinManager, 12, 11, new CSlideShow()));
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

void CFrontEndUI::UpdateMenuHighlights(CGuiTableGroup* grp)
{}

void CFrontEndUI::CompleteStateTransition()
{}

bool CFrontEndUI::CanBuild(const SObjectTag& tag)
{
    return false;
}

void CFrontEndUI::StartStateTransition(EScreen screen)
{
    switch (x50_curScreen)
    {
    case EScreen::One:
        if (screen != EScreen::Three)
            break;
        SetCurrentMovie(EMenuMovie::StartFileSelectA);
        SetMovieSeconds(xcc_curMoviePtr->GetTotalSeconds());
        break;
    case EScreen::Three:
        if (screen == EScreen::Five)
        {
            SetCurrentMovie(EMenuMovie::FileSelectPlayGameA);
            SetMovieSeconds(xcc_curMoviePtr->GetTotalSeconds());
        }
        else if (screen == EScreen::Four)
        {
            SetCurrentMovie(EMenuMovie::FileSelectGBA);
            SetMovieSeconds(xcc_curMoviePtr->GetTotalSeconds());
            CSfxManager::SfxStart(1108, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
            CSfxManager::SfxStart(1109, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        }
        break;
    case EScreen::Four:
        if (screen == EScreen::Five)
        {
            SetCurrentMovie(EMenuMovie::GBAFileSelectB);
            SetMovieSeconds(xcc_curMoviePtr->GetTotalSeconds());
        }
        else if (screen == EScreen::Three)
        {
            SetCurrentMovie(EMenuMovie::GBAFileSelectA);
            SetMovieSeconds(xcc_curMoviePtr->GetTotalSeconds());
            CSfxManager::SfxStart(1110, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
            CSfxManager::SfxStart(1111, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        }
    default: break;
    }

    switch (screen)
    {
    case EScreen::Zero:
    case EScreen::One:
        SetCurrentMovie(EMenuMovie::FirstStart);
        SetMovieSeconds(xcc_curMoviePtr->GetTotalSeconds());
        break;
    case EScreen::Two:
        StartAttractMovie();
        SetMovieSecondsDeferred();
    default: break;
    }

    x54_nextScreen = screen;
}

void CFrontEndUI::HandleDebugMenuReturnValue(CGameDebug::EReturnValue val, CArchitectureQueue& queue)
{}

void CFrontEndUI::Draw() const
{
    //printf("DRAW\n");
}

void CFrontEndUI::UpdateMovies(float dt)
{
    if (xcc_curMoviePtr && x5c_movieSecondsNeeded)
    {
        x5c_movieSecondsNeeded = false;
        x58_movieSeconds = xcc_curMoviePtr->GetTotalSeconds();
    }

    for (auto& movie : x70_menuMovies)
        if (movie)
            movie->Update(dt);

    if (xc4_attractMovie)
        xc4_attractMovie->Update(dt);
}

void CFrontEndUI::FinishedLoadingDepsGroup()
{
}

bool CFrontEndUI::PumpLoad()
{
    return false;
}

bool CFrontEndUI::PumpMovieLoad()
{
    if (xd1_moviesLoaded)
        return true;
    for (int i=0 ; i<9 ; ++i)
    {
        if (!x70_menuMovies[i])
        {
            const FEMovie& movie = FEMovies[i];
            x70_menuMovies[i] = std::make_unique<CMoviePlayer>(movie.path, 0.05f, movie.loop, false);
            x70_menuMovies[i]->SetPlayMode(CMoviePlayer::EPlayMode::Stopped);
            return false;
        }
    }
    xd1_moviesLoaded = true;
    return true;
}

void CFrontEndUI::ProcessUserInput(const CFinalInput& input, CArchitectureQueue& queue)
{
}

void CFrontEndUI::TransitionToFive()
{
    if (x14_phase >= EPhase::Five)
        return;

    const u16* sfx = FETransitionForwardSFX[x1c_rndB];
    CSfxManager::SfxStart(sfx[0], 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
    CSfxManager::SfxStart(sfx[1], 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);

    x14_phase = EPhase::Five;
    StartStateTransition(EScreen::Five);
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

CIOWin::EMessageReturn CFrontEndUI::Update(float dt, CArchitectureQueue& queue)
{
    if (xdc_saveUI && x50_curScreen >= EScreen::Three)
    {
        switch (xdc_saveUI->Update(dt))
        {
        case EMessageReturn::Exit:
            TransitionToFive();
            break;
        case EMessageReturn::RemoveIOWinAndExit:
        case EMessageReturn::RemoveIOWin:
            xe0_newFileSel.reset();
            xdc_saveUI.reset();
        default: break;
        }
    }

    UpdateMusicVolume();

    switch (x14_phase)
    {
    case EPhase::Zero:
        if (!x20_depsGroup.IsLoaded())
            return EMessageReturn::Exit;
        FinishedLoadingDepsGroup();
        x20_depsGroup.Unlock();
        x14_phase = EPhase::One;

    case EPhase::One:
        if (PumpLoad())
        {
            xe0_newFileSel = std::make_unique<SNewFileSelectFrame>(xdc_saveUI.get(), x1c_rndB);
            xe4_gbaSupportFrme = std::make_unique<SGBASupportFrame>();
            xe8_frontendFrme = std::make_unique<SFrontEndFrame>(x1c_rndB);
            x38_pressStart.GetObj();
            CAudioSys::AddAudioGroup(x44_frontendAudioGrp->GetAudioGroupData());
            xd4_audio1 = std::make_unique<CStaticAudioPlayer>("Audio/frontend_1.rsf", 416480, 1973664);
            xd8_audio2 = std::make_unique<CStaticAudioPlayer>("Audio/frontend_2.rsf", 273556, 1636980);
            x14_phase = EPhase::Two;
        }
        if (x14_phase == EPhase::One)
            return EMessageReturn::Exit;

    case EPhase::Two:
        if (!xd4_audio1->IsReady() || !xd8_audio2->IsReady() ||
            !xe0_newFileSel->PumpLoad() || !xe4_gbaSupportFrme->PumpLoad() ||
            !xe8_frontendFrme->PumpLoad() || !xdc_saveUI->PumpLoad())
            return EMessageReturn::Exit;
        xf4_curAudio = xd4_audio1.get();
        xf4_curAudio->StartMixing();
        x14_phase = EPhase::Three;

    case EPhase::Three:
    {
        bool moviesReady = true;
        if (PumpMovieLoad())
        {
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
            x14_phase = EPhase::Four;
            StartStateTransition(EScreen::One);
        }
        else
            return EMessageReturn::Exit;
    }
    case EPhase::Four:
    case EPhase::Five:

        if (xec_fusionFrme)
        {
            if (xec_fusionFrme->DoUpdateWithSaveUI(dt, xdc_saveUI.get()))
            {
                xec_fusionFrme.reset();
                xf4_curAudio->StartMixing();
            }
            break;
        }
    }

    return EMessageReturn::Exit;
}

CIOWin::EMessageReturn CFrontEndUI::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue)
{
    switch (msg.GetType())
    {
    case EArchMsgType::UserInput:
    {
        const CArchMsgParmUserInput& input = MakeMsg::GetParmUserInput(msg);
        ProcessUserInput(input.x4_parm, queue);
        break;
    }
    case EArchMsgType::TimerTick:
    {
        float dt = MakeMsg::GetParmTimerTick(msg).x4_parm;
        return Update(dt, queue);
    }
    case EArchMsgType::QuitGameplay:
    {
        x14_phase = EPhase::Six;
        break;
    }
    default: break;
    }
    return EMessageReturn::Normal;
}

void CFrontEndUI::StartGame()
{}

void CFrontEndUI::InitializeFrame()
{}

}
}
