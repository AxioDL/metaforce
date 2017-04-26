#ifndef __URDE_CINVENTORYSCREEN_HPP__
#define __URDE_CINVENTORYSCREEN_HPP__

#include "CInGameGuiManager.hpp"
#include "CPauseScreenBase.hpp"
#include "CSamusDoll.hpp"

namespace urde
{
class CDependencyGroup;

namespace MP1
{

class CInventoryScreen : public CPauseScreenBase
{
    std::unique_ptr<CSamusDoll> x19c_samusDoll;
public:
    CInventoryScreen(const CStateManager& mgr, const CGuiFrame& frame, const CStringTable& pauseStrg,
                     const CDependencyGroup& suitDgrp, const CDependencyGroup& ballDgrp);
};

}
}

#endif // __URDE_CINVENTORYSCREEN_HPP__
