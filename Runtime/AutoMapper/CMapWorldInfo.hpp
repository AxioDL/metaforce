#ifndef __URDE_CMAPWORLDINFO_HPP__
#define __URDE_CMAPWORLDINFO_HPP__

#include "RetroTypes.hpp"

namespace urde
{
class CSaveWorld;

class CMapWorldInfo
{
    std::vector<u32> x4_visitedAreas;
    std::map<TEditorId, bool> x14_;
    std::vector<u32> x18_visitedAreas;
public:
    CMapWorldInfo()=default;
    CMapWorldInfo(CBitStreamReader&, const CSaveWorld& saveWorld, ResId mlvlId);
    void PutTo(CBitStreamWriter& writer, const CSaveWorld& savw, ResId mlvlId) const;
    bool IsMapped() const;
    void SetIsMapped(bool) const;
    void SetDoorVisited(TEditorId eid, bool val);
    bool IsDoorVisited() const;
    bool IsAreaVisted(TAreaId);
    void SetAreaVisited(TAreaId, bool);
};
}

#endif // __URDE_CMAPWORLDINFO_HPP__
