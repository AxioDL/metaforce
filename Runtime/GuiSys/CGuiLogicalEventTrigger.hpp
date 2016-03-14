#ifndef __URDE_CGUILOGICALEVENTTRIGGER_HPP__
#define __URDE_CGUILOGICALEVENTTRIGGER_HPP__

#include "CGuiPhysicalMsg.hpp"

namespace urde
{

class CGuiLogicalEventTrigger
{
    CGuiPhysicalMsg x0_msg;
    int x10_triggerId;
    bool x14_flag;
public:
    CGuiLogicalEventTrigger(const CGuiPhysicalMsg& msg, int val, bool flag)
    : x0_msg(msg), x10_triggerId(val), x14_flag(flag) {}
    const CGuiPhysicalMsg& GetPhysicalMsg() const {return x0_msg;}
    int GetTriggerId() const {return x10_triggerId;}
};

}

#endif // __URDE_CGUILOGICALEVENTTRIGGER_HPP__
