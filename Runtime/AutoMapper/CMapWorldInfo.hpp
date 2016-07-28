#ifndef __URDE_CMAPWORLDINFO_HPP__
#define __URDE_CMAPWORLDINFO_HPP__

#include "RetroTypes.hpp"

namespace urde
{
class CMapWorldInfo
{
    std::map<TEditorId, bool> x14_;
public:
    CMapWorldInfo()=default;
    CMapWorldInfo(CInputStream&);
    void PutTo(COutputStream&);
    bool IsMapped() const;
    void SetIsMapped(bool) const;
    void SetDoorVisited(TEditorId eid, bool val);
    bool IsDoorVisited() const;
    bool IsAreaVisted(TAreaId) const;
    void SetAreaVisited(TAreaId, bool) const;
};
}

#endif // __URDE_CMAPWORLDINFO_HPP__
