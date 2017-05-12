#ifndef __URDE_CLOGBOOKSCREEN_HPP__
#define __URDE_CLOGBOOKSCREEN_HPP__

#include "CInGameGuiManager.hpp"
#include "CPauseScreenBase.hpp"

namespace urde
{
namespace MP1
{

class CLogBookScreen : public CPauseScreenBase
{
public:
    CLogBookScreen(const CStateManager& mgr, CGuiFrame& frame, const CStringTable& pauseStrg);

    bool VReady() const;
    void VActivate();
    u32 GetRightTableCount() const;
};

}
}

#endif // __URDE_CLOGBOOKSCREEN_HPP__
