#pragma once

#include "RetroTypes.hpp"

namespace urde {
class CSaveWorld;

class CMapWorldInfo {
  u32 x0_visitedAreasAllocated = 0;
  std::vector<u32> x4_visitedAreas;
  u32 x14_mappedAreasAllocated = 0;
  std::vector<u32> x18_mappedAreas;
  std::map<TEditorId, bool> x28_visitedDoors;
  bool x38_mapStationUsed = false;

public:
  CMapWorldInfo() = default;
  CMapWorldInfo(CBitStreamReader&, const CSaveWorld& saveWorld, CAssetId mlvlId);
  void PutTo(CBitStreamWriter& writer, const CSaveWorld& savw, CAssetId mlvlId) const;
  bool IsMapped(TAreaId) const;
  void SetIsMapped(TAreaId, bool);
  void SetDoorVisited(TEditorId eid, bool val);
  bool IsDoorVisited(TEditorId eid) const;
  bool IsAreaVisted(TAreaId) const;
  void SetAreaVisited(TAreaId, bool);
  bool IsWorldVisible(TAreaId) const;
  bool IsAreaVisible(TAreaId) const;
  bool IsAnythingSet() const;
  void SetMapStationUsed(bool val) { x38_mapStationUsed = val; }
};
} // namespace urde
