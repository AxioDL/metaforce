#include "CFrontEndUI.hpp"
#include "CArchitectureMessage.hpp"
#include "CArchitectureQueue.hpp"
#include "CDvdFile.hpp"
#include "CSimplePool.hpp"
#include "GameGlobalObjects.hpp"
#include "MP1.hpp"
#include "CSlideShow.hpp"

namespace urde
{
namespace MP1
{

SObjectTag g_DefaultWorldTag = {FOURCC('MLVL'), 0x158efe17};

CFrontEndUI::CFrontEndUI(CArchitectureQueue& queue)
: CIOWin("FrontEndUI")
{
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
    queue.Push(std::move(MakeMsg::CreateCreateIOWin(EArchMsgTarget::IOWinManager, 12, 11, new CSlideShow())));
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
{}

void CFrontEndUI::StopAttractMovie()
{}

void CFrontEndUI::StartAttractMovie(int idx)
{}

void CFrontEndUI::UpdateMenuHighlights(CGuiTableGroup* grp)
{}

void CFrontEndUI::CompleteStateTransition()
{}

bool CFrontEndUI::CanBuild(const SObjectTag& tag)
{
    return false;
}

void CFrontEndUI::StartStateTransition(EScreen screen)
{}

void CFrontEndUI::HandleDebugMenuReturnValue(CGameDebug::EReturnValue val, CArchitectureQueue& queue)
{}

void CFrontEndUI::Draw() const
{
    printf("DRAW\n");
}

void CFrontEndUI::UpdateMovies(float dt)
{}

void CFrontEndUI::ProcessUserInput(const CFinalInput& input, CArchitectureQueue& queue)
{
}

CIOWin::EMessageReturn CFrontEndUI::Update(float dt, CArchitectureQueue& queue)
{
    printf("UPDATE\n");
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
        x14_phase = Phase::Six;
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
