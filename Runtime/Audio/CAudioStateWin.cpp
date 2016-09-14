#include "CAudioStateWin.hpp"
#include "CSfxManager.hpp"
#include "CArchitectureMessage.hpp"
#include "CArchitectureQueue.hpp"
#include "GameGlobalObjects.hpp"
#include "CGameState.hpp"

namespace urde
{

CIOWin::EMessageReturn CAudioStateWin::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue)
{
#if 0
    const EArchMsgType msgType = msg.GetType();
    if (msgType == EArchMsgType::SetGameState)
    {
        CSfxManager::KillAll(CSfxManager::ESfxChannels::One);
        CSfxManager::TurnOnChannel(CSfxManager::ESfxChannels::One);
    }
    else if (msgType == EArchMsgType::QuitGameplay)
    {
        if (g_GameState->GetWorldTransitionManager()->GetTransitionType() == CWorldTransManager::ETransType::Disabled ||
            g_Main->x12c_ != 0)
        {
            CSfxManager::SetChannel(CSfxManager::ESfxChannels::Zero);
            CSfxManager::KillAll(CSfxManager::ESfxChannels::One);
        }
    }
#endif
    return EMessageReturn::Normal;
}

}
