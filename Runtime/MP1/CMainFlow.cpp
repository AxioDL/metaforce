#include "CMainFlow.hpp"
#include "MP1.hpp"
#include "CArchitectureQueue.hpp"
#include "CMFGame.hpp"
#include "CPlayMovie.hpp"
#include "CResFactory.hpp"
#include "CFrontEndUI.hpp"
#include "GameGlobalObjects.hpp"
#include "Character/CCharLayoutInfo.hpp"
#include "CSaveGameScreen.hpp"
#include "CCredits.hpp"
#include "CPreFrontEnd.hpp"
#include "CStateSetterFlow.hpp"
#include "CNESEmulator.hpp"
#include "CQuitGameScreen.hpp"

namespace urde
{
namespace MP1
{

void CMainFlow::AdvanceGameState(CArchitectureQueue& queue)
{
    switch (x14_gameState)
    {
    case EClientFlowStates::Game:
        CMainFlow::SetGameState(EClientFlowStates::GameExit, queue);
        break;
    case EClientFlowStates::PreFrontEnd:
        CMainFlow::SetGameState(EClientFlowStates::FrontEnd, queue);
        break;
    case EClientFlowStates::FrontEnd:
        CMainFlow::SetGameState(EClientFlowStates::Game, queue);
        break;
    case EClientFlowStates::GameExit:
    {
        MP1::CMain* main = static_cast<MP1::CMain*>(g_Main);
        if (main->GetFlowState() != EFlowState::Zero &&
            main->GetFlowState() != EFlowState::Six)
            main->SetX30(true);
    }
    case EClientFlowStates::Unspecified:
        CMainFlow::SetGameState(EClientFlowStates::PreFrontEnd, queue);
        break;
    }
}

void CMainFlow::SetGameState(EClientFlowStates state, CArchitectureQueue& queue)
{
    x14_gameState = state;
    MP1::CMain* main = static_cast<MP1::CMain*>(g_Main);

    switch (state)
    {
    case EClientFlowStates::GameExit:
    {
        switch (main->GetFlowState())
        {
        case EFlowState::One:
        case EFlowState::Two:
        case EFlowState::Three:
            queue.Push(MakeMsg::CreateCreateIOWin(EArchMsgTarget::IOWinManager, 12, 11,
                                                  std::make_shared<CCredits>()));
            break;
        case EFlowState::Four:
            queue.Push(MakeMsg::CreateCreateIOWin(EArchMsgTarget::IOWinManager, 12, 11,
                                                  std::make_shared<CPlayMovie>(CPlayMovie::EWhichMovie::AfterCredits)));
            break;
        default: break;
        }
        break;
    }
    case EClientFlowStates::PreFrontEnd:
    {
        if (main->GetFlowState() == EFlowState::Zero)
            return;
        queue.Push(MakeMsg::CreateCreateIOWin(EArchMsgTarget::IOWinManager, 12, 11,
                                              std::make_shared<CPreFrontEnd>()));
        break;
    }
    case EClientFlowStates::FrontEnd:
    {
        std::shared_ptr<CIOWin> nextIOWin;
        switch (main->GetFlowState())
        {
        case EFlowState::Six:
            nextIOWin = std::make_shared<CStateSetterFlow>();
            break;
        case EFlowState::One:
        case EFlowState::Two:
        case EFlowState::Three:
        case EFlowState::Four:
        case EFlowState::Five:
            nextIOWin = std::make_shared<CFrontEndUI>();
            break;
        default: return;
        }

        queue.Push(MakeMsg::CreateCreateIOWin(EArchMsgTarget::IOWinManager, 12, 11, std::move(nextIOWin)));
        break;
    }
    case EClientFlowStates::Game:
    {
        g_GameState->GameOptions().EnsureSettings();
        main->SetFlowState(EFlowState::Five);
        queue.Push(MakeMsg::CreateCreateIOWin(EArchMsgTarget::IOWinManager, 10, 1000,
                                              std::make_shared<CMFGameLoader>()));
        break;
    }
    default: break;
    }
}


}
}
