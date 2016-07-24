#ifndef __URDE_CMAPWORLD_HPP__
#define __URDE_CMAPWORLD_HPP__

#include "RetroTypes.hpp"

namespace urde
{
class IWorld;

class CMapWorld
{
public:
    void SetWhichMapAreasLoaded(const IWorld&, int start, int count);
    bool IsMapAreasStreaming() const;
};

}

#endif // __URDE_CMAPWORLD_HPP__
