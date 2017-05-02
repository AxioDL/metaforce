#include "CInventoryScreen.hpp"

namespace urde
{
namespace MP1
{

CInventoryScreen::CInventoryScreen(const CStateManager& mgr, CGuiFrame& frame, const CStringTable& pauseStrg,
                                   const CDependencyGroup& suitDgrp, const CDependencyGroup& ballDgrp)
: CPauseScreenBase(mgr, frame, pauseStrg)
{

}

bool CInventoryScreen::VReady() const
{
    return true;
}

void CInventoryScreen::VActivate() const
{

}

u32 CInventoryScreen::GetRightTableCount() const
{
    return 0;
}

}
}
