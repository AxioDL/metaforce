#include "CMapWorldInfo.hpp"
#include "GameGlobalObjects.hpp"
#include "CMemoryCardSys.hpp"

namespace urde
{

CMapWorldInfo::CMapWorldInfo(CBitStreamReader& reader, const CSaveWorld& savw, ResId mlvlId)
{
    const CSaveWorldMemory& worldMem = g_MemoryCardSys->GetSaveWorldMemory(mlvlId);

    x4_visitedAreas.reserve((worldMem.GetAreaCount() + 31) / 32);
    for (int i=0 ; i<worldMem.GetAreaCount() ; ++i)
    {
        bool visited = reader.ReadEncoded(1);
        SetAreaVisited(i, visited);
    }

    x18_mappedAreas.reserve((worldMem.GetAreaCount() + 31) / 32);
    for (int i=0 ; i<worldMem.GetAreaCount() ; ++i)
    {
        bool mapped = reader.ReadEncoded(1);
        SetIsMapped(i, mapped);
    }

    for (TEditorId doorId : savw.GetDoors())
        SetDoorVisited(doorId, reader.ReadEncoded(1));

    x38_worldVisited = reader.ReadEncoded(1);
}

void CMapWorldInfo::PutTo(CBitStreamWriter& writer, const CSaveWorld& savw, ResId mlvlId) const
{
    const CSaveWorldMemory& worldMem = g_MemoryCardSys->GetSaveWorldMemory(mlvlId);

    for (int i=0 ; i<worldMem.GetAreaCount() ; ++i)
    {
        if (i < x0_visitedAreasAllocated)
            writer.WriteEncoded(const_cast<CMapWorldInfo&>(*this).IsAreaVisted(i), 1);
        else
            writer.WriteEncoded(0, 1);
    }

    for (int i=0 ; i<worldMem.GetAreaCount() ; ++i)
    {
        if (i < x14_mappedAreasAllocated)
            writer.WriteEncoded(const_cast<CMapWorldInfo&>(*this).IsMapped(i), 1);
        else
            writer.WriteEncoded(0, 1);
    }

    for (TEditorId doorId : savw.GetDoors())
        writer.WriteEncoded(IsDoorVisited(doorId), 1);

    writer.WriteEncoded(x38_worldVisited, 1);
}

void CMapWorldInfo::SetDoorVisited(TEditorId eid, bool visited)
{
    x28_visitedDoors[eid] = visited;
}

bool CMapWorldInfo::IsDoorVisited(TEditorId eid) const
{
    return x28_visitedDoors.find(eid) != x28_visitedDoors.end();
}

bool CMapWorldInfo::IsAreaVisted(TAreaId aid)
{
    if (aid + 1 > x0_visitedAreasAllocated)
    {
        x4_visitedAreas.resize((aid + 32) / 32);
        x0_visitedAreasAllocated = aid + 1;
    }
    return (x4_visitedAreas[aid / 32] >> (aid % 32)) & 0x1;
}

void CMapWorldInfo::SetAreaVisited(TAreaId aid, bool visited)
{
    if (aid + 1 > x0_visitedAreasAllocated)
    {
        x4_visitedAreas.resize((aid + 32) / 32);
        x0_visitedAreasAllocated = aid + 1;
    }
    if (visited)
        x4_visitedAreas[aid / 32] |= 1 << (aid % 32);
    else
        x4_visitedAreas[aid / 32] &= ~(1 << (aid % 32));
}

bool CMapWorldInfo::IsMapped(TAreaId aid)
{
    if (aid + 1 > x14_mappedAreasAllocated)
    {
        x18_mappedAreas.resize((aid + 32) / 32);
        x14_mappedAreasAllocated = aid + 1;
    }
    return (x18_mappedAreas[aid / 32] >> (aid % 32)) & 0x1;
}

void CMapWorldInfo::SetIsMapped(TAreaId aid, bool mapped)
{
    if (aid + 1 > x14_mappedAreasAllocated)
    {
        x18_mappedAreas.resize((aid + 32) / 32);
        x14_mappedAreasAllocated = aid + 1;
    }
    if (mapped)
        x18_mappedAreas[aid / 32] |= 1 << (aid % 32);
    else
        x18_mappedAreas[aid / 32] &= ~(1 << (aid % 32));
}

bool CMapWorldInfo::IsWorldVisible(TAreaId aid)
{
    return x38_worldVisited || IsMapped(aid);
}

}
