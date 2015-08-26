#include "CMainFlow.hpp"
#include "CArchitectureQueue.hpp"
#include "CMFGame.hpp"

namespace Retro
{

void CMainFlow::AdvanceGameState(CArchitectureQueue& queue)
{
}
void CMainFlow::SetGameState(EClientFlowStates state, CArchitectureQueue& queue)
{
    switch (state)
    {
    case StateGameLoad:
        queue.Push(std::move(MakeMsg::CreateCreateIOWin(TargetIOWinManager, 10, 1000, new CMFGameLoader())));
        break;
    default: break;
    }
}
CIOWin::EMessageReturn CMainFlow::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue)
{
    switch (msg.GetType())
    {
    case MsgTimerTick:
        AdvanceGameState(queue);
        break;
    case MsgSetGameState:
    {
        CArchMsgParmInt32 state = MakeMsg::GetParmNewGameflowState(msg);
        SetGameState(EClientFlowStates(state.x4_parm), queue);
        return MsgRetExit;
    }
    default: break;
    }
    return MsgRetNormal;
}

}
