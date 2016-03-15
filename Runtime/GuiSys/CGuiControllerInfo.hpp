#ifndef __URDE_CGUICONTROLLERINFO_HPP__
#define __URDE_CGUICONTROLLERINFO_HPP__

#include "RetroTypes.hpp"

namespace urde
{

struct CGuiControllerInfo
{
public:
    struct CGuiControllerStateInfo
    {
        char cIdx = 0;
        char lx = 0, ly = 0, rx = 0, ry = 0;
    };
    u32 x0_ = 0;
    CGuiControllerStateInfo x4_stateInfo;
};

}

#endif // __URDE_CGUICONTROLLERINFO_HPP__
