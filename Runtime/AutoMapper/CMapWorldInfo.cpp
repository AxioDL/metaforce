#include "CMapWorldInfo.hpp"

namespace urde
{

void CMapWorldInfo::SetDoorVisited(TEditorId eid, bool visited)
{
    x14_[eid] = visited;
}

bool CMapWorldInfo::IsAreaVisted(TAreaId aid)
{
    x4_visitedAreas.resize((aid + 31) / 32);
    return (x4_visitedAreas[aid / 32] >> (aid % 32)) & 0x1;
}

void CMapWorldInfo::SetAreaVisited(TAreaId aid, bool visited)
{
    x4_visitedAreas.resize((aid + 31) / 32);
    if (visited)
        x4_visitedAreas[aid / 32] |= 1 << (aid % 32);
    else
        x4_visitedAreas[aid / 32] &= ~(1 << (aid % 32));
}

}
