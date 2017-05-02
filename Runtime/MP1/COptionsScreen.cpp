#include "COptionsScreen.hpp"

namespace urde
{
namespace MP1
{

COptionsScreen::COptionsScreen(const CStateManager& mgr, CGuiFrame& frame,
                               const CStringTable& pauseStrg)
: CPauseScreenBase(mgr, frame, pauseStrg)
{

}

bool COptionsScreen::VReady() const
{
    return true;
}

void COptionsScreen::VActivate() const
{

}

u32 COptionsScreen::GetRightTableCount() const
{
    return 0;
}

}
}
