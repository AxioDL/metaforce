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
        queue.PushMessage(std::move(MakeMsg::CreateCreateIOWin(TargetMainFlow, 10, 1000, new CMFGameLoader())));
        break;
    default: break;
    }
}
bool CMainFlow::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue)
{
    switch (msg.GetType())
    {
    case MsgTimerTick:
        AdvanceGameState(queue);
        break;
    case MsgSetGameState:
    {
        CArchMsgParmInt32 state = MakeMsg::GetParmNewGameflowState(msg);
        SetGameState(EClientFlowStates(state.m_parm), queue);
        return true;
    }
    default: break;
    }
    return false;
}

}
