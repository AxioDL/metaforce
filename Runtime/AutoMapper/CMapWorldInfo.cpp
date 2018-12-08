#include "CMapWorldInfo.hpp"
#include "GameGlobalObjects.hpp"
#include "CMemoryCardSys.hpp"

namespace urde {

CMapWorldInfo::CMapWorldInfo(CBitStreamReader& reader, const CSaveWorld& savw, CAssetId mlvlId) {
  const CSaveWorldMemory& worldMem = g_MemoryCardSys->GetSaveWorldMemory(mlvlId);

  x4_visitedAreas.reserve((worldMem.GetAreaCount() + 31) / 32);
  for (int i = 0; i < worldMem.GetAreaCount(); ++i) {
    bool visited = reader.ReadEncoded(1);
    SetAreaVisited(i, visited);
  }

  x18_mappedAreas.reserve((worldMem.GetAreaCount() + 31) / 32);
  for (int i = 0; i < worldMem.GetAreaCount(); ++i) {
    bool mapped = reader.ReadEncoded(1);
    SetIsMapped(i, mapped);
  }

  for (TEditorId doorId : savw.GetDoors())
    SetDoorVisited(doorId, reader.ReadEncoded(1));

  x38_mapStationUsed = reader.ReadEncoded(1);
}

void CMapWorldInfo::PutTo(CBitStreamWriter& writer, const CSaveWorld& savw, CAssetId mlvlId) const {
  const CSaveWorldMemory& worldMem = g_MemoryCardSys->GetSaveWorldMemory(mlvlId);

  for (int i = 0; i < worldMem.GetAreaCount(); ++i) {
    if (i < x0_visitedAreasAllocated)
      writer.WriteEncoded(IsAreaVisted(i), 1);
    else
      writer.WriteEncoded(0, 1);
  }

  for (int i = 0; i < worldMem.GetAreaCount(); ++i) {
    if (i < x14_mappedAreasAllocated)
      writer.WriteEncoded(IsMapped(i), 1);
    else
      writer.WriteEncoded(0, 1);
  }

  for (TEditorId doorId : savw.GetDoors())
    writer.WriteEncoded(IsDoorVisited(doorId), 1);

  writer.WriteEncoded(x38_mapStationUsed, 1);
}

void CMapWorldInfo::SetDoorVisited(TEditorId eid, bool visited) { x28_visitedDoors[eid] = visited; }

bool CMapWorldInfo::IsDoorVisited(TEditorId eid) const { return x28_visitedDoors.find(eid) != x28_visitedDoors.end(); }

bool CMapWorldInfo::IsAreaVisted(TAreaId aid) const {
  if (aid + 1 > x0_visitedAreasAllocated) {
    const_cast<CMapWorldInfo&>(*this).x4_visitedAreas.resize((aid + 32) / 32);
    const_cast<CMapWorldInfo&>(*this).x0_visitedAreasAllocated = aid + 1;
  }
  return (x4_visitedAreas[aid / 32] >> (aid % 32)) & 0x1;
}

void CMapWorldInfo::SetAreaVisited(TAreaId aid, bool visited) {
  if (aid + 1 > x0_visitedAreasAllocated) {
    x4_visitedAreas.resize((aid + 32) / 32);
    x0_visitedAreasAllocated = aid + 1;
  }
  if (visited)
    x4_visitedAreas[aid / 32] |= 1 << (aid % 32);
  else
    x4_visitedAreas[aid / 32] &= ~(1 << (aid % 32));
}

bool CMapWorldInfo::IsMapped(TAreaId aid) const {
  if (aid + 1 > x14_mappedAreasAllocated) {
    const_cast<CMapWorldInfo&>(*this).x18_mappedAreas.resize((aid + 32) / 32);
    const_cast<CMapWorldInfo&>(*this).x14_mappedAreasAllocated = aid + 1;
  }
  return (x18_mappedAreas[aid / 32] >> (aid % 32)) & 0x1;
}

void CMapWorldInfo::SetIsMapped(TAreaId aid, bool mapped) {
  if (aid + 1 > x14_mappedAreasAllocated) {
    x18_mappedAreas.resize((aid + 32) / 32);
    x14_mappedAreasAllocated = aid + 1;
  }
  if (mapped)
    x18_mappedAreas[aid / 32] |= 1 << (aid % 32);
  else
    x18_mappedAreas[aid / 32] &= ~(1 << (aid % 32));
}

bool CMapWorldInfo::IsWorldVisible(TAreaId aid) const { return x38_mapStationUsed || IsMapped(aid); }

bool CMapWorldInfo::IsAreaVisible(TAreaId aid) const {
  if (IsAreaVisted(aid) || IsMapped(aid))
    return true;
  return false;
}

bool CMapWorldInfo::IsAnythingSet() const {
  for (int i = 0; i < x0_visitedAreasAllocated; ++i)
    if (x4_visitedAreas[i / 32] & (1 << (i % 32)))
      return true;
  for (int i = 0; i < x14_mappedAreasAllocated; ++i)
    if (x18_mappedAreas[i / 32] & (1 << (i % 32)))
      return true;
  return x38_mapStationUsed;
}

} // namespace urde
