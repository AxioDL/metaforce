#include "CMainFlow.hpp"
#include "MP1.hpp"
#include "CArchitectureQueue.hpp"
#include "CMFGame.hpp"
#include "CPlayMovie.hpp"
#include "CResFactory.hpp"
#include "CFrontEndUI.hpp"
#include "GameGlobalObjects.hpp"

namespace urde
{
namespace MP1
{

void CMainFlow::AdvanceGameState(CArchitectureQueue& queue)
{
    switch (x14_gameState)
    {
    case EClientFlowStates::FrontEnd:
        CMainFlow::SetGameState(EClientFlowStates::GameLoad, queue);
        break;
    case EClientFlowStates::Unspecified:
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
        if (g_main->GetGameplayResult() == EGameplayResult::None)
        {
            g_main->SetGameplayResult(EGameplayResult::Playing);
            break;
        }
        CResLoader& loader = g_ResFactory->GetLoader();
        while (!loader.AreAllPaksLoaded())
            loader.AsyncIdlePakLoading();
        g_main->LoadAudio();
        g_main->RegisterResourceTweaks();
        queue.Push(std::move(MakeMsg::CreateCreateIOWin(EArchMsgTarget::IOWinManager, 12, 11, new CFrontEndUI(queue))));
        break;
    }
    case EClientFlowStates::GameLoad:
    {
        queue.Push(std::move(MakeMsg::CreateCreateIOWin(EArchMsgTarget::IOWinManager, 10, 1000, new CMFGameLoader())));
        break;
    }
    case EClientFlowStates::MoviePlay:
    {
        switch (g_main->GetGameplayResult())
        {
        case EGameplayResult::Win:
            queue.Push(std::move(MakeMsg::CreateCreateIOWin(EArchMsgTarget::IOWinManager, 12, 11, new CPlayMovie(CPlayMovie::EWhichMovie::WinGame))));
            break;
        case EGameplayResult::Lose:
            queue.Push(std::move(MakeMsg::CreateCreateIOWin(EArchMsgTarget::IOWinManager, 12, 11, new CPlayMovie(CPlayMovie::EWhichMovie::LoseGame))));
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
