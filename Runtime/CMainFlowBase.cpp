#include "CMainFlowBase.hpp"
#include "CArchitectureMessage.hpp"

namespace Retro
{

CIOWin::EMessageReturn CMainFlowBase::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue)
{
    switch (msg.GetType())
    {
    case MsgTimerTick:
        AdvanceGameState(queue);
        break;
    case MsgSetGameState:
    {
        const CArchMsgParmInt32& state = MakeMsg::GetParmNewGameflowState(msg);
        x14_gameState = EClientFlowStates(state.x4_parm);
        SetGameState(x14_gameState, queue);
        return MsgRetExit;
    }
    default: break;
    }
    return MsgRetNormal;
}

}
