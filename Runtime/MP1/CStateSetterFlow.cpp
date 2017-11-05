#include "CStateSetterFlow.hpp"
#include "CArchitectureMessage.hpp"
#include "GameGlobalObjects.hpp"
namespace urde
{
namespace MP1
{

CStateSetterFlow::CStateSetterFlow()
: CIOWin("")
{

}

CIOWin::EMessageReturn CStateSetterFlow::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue)
{
    if (msg.GetType() == EArchMsgType::TimerTick)
    {
        //g_Main->sub80004590();
        return EMessageReturn::RemoveIOWinAndExit;
    }
    return EMessageReturn::Exit;
}

}
}
