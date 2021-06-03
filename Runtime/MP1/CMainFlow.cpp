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

namespace metaforce::MP1 {

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
    if (main->GetFlowState() != EClientFlowStates::None && main->GetFlowState() != EClientFlowStates::StateSetter)
      main->SetX30(true);
    [[fallthrough]];
  }
  case EClientFlowStates::Unspecified:
    SetGameState(EClientFlowStates::PreFrontEnd, queue);
    break;
  default:
    break;
  }
}

void CMainFlow::SetGameState(EClientFlowStates state, CArchitectureQueue& queue) {
  x14_gameState = state;
  MP1::CMain* main = static_cast<MP1::CMain*>(g_Main);

  switch (state) {
  case EClientFlowStates::GameExit: {
    switch (main->GetFlowState()) {
    case EClientFlowStates::WinBad:
    case EClientFlowStates::WinGood:
    case EClientFlowStates::WinBest:
      queue.Push(MakeMsg::CreateCreateIOWin(EArchMsgTarget::IOWinManager, 12, 11, std::make_shared<CCredits>()));
      break;
    case EClientFlowStates::LoseGame:
      queue.Push(MakeMsg::CreateCreateIOWin(EArchMsgTarget::IOWinManager, 12, 11,
                                            std::make_shared<CPlayMovie>(CPlayMovie::EWhichMovie::LoseGame)));
      break;
    default:
      break;
    }
    break;
  }
  case EClientFlowStates::PreFrontEnd: {
    if (main->GetFlowState() == EClientFlowStates::None)
      return;
    queue.Push(MakeMsg::CreateCreateIOWin(EArchMsgTarget::IOWinManager, 12, 11, std::make_shared<CPreFrontEnd>()));
    break;
  }
  case EClientFlowStates::FrontEnd: {
    std::shared_ptr<CIOWin> nextIOWin;
    switch (main->GetFlowState()) {
    case EClientFlowStates::StateSetter:
      nextIOWin = std::make_shared<CStateSetterFlow>();
      break;
    case EClientFlowStates::WinBad:
    case EClientFlowStates::WinGood:
    case EClientFlowStates::WinBest:
    case EClientFlowStates::LoseGame:
    case EClientFlowStates::Default:
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
    auto gameLoader = std::make_shared<CMFGameLoader>();
    main->SetFlowState(EClientFlowStates::Default);
    queue.Push(MakeMsg::CreateCreateIOWin(EArchMsgTarget::IOWinManager, 10, 1000, std::move(gameLoader)));
    break;
  }
  default:
    break;
  }
}

} // namespace metaforce::MP1
