#ifndef __URDE_CMAPWORLD_HPP__
#define __URDE_CMAPWORLD_HPP__

namespace urde
{
class IWorld;

class CMapWorld
{
public:
    void SetWhichMapAreasLoaded(const IWorld&, int, int);
    bool IsMapAreasStreaming() const;
};

}

#endif // __URDE_CMAPWORLD_HPP__
