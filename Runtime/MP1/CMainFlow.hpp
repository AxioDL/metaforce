#ifndef __PSHAG_CMAINFLOW_HPP__
#define __PSHAG_CMAINFLOW_HPP__

#include "CMainFlowBase.hpp"

namespace urde
{
class CArchitectureMessage;
class CArchitectureQueue;

namespace MP1
{

class CMainFlow : public CMainFlowBase
{
public:
    CMainFlow() : CMainFlowBase("CMainFlow") {}
    void AdvanceGameState(CArchitectureQueue& queue);
    void SetGameState(EClientFlowStates state, CArchitectureQueue& queue);
    bool GetIsContinueDraw() const {return false;}
    void Draw() const {}
};

}

}

#endif // __PSHAG_CMAINFLOW_HPP__
