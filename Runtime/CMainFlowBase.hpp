#ifndef __PSHAG_CMAINFLOWBASE_HPP__
#define __PSHAG_CMAINFLOWBASE_HPP__

#include "CIOWin.hpp"

namespace pshag
{

enum class EClientFlowStates
{
    Unspecified = -1,
    FrontEnd = 7,
    GameLoad = 13,
    MoviePlay = 14
};

class CMainFlowBase : public CIOWin
{
protected:
    EClientFlowStates x14_gameState;
public:
    CMainFlowBase(const char* name) : CIOWin(name) {}
    EMessageReturn OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue);
    virtual void AdvanceGameState(CArchitectureQueue& queue)=0;
    virtual void SetGameState(EClientFlowStates state, CArchitectureQueue& queue)=0;
};

}

#endif // __PSHAG_CMAINFLOWBASE_HPP__
