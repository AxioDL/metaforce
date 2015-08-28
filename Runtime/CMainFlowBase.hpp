#ifndef __RETRO_CMAINFLOWBASE_HPP__
#define __RETRO_CMAINFLOWBASE_HPP__

#include "CIOWin.hpp"

namespace Retro
{

enum EClientFlowStates
{
    ClientFlowUnspecified = -1,
    ClientFlowFrontEnd = 7,
    ClientFlowGameLoad = 13,
    ClientFlowMoviePlay = 14
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

#endif // __RETRO_CMAINFLOWBASE_HPP__
