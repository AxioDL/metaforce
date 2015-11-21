#include "CMainFlowBase.hpp"
#include "CArchitectureMessage.hpp"

namespace Retro
{

CIOWin::EMessageReturn CMainFlowBase::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue)
{
    switch (msg.GetType())
    {
    case EArchMsgType::TimerTick:
        AdvanceGameState(queue);
        break;
    case EArchMsgType::SetGameState:
    {
        const CArchMsgParmInt32& state = MakeMsg::GetParmNewGameflowState(msg);
        x14_gameState = EClientFlowStates(state.x4_parm);
        SetGameState(x14_gameState, queue);
        return EMessageReturn::Exit;
    }
    default: break;
    }
    return EMessageReturn::Normal;
}

}
