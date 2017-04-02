#ifndef __URDE_CHUDVISORBEAMMENU_HPP__
#define __URDE_CHUDVISORBEAMMENU_HPP__

#include "RetroTypes.hpp"

namespace urde
{
class CGuiFrame;

class CHudVisorBeamMenu
{
public:
    enum class EHudVisorBeamMenu
    {
        Visor,
        Beam
    };
private:
public:
    CHudVisorBeamMenu(CGuiFrame* baseHud, EHudVisorBeamMenu type, rstl::prereserved_vector<bool, 4> enables);
};

}

#endif // __URDE_CHUDVISORBEAMMENU_HPP__
