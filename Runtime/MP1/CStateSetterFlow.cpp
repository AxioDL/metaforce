#include "CStateSetterFlow.hpp"

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
    return EMessageReturn::Normal;
}

}
}
