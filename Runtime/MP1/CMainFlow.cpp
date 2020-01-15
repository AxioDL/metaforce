#include "Runtime/MP1/CMainFlow.hpp"

#include "NESEmulator/CNESEmulator.hpp"

#include "Runtime/CArchitectureQueue.hpp"
#include "Runtime/CResFactory.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Character/CCharLayoutInfo.hpp"
#include "Runtime/MP1/CCredits.hpp"
#include "Runtime/MP1/CFrontEndUI.hpp"
#include "Runtime/MP1/CMFGame.hpp"
#include "Runtime/MP1/CPlayMovie.hpp"
#include "Runtime/MP1/CPreFrontEnd.hpp"
#include "Runtime/MP1/CQuitGameScreen.hpp"
#include "Runtime/MP1/CSaveGameScreen.hpp"
#include "Runtime/MP1/CStateSetterFlow.hpp"
#include "Runtime/MP1/MP1.hpp"

namespace urde::MP1 {

void CMainFlow::AdvanceGameState(CArchitectureQueue& queue) {
  switch (x14_gameState) {
  case EClientFlowStates::Game:
    SetGameState(EClientFlowStates::GameExit, queue);
    break;
  case EClientFlowStates::PreFrontEnd:
    SetGameState(EClientFlowStates::FrontEnd, queue);
    break;
  case EClientFlowStates::FrontEnd:
    SetGameState(EClientFlowStates::Game, queue);
    break;
  case EClientFlowStates::GameExit: {
    MP1::CMain* main = static_cast<MP1::CMain*>(g_Main);
    if (main->GetFlowState() != EFlowState::None && main->GetFlowState() != EFlowState::StateSetter)
      main->SetX30(true);
    [[fallthrough]];
  }
  case EClientFlowStates::Unspecified:
    SetGameState(EClientFlowStates::PreFrontEnd, queue);
    break;
  }
}

void CMainFlow::SetGameState(EClientFlowStates state, CArchitectureQueue& queue) {
  x14_gameState = state;
  MP1::CMain* main = static_cast<MP1::CMain*>(g_Main);

  switch (state) {
  case EClientFlowStates::GameExit: {
    switch (main->GetFlowState()) {
    case EFlowState::WinBad:
    case EFlowState::WinGood:
    case EFlowState::WinBest:
      queue.Push(MakeMsg::CreateCreateIOWin(EArchMsgTarget::IOWinManager, 12, 11, std::make_shared<CCredits>()));
      break;
    case EFlowState::LoseGame:
      queue.Push(MakeMsg::CreateCreateIOWin(EArchMsgTarget::IOWinManager, 12, 11,
                                            std::make_shared<CPlayMovie>(CPlayMovie::EWhichMovie::LoseGame)));
      break;
    default:
      break;
    }
    break;
  }
  case EClientFlowStates::PreFrontEnd: {
    if (main->GetFlowState() == EFlowState::None)
      return;
    queue.Push(MakeMsg::CreateCreateIOWin(EArchMsgTarget::IOWinManager, 12, 11, std::make_shared<CPreFrontEnd>()));
    break;
  }
  case EClientFlowStates::FrontEnd: {
    std::shared_ptr<CIOWin> nextIOWin;
    switch (main->GetFlowState()) {
    case EFlowState::StateSetter:
      nextIOWin = std::make_shared<CStateSetterFlow>();
      break;
    case EFlowState::WinBad:
    case EFlowState::WinGood:
    case EFlowState::WinBest:
    case EFlowState::LoseGame:
    case EFlowState::Default:
      nextIOWin = std::make_shared<CFrontEndUI>();
      break;
    default:
      return;
    }

    queue.Push(MakeMsg::CreateCreateIOWin(EArchMsgTarget::IOWinManager, 12, 11, std::move(nextIOWin)));
    break;
  }
  case EClientFlowStates::Game: {
    g_GameState->GameOptions().EnsureSettings();
    std::shared_ptr<CMFGameLoader> gameLoader = std::make_shared<CMFGameLoader>();
    main->SetFlowState(EFlowState::Default);
    queue.Push(MakeMsg::CreateCreateIOWin(EArchMsgTarget::IOWinManager, 10, 1000, gameLoader));
    break;
  }
  default:
    break;
  }
}

} // namespace urde::MP1
