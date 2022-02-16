#pragma once

#include <map>
#include <vector>

#include "RetroTypes.hpp"
#include "IOStreams.hpp"

namespace metaforce {
class CWorldSaveGameInfo;

class CMapWorldInfo {
  mutable u32 x0_visitedAreasAllocated = 0;
  mutable std::vector<u32> x4_visitedAreas;
  mutable u32 x14_mappedAreasAllocated = 0;
  mutable std::vector<u32> x18_mappedAreas;
  std::map<TEditorId, bool> x28_visitedDoors;
  bool x38_mapStationUsed = false;

public:
  CMapWorldInfo() = default;
  explicit CMapWorldInfo(CBitStreamReader& reader, const CWorldSaveGameInfo& saveWorld, CAssetId mlvlId);
  void PutTo(CBitStreamWriter& writer, const CWorldSaveGameInfo& savw, CAssetId mlvlId) const;
  bool IsMapped(TAreaId aid) const;
  void SetIsMapped(TAreaId aid, bool mapped);
  void SetDoorVisited(TEditorId eid, bool val);
  bool IsDoorVisited(TEditorId eid) const;
  bool IsAreaVisited(TAreaId aid) const;
  void SetAreaVisited(TAreaId aid, bool visited);
  bool IsWorldVisible(TAreaId aid) const;
  bool IsAreaVisible(TAreaId aid) const;
  bool IsAnythingSet() const;
  bool GetMapStationUsed() const { return x38_mapStationUsed; }
  void SetMapStationUsed(bool isUsed) { x38_mapStationUsed = isUsed; }
};
} // namespace metaforce
