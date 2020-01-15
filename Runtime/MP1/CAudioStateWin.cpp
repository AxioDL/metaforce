#include "CAudioStateWin.hpp"

#include "Runtime/CArchitectureMessage.hpp"
#include "Runtime/CArchitectureQueue.hpp"
#include "Runtime/CGameState.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Audio/CSfxManager.hpp"
#include "Runtime/MP1/MP1.hpp"

namespace urde::MP1 {

CIOWin::EMessageReturn CAudioStateWin::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue) {
  CMain* m = static_cast<CMain*>(g_Main);

  const EArchMsgType msgType = msg.GetType();
  if (msgType == EArchMsgType::SetGameState) {
    CSfxManager::KillAll(CSfxManager::ESfxChannels::Game);
    CSfxManager::TurnOnChannel(CSfxManager::ESfxChannels::Game);
  } else if (msgType == EArchMsgType::QuitGameplay) {
    if (g_GameState->GetWorldTransitionManager()->GetTransType() == CWorldTransManager::ETransType::Disabled ||
        m->GetFlowState() != EFlowState::None) {
      CSfxManager::SetChannel(CSfxManager::ESfxChannels::Default);
      CSfxManager::KillAll(CSfxManager::ESfxChannels::Game);
    }
  }
  return EMessageReturn::Normal;
}

} // namespace urde::MP1
