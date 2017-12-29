#ifndef __URDE_MP1_CAUDIOSTATEWIN_HPP__
#define __URDE_MP1_CAUDIOSTATEWIN_HPP__

#include "../CIOWin.hpp"

namespace urde::MP1
{
class CAudioStateWin : public CIOWin
{
public:
    CAudioStateWin() : CIOWin("CAudioStateWin") {}
    CIOWin::EMessageReturn OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue);
};

}

#endif // __URDE_MP1_CAUDIOSTATEWIN_HPP__
