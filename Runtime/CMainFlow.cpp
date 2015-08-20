#include "CMainFlow.hpp"
#include "GameGlobalObjects.hpp"
#include "CGameState.hpp"

namespace Retro
{

bool CMFGameLoader::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue)
{
    switch (msg.GetType())
    {
    case MsgTimerTick:
    {
        const CArchMsgParmReal32& tick = MakeMsg::GetParmTimerTick(msg);
        g_GameState->WorldTransitionManager();
    }
    default: break;
    }
    return true;
}

void CMFGameLoader::Draw() const
{
    g_GameState->WorldTransitionManager().Draw();
}

}
