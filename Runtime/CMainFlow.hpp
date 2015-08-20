#ifndef __RETRO_CMAINFLOW_HPP__
#define __RETRO_CMAINFLOW_HPP__

#include "CIOWin.hpp"
#include "CMFGame.hpp"

namespace Retro
{

enum EClientFlowStates
{
    StateGameLoad = 13,
};

class CMainFlow : public CIOWin
{
public:
    CMainFlow() : CIOWin("CMainFlow") {}
    void AdvanceGameState(CArchitectureQueue& queue)
    {
    }
    void SetGameState(EClientFlowStates state, CArchitectureQueue& queue)
    {
        switch (state)
        {
        case StateGameLoad:
            queue.PushMessage(std::move(MakeMsg::CreateCreateIOWin(TargetMainFlow, 10, 1000, new CMFGameLoader())));
            break;
        default: break;
        }
    }
    bool OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue)
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
    bool GetIsContinueDraw() const {return false;}
    void Draw() const {}
};

}

#endif // __RETRO_CMAINFLOW_HPP__
