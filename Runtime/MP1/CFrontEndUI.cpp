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

namespace urde
{
namespace MP1
{

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

bool CFrontEndUI::PumpMovieLoad()
{
    if (xd1_moviesLoaded)
        return true;
    for (int i=0 ; i<9 ; ++i)
    {
        if (!x70_menuMovies[i])
        {
            x70_menuMovies[i] = std::make_unique<CMoviePlayer>(FEMovies[i].path, 0.05f, FEMovies[i].loop, false);
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
    if (x14_screen >= EScreen::Five)
        return;

    const u16* sfx = FETransitionForwardSFX[x1c_rndB];
    CSfxManager::SfxStart(sfx[0], 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
    CSfxManager::SfxStart(sfx[1], 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);

    x14_screen = EScreen::Five;
    StartStateTransition(EScreen::Five);
}

CIOWin::EMessageReturn CFrontEndUI::Update(float dt, CArchitectureQueue& queue)
{
    //printf("UPDATE\n");

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
        x14_screen = EScreen::Six;
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
