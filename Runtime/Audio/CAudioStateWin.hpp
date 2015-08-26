#ifndef __RETRO_CAUDIOSTATEWIN_HPP__
#define __RETRO_CAUDIOSTATEWIN_HPP__

#include "../CIOWin.hpp"

namespace Retro
{

class CAudioStateWin : public CIOWin
{
public:
    CAudioStateWin() : CIOWin("CAudioStateWin") {}
    virtual CIOWin::EMessageReturn OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue)
    {
    }
};

}

#endif // __RETRO_CAUDIOSTATEWIN_HPP__
