#include "CAudioStateWin.hpp"
#include "CSfxManager.hpp"
#include "CArchitectureMessage.hpp"
#include "CArchitectureQueue.hpp"
#include "GameGlobalObjects.hpp"
#include "CGameState.hpp"
#include "IMain.hpp"

namespace urde
{

CIOWin::EMessageReturn CAudioStateWin::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue)
{
    const EArchMsgType msgType = msg.GetType();
    if (msgType == EArchMsgType::SetGameState)
    {
        CSfxManager::KillAll(CSfxManager::ESfxChannels::One);
        CSfxManager::TurnOnChannel(CSfxManager::ESfxChannels::One);
    }
    else if (msgType == EArchMsgType::QuitGameplay)
    {
        if (g_GameState->GetWorldTransitionManager()->GetTransType() == CWorldTransManager::ETransType::Disabled ||
            g_Main->GetFlowState() != IMain::EFlowState::Zero)
        {
            CSfxManager::SetChannel(CSfxManager::ESfxChannels::Zero);
            CSfxManager::KillAll(CSfxManager::ESfxChannels::One);
        }
    }
    return EMessageReturn::Normal;
}

}
