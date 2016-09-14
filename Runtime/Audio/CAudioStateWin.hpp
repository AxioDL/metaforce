#ifndef __URDE_CAUDIOSTATEWIN_HPP__
#define __URDE_CAUDIOSTATEWIN_HPP__

#include "../CIOWin.hpp"

namespace urde
{
class CAudioStateWin : public CIOWin
{
public:
    CAudioStateWin() : CIOWin("CAudioStateWin") {}
    CIOWin::EMessageReturn OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue);
};

}

#endif // __URDE_CAUDIOSTATEWIN_HPP__
