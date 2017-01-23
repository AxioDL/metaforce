#include "CAudioStateWin.hpp"
#include "Audio/CSfxManager.hpp"
#include "CArchitectureMessage.hpp"
#include "CArchitectureQueue.hpp"
#include "GameGlobalObjects.hpp"
#include "CGameState.hpp"
#include "MP1.hpp"

namespace urde
{
namespace MP1
{

CIOWin::EMessageReturn CAudioStateWin::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue)
{
    CMain* m = static_cast<CMain*>(g_Main);

    const EArchMsgType msgType = msg.GetType();
    if (msgType == EArchMsgType::SetGameState)
    {
        CSfxManager::KillAll(CSfxManager::ESfxChannels::Game);
        CSfxManager::TurnOnChannel(CSfxManager::ESfxChannels::Game);
    }
    else if (msgType == EArchMsgType::QuitGameplay)
    {
        if (g_GameState->GetWorldTransitionManager()->GetTransType() == CWorldTransManager::ETransType::Disabled ||
            m->GetFlowState() != EFlowState::Zero)
        {
            CSfxManager::SetChannel(CSfxManager::ESfxChannels::Default);
            CSfxManager::KillAll(CSfxManager::ESfxChannels::Game);
        }
    }
    return EMessageReturn::Normal;
}

}
}
