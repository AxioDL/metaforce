#include "MetroidPrime/CMainFlow.hpp"

#include "MetroidPrime/CArchMsgParmInt32.hpp"
#include "MetroidPrime/CArchitectureMessage.hpp"
#include "MetroidPrime/CArchitectureQueue.hpp"
#include "MetroidPrime/CCredits.hpp"
#include "MetroidPrime/CFrontEndUI.hpp"
#include "MetroidPrime/CMFGameLoader.hpp"
#include "MetroidPrime/CPlayMovie.hpp"
#include "MetroidPrime/CPreFrontEnd.hpp"
#include "MetroidPrime/CStateSetterFlow.hpp"
#include "MetroidPrime/Decode.hpp"

#include "MetroidPrime/CMain.hpp"

#if defined(TARGET_PC)
#include "Metaforce/Runtime.hpp"
#endif

CMainFlow::CMainFlow() : CIOWin(rstl::string_l("MainFlow")), x14_gameState(kCFS_Unspecified) {}

CIOWin::EMessageReturn CMainFlow::OnMessage(const CArchitectureMessage& msg,
                                            CArchitectureQueue& queue) {
  switch (msg.GetType()) {
  case kAM_TimerTick:
    AdvanceGameState(queue);
    break;
  case kAM_SetGameState:
    CArchMsgParmInt32 state = MakeMsg::GetParmNewGameflowState(msg);
    SetGameState(static_cast< EClientFlowStates >(state.GetInt32()), queue);
    return CIOWin::kMR_Exit;
  }

  return CIOWin::kMR_Normal;
}

bool CMainFlow::GetIsContinueDraw() const { return false; }

void CMainFlow::Draw() const {}

void CMainFlow::AdvanceGameState(CArchitectureQueue& queue) {
  switch (x14_gameState) {
  case kCFS_Game:
    SetGameState(kCFS_GameExit, queue);
    break;
  case kCFS_PreFrontEnd:
    SetGameState(kCFS_FrontEnd, queue);
    break;
  case kCFS_FrontEnd:
    SetGameState(kCFS_Game, queue);
    break;
  case kCFS_GameExit: {
    if (gpMain->GetRestartMode() != CMain::kRM_None &&
        gpMain->GetRestartMode() != CMain::kRM_StateSetter) {
      gpMain->SetX30(true);
    }
    // Fall through.
  }
  case kCFS_Unspecified:
    SetGameState(kCFS_PreFrontEnd, queue);
    break;
  default:
    break;
  }
}

static inline bool IsCreditsMode(CMain::ERestartMode m) {
  return m >= CMain::kRM_WinBad && m <= CMain::kRM_LoseGame;
}

void CMainFlow::SetGameState(EClientFlowStates state, CArchitectureQueue& queue) {
  x14_gameState = state;

  switch (x14_gameState) {
  case kCFS_GameExit: {
    CMain::ERestartMode m = gpMain->GetRestartMode();
    if (IsCreditsMode(m)) {
      switch (m) {
      case CMain::kRM_WinBad:
      case CMain::kRM_WinGood:
      case CMain::kRM_WinBest: {
        CIOWin* credits = rs_new CCredits();
        queue.Push(MakeMsg::CreateCreateIOWin(kAMT_IOWinManager, kFrontEndUIMsgPriority,
                                              kFrontEndUIDrawPriority, credits));
        break;
      }
      default: {
        CIOWin* movie = rs_new CPlayMovie(CPlayMovie::kWM_LoseGame);
        queue.Push(MakeMsg::CreateCreateIOWin(kAMT_IOWinManager, kFrontEndUIMsgPriority,
                                              kFrontEndUIDrawPriority, movie));
        break;
      }
      }
      break;
    }
    break;
  }
  case kCFS_PreFrontEnd: {
    if (gpMain->GetRestartMode() == CMain::kRM_None) {
      break;
    }

    CIOWin* preFrontEnd = rs_new CPreFrontEnd();
    queue.Push(MakeMsg::CreateCreateIOWin(kAMT_IOWinManager, kFrontEndUIMsgPriority,
                                          kFrontEndUIDrawPriority, preFrontEnd));
    break;
  }
  case kCFS_FrontEnd: {
    if (gpMain->GetRestartMode() == CMain::kRM_None) {
      break;
    }
    CIOWin* ioWin;
#if defined(TARGET_PC)
    if (gpMain->GetRestartMode() == CMain::kRM_StateSetter || metaforce::HasStartupRequest()) {
#else
    if (gpMain->GetRestartMode() == CMain::kRM_StateSetter) {
#endif
      ioWin = rs_new CStateSetterFlow();
    } else {
      ioWin = rs_new CFrontEndUI();
    }
    queue.Push(MakeMsg::CreateCreateIOWin(kAMT_IOWinManager, kFrontEndUIMsgPriority,
                                          kFrontEndUIDrawPriority, ioWin));
    break;
  }
  case kCFS_Game: {
    gpGameState->GameOptions().EnsureOptions();
    CIOWin* const gameFlow = rs_new CMFGameLoader();
    gpMain->SetRestartMode(CMain::kRM_Default);
    queue.Push(MakeMsg::CreateCreateIOWin(kAMT_IOWinManager, kMFGameMsgPriority,
                                          kMFGameDrawPriority, gameFlow));
    break;
  }
  }
}

CMainFlow::~CMainFlow() {}
