#ifndef __PSHAG_CAUDIOSTATEWIN_HPP__
#define __PSHAG_CAUDIOSTATEWIN_HPP__

#include "../CIOWin.hpp"

namespace pshag
{

class CAudioStateWin : public CIOWin
{
public:
    CAudioStateWin() : CIOWin("CAudioStateWin") {}
    CIOWin::EMessageReturn OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue)
    {
        return EMessageReturn::Normal;
    }
};

}

#endif // __PSHAG_CAUDIOSTATEWIN_HPP__
