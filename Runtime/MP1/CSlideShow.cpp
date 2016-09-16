#include "CSlideShow.hpp"

namespace urde
{

CSlideShow::CSlideShow()
: CIOWin("SlideShow")
{
}

CIOWin::EMessageReturn CSlideShow::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue)
{
    return EMessageReturn::Exit;
}

void CSlideShow::Draw() const
{
}

}
