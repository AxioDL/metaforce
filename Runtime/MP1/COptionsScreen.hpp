#ifndef __URDE_COPTIONSSCREEN_HPP__
#define __URDE_COPTIONSSCREEN_HPP__

#include "CInGameGuiManager.hpp"
#include "CPauseScreenBase.hpp"

namespace urde
{
namespace MP1
{

class COptionsScreen : public CPauseScreenBase
{
public:
    COptionsScreen(const CStateManager& mgr, CGuiFrame& frame, const CStringTable& pauseStrg);

    bool VReady() const;
    void VActivate() const;
    u32 GetRightTableCount() const;
};

}
}

#endif // __URDE_COPTIONSSCREEN_HPP__
