#ifndef __URDE_ITWEAKGUI_HPP__
#define __URDE_ITWEAKGUI_HPP__

#include "RetroTypes.hpp"
#include "ITweak.hpp"

namespace urde
{
class ITweakGui : public ITweak
{
public:
    virtual ~ITweakGui() {}

    virtual float GetScanSpeed(s32) const=0;
};
}

#endif // __URDE_ITWEAKGUI_HPP__
