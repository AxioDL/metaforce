#ifndef __URDE_CMAPWORLDINFO_HPP__
#define __URDE_CMAPWORLDINFO_HPP__

#include "RetroTypes.hpp"

namespace urde
{
class CMapWorldInfo
{
    std::map<TEditorId, bool> x14_;
public:
    void SetDoorVisited(TEditorId eid, bool val);
};
}

#endif // __URDE_CMAPWORLDINFO_HPP__
