#ifndef __RETRO_CMAINFLOW_HPP__
#define __RETRO_CMAINFLOW_HPP__

#include "CIOWin.hpp"

namespace Retro
{
class CArchitectureMessage;
class CArchitectureQueue;

enum EClientFlowStates
{
    StateGameLoad = 13,
};

class CMainFlow : public CIOWin
{
public:
    CMainFlow() : CIOWin("CMainFlow") {}
    void AdvanceGameState(CArchitectureQueue& queue);
    void SetGameState(EClientFlowStates state, CArchitectureQueue& queue);
    bool OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue);
    bool GetIsContinueDraw() const {return false;}
    void Draw() const {}
};

}

#endif // __RETRO_CMAINFLOW_HPP__
