#include "CPreFrontEnd.hpp"

namespace urde
{
namespace MP1
{

CPreFrontEnd::CPreFrontEnd()
: CIOWin("Pre front-end window")
{

}

CIOWin::EMessageReturn CPreFrontEnd::OnMessage(const CArchitectureMessage&, CArchitectureQueue&)
{
    return EMessageReturn::Exit;
}

}
}
