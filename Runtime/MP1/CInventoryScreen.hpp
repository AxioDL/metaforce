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
    CInventoryScreen(const CStateManager& mgr, CGuiFrame& frame, const CStringTable& pauseStrg,
                     const CDependencyGroup& suitDgrp, const CDependencyGroup& ballDgrp);

    bool VReady() const;
    void VActivate() const;
    u32 GetRightTableCount() const;
};

}
}

#endif // __URDE_CINVENTORYSCREEN_HPP__
