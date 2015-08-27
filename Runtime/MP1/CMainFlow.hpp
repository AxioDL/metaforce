#ifndef __RETRO_CMAINFLOW_HPP__
#define __RETRO_CMAINFLOW_HPP__

#include "CMainFlowBase.hpp"

namespace Retro
{
class CArchitectureMessage;
class CArchitectureQueue;

namespace MP1
{

class CMainFlow : public CMainFlowBase
{
public:
    CMainFlow() : CMainFlowBase("CMainFlow") {}
    void SetGameState(EClientFlowStates state, CArchitectureQueue& queue);
    bool GetIsContinueDraw() const {return false;}
    void Draw() const {}
};

}

}

#endif // __RETRO_CMAINFLOW_HPP__
