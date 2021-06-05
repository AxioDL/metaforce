#include "Runtime/AutoMapper/CMapWorldInfo.hpp"

#include "Runtime/CMemoryCardSys.hpp"
#include "Runtime/GameGlobalObjects.hpp"

namespace metaforce {

CMapWorldInfo::CMapWorldInfo(CBitStreamReader& reader, const CWorldSaveGameInfo& savw, CAssetId mlvlId) {
  const CSaveWorldMemory& worldMem = g_MemoryCardSys->GetSaveWorldMemory(mlvlId);

  x4_visitedAreas.reserve((worldMem.GetAreaCount() + 31) / 32);
  for (u32 i = 0; i < worldMem.GetAreaCount(); ++i) {
    const bool visited = reader.ReadEncoded(1) != 0;
    SetAreaVisited(i, visited);
  }

  x18_mappedAreas.reserve((worldMem.GetAreaCount() + 31) / 32);
  for (u32 i = 0; i < worldMem.GetAreaCount(); ++i) {
    const bool mapped = reader.ReadEncoded(1) != 0;
    SetIsMapped(i, mapped);
  }

  for (const auto& doorId : savw.GetDoors()) {
    SetDoorVisited(doorId, reader.ReadEncoded(1) != 0);
  }

  x38_mapStationUsed = reader.ReadEncoded(1) != 0;
}

void CMapWorldInfo::PutTo(CBitStreamWriter& writer, const CWorldSaveGameInfo& savw, CAssetId mlvlId) const {
  const CSaveWorldMemory& worldMem = g_MemoryCardSys->GetSaveWorldMemory(mlvlId);

  for (u32 i = 0; i < worldMem.GetAreaCount(); ++i) {
    if (i < x0_visitedAreasAllocated) {
      writer.WriteEncoded(u32(IsAreaVisited(i)), 1);
    } else {
      writer.WriteEncoded(0, 1);
    }
  }

  for (u32 i = 0; i < worldMem.GetAreaCount(); ++i) {
    if (i < x14_mappedAreasAllocated) {
      writer.WriteEncoded(u32(IsMapped(i)), 1);
    } else {
      writer.WriteEncoded(0, 1);
    }
  }

  for (const auto& doorId : savw.GetDoors()) {
    writer.WriteEncoded(u32(IsDoorVisited(doorId)), 1);
  }

  writer.WriteEncoded(u32(x38_mapStationUsed), 1);
}

void CMapWorldInfo::SetDoorVisited(TEditorId eid, bool visited) { x28_visitedDoors[eid] = visited; }

bool CMapWorldInfo::IsDoorVisited(TEditorId eid) const { return x28_visitedDoors.find(eid) != x28_visitedDoors.end(); }

bool CMapWorldInfo::IsAreaVisited(TAreaId aid) const {
  if (u32(aid) + 1 > x0_visitedAreasAllocated) {
    x4_visitedAreas.resize((u32(aid) + 32) / 32);
    x0_visitedAreasAllocated = u32(aid) + 1;
  }
  return ((x4_visitedAreas[aid / 32] >> (aid % 32)) & 1) != 0;
}

void CMapWorldInfo::SetAreaVisited(TAreaId aid, bool visited) {
  if (u32(aid) + 1 > x0_visitedAreasAllocated) {
    x4_visitedAreas.resize((u32(aid) + 32) / 32);
    x0_visitedAreasAllocated = u32(aid) + 1;
  }

  if (visited) {
    x4_visitedAreas[aid / 32] |= 1U << (aid % 32);
  } else {
    x4_visitedAreas[aid / 32] &= ~(1U << (aid % 32));
  }
}

bool CMapWorldInfo::IsMapped(TAreaId aid) const {
  if (u32(aid) + 1 > x14_mappedAreasAllocated) {
    x18_mappedAreas.resize((u32(aid) + 32) / 32);
    x14_mappedAreasAllocated = u32(aid) + 1;
  }
  return ((x18_mappedAreas[aid / 32] >> (aid % 32)) & 1) != 0;
}

void CMapWorldInfo::SetIsMapped(TAreaId aid, bool mapped) {
  if (u32(aid) + 1 > x14_mappedAreasAllocated) {
    x18_mappedAreas.resize((u32(aid) + 32) / 32);
    x14_mappedAreasAllocated = u32(aid) + 1;
  }

  if (mapped) {
    x18_mappedAreas[aid / 32] |= 1U << (aid % 32);
  } else {
    x18_mappedAreas[aid / 32] &= ~(1U << (aid % 32));
  }
}

bool CMapWorldInfo::IsWorldVisible(TAreaId aid) const { return x38_mapStationUsed || IsMapped(aid); }

bool CMapWorldInfo::IsAreaVisible(TAreaId aid) const { return IsAreaVisited(aid) || IsMapped(aid); }

bool CMapWorldInfo::IsAnythingSet() const {
  for (u32 i = 0; i < x0_visitedAreasAllocated; ++i) {
    if ((x4_visitedAreas[i / 32] & (1U << (i % 32))) != 0) {
      return true;
    }
  }
  for (u32 i = 0; i < x14_mappedAreasAllocated; ++i) {
    if ((x18_mappedAreas[i / 32] & (1U << (i % 32))) != 0) {
      return true;
    }
  }
  return x38_mapStationUsed;
}

} // namespace metaforce
