#include "CLogBookScreen.hpp"

namespace urde
{
namespace MP1
{

CLogBookScreen::CLogBookScreen(const CStateManager& mgr, CGuiFrame& frame,
                               const CStringTable& pauseStrg)
: CPauseScreenBase(mgr, frame, pauseStrg)
{

}

bool CLogBookScreen::VReady() const
{
    return true;
}

void CLogBookScreen::VActivate()
{

}

u32 CLogBookScreen::GetRightTableCount() const
{
    return 0;
}

}
}
