#ifndef __URDE_CGUILOGICALEVENTTRIGGER_HPP__
#define __URDE_CGUILOGICALEVENTTRIGGER_HPP__

#include "CGuiPhysicalMsg.hpp"

namespace urde
{

class CGuiLogicalEventTrigger
{
    CGuiPhysicalMsg x0_msg;
    int x10_val;
    bool x14_flag;
public:
    CGuiLogicalEventTrigger(const CGuiPhysicalMsg& msg, int val, bool flag)
    : x0_msg(msg), x10_val(val), x14_flag(flag) {}
};

}

#endif // __URDE_CGUILOGICALEVENTTRIGGER_HPP__
