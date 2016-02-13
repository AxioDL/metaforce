#include "CMFGame.hpp"
#include "CArchitectureQueue.hpp"
#include "GameGlobalObjects.hpp"
#include "CGameState.hpp"

namespace pshag
{
namespace MP1
{

CIOWin::EMessageReturn CMFGameLoader::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue)
{
    switch (msg.GetType())
    {
    case EArchMsgType::TimerTick:
    {
        const CArchMsgParmReal32& tick = MakeMsg::GetParmTimerTick(msg);
        g_GameState->WorldTransitionManager();
    }
    default: break;
    }
    return EMessageReturn::Exit;
}

void CMFGameLoader::Draw() const
{
    g_GameState->WorldTransitionManager().Draw();
}

}
}
