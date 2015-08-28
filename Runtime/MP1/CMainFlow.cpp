#include "CMainFlow.hpp"
#include "CArchitectureQueue.hpp"
#include "CMFGame.hpp"
#include "CMain.hpp"
#include "CPlayMovie.hpp"
#include "CResFactory.hpp"
#include "CFrontEndUI.hpp"
#include "GameGlobalObjects.hpp"

namespace Retro
{
namespace MP1
{

void CMainFlow::AdvanceGameState(CArchitectureQueue& queue)
{
    switch (x14_gameState)
    {
    case ClientFlowFrontEnd:
        CMainFlow::SetGameState(ClientFlowGameLoad, queue);
        break;
    case ClientFlowUnspecified:
    case ClientFlowGameLoad:
        CMainFlow::SetGameState(ClientFlowMoviePlay, queue);
        break;
    case ClientFlowMoviePlay:
        CMainFlow::SetGameState(ClientFlowFrontEnd, queue);
        break;
    }
}

void CMainFlow::SetGameState(EClientFlowStates state, CArchitectureQueue& queue)
{
    switch (state)
    {
    case ClientFlowFrontEnd:
    {
        if (g_main->GetGameplayResult() == GameplayResultNone)
        {
            g_main->SetGameplayResult(GameplayResultPlaying);
            break;
        }
        CResLoader& loader = g_ResFactory->GetLoader();
        while (!loader.AreAllPaksLoaded())
            loader.AsyncIdlePakLoading();
        g_main->LoadAudio();
        g_main->RegisterResourceTweaks();
        queue.Push(std::move(MakeMsg::CreateCreateIOWin(TargetIOWinManager, 12, 11, new CFrontEndUI(queue))));
        break;
    }
    case ClientFlowGameLoad:
    {
        queue.Push(std::move(MakeMsg::CreateCreateIOWin(TargetIOWinManager, 10, 1000, new CMFGameLoader())));
        break;
    }
    case ClientFlowMoviePlay:
    {
        switch (g_main->GetGameplayResult())
        {
        case GameplayResultWin:
            queue.Push(std::move(MakeMsg::CreateCreateIOWin(TargetIOWinManager, 12, 11, new CPlayMovie(CPlayMovie::MovieWinGame))));
            break;
        case GameplayResultLose:
            queue.Push(std::move(MakeMsg::CreateCreateIOWin(TargetIOWinManager, 12, 11, new CPlayMovie(CPlayMovie::MovieLoseGame))));
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
