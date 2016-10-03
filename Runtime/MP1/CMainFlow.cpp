#include "CMainFlow.hpp"
#include "MP1.hpp"
#include "CArchitectureQueue.hpp"
#include "CMFGame.hpp"
#include "CPlayMovie.hpp"
#include "CResFactory.hpp"
#include "CFrontEndUI.hpp"
#include "GameGlobalObjects.hpp"
#include "Character/CCharLayoutInfo.hpp"

namespace urde
{
namespace MP1
{

void CMainFlow::AdvanceGameState(CArchitectureQueue& queue)
{
    switch (x14_gameState)
    {
    case EClientFlowStates::Unspecified:
        CMainFlow::SetGameState(EClientFlowStates::FrontEnd, queue);
        break;
    case EClientFlowStates::FrontEnd:
        CMainFlow::SetGameState(EClientFlowStates::GameLoad, queue);
        break;
    case EClientFlowStates::GameLoad:
        CMainFlow::SetGameState(EClientFlowStates::MoviePlay, queue);
        break;
    case EClientFlowStates::MoviePlay:
        CMainFlow::SetGameState(EClientFlowStates::FrontEnd, queue);
        break;
    }
}

void CMainFlow::SetGameState(EClientFlowStates state, CArchitectureQueue& queue)
{
    switch (state)
    {
    case EClientFlowStates::FrontEnd:
    {
        if (g_Main->GetGameplayResult() == EGameplayResult::None)
        {
            g_Main->SetGameplayResult(EGameplayResult::Playing);
            break;
        }
        /* TODO: URDE handling
        CResLoader& loader = g_ResFactory->GetLoader();
        while (!loader.AreAllPaksLoaded())
            loader.AsyncIdlePakLoading();
        */
        g_Main->LoadAudio();
        g_Main->RegisterResourceTweaks();
        queue.Push(MakeMsg::CreateCreateIOWin(EArchMsgTarget::IOWinManager, 12, 11, new CFrontEndUI(queue)));
        break;
    }
    case EClientFlowStates::GameLoad:
    {
        queue.Push(MakeMsg::CreateCreateIOWin(EArchMsgTarget::IOWinManager, 10, 1000, new CMFGameLoader()));
        break;
    }
    case EClientFlowStates::MoviePlay:
    {
        switch (g_Main->GetGameplayResult())
        {
        case EGameplayResult::Win:
            queue.Push(MakeMsg::CreateCreateIOWin(EArchMsgTarget::IOWinManager, 12, 11, new CPlayMovie(CPlayMovie::EWhichMovie::WinGame)));
            break;
        case EGameplayResult::Lose:
            queue.Push(MakeMsg::CreateCreateIOWin(EArchMsgTarget::IOWinManager, 12, 11, new CPlayMovie(CPlayMovie::EWhichMovie::LoseGame)));
            break;
        default: break;
        }
        break;
    }
    default: break;
    }
}


}
}
