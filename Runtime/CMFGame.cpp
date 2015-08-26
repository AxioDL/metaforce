#include "CMFGame.hpp"
#include "CArchitectureQueue.hpp"
#include "GameGlobalObjects.hpp"
#include "CGameState.hpp"

namespace Retro
{

CIOWin::EMessageReturn CMFGameLoader::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue)
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
    return MsgRetExit;
}

void CMFGameLoader::Draw() const
{
    g_GameState->WorldTransitionManager().Draw();
}

}
