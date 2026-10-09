#ifndef _CMAPWORLDINFO
#define _CMAPWORLDINFO

#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include <rstl/bit_vector.hpp>
#include <rstl/pair.hpp>

class CWorldSaveGameInfo;
class CMapWorldInfo {
public:
  CMapWorldInfo();
  CMapWorldInfo(CInputStream& in, const CWorldSaveGameInfo&, CAssetId worldResId);
  bool IsMapped(TAreaId areaId) const;
  bool IsAreaVisited(TAreaId areaId) const;
  bool IsAreaVisible(const TAreaId areaId) const;
  bool IsWorldVisible(const TAreaId areaId) const;
  bool IsAnythingSet();
  bool IsDoorVisited(const TEditorId eid) const;
  void SetIsMapped(TAreaId areaId, bool mapped);
  void PutTo(COutputStream& out, const CWorldSaveGameInfo& saveInfo, int worldId) const;
  bool GetMapStationUsed() const { return mMapStationUsed; }
  void SetIsMapped(bool mapped) { mMapStationUsed = mapped; }
  void SetAreaVisited(TAreaId areaId, bool visited);
  void SetDoorVisited(TEditorId eid, const bool visited);

private:
  mutable rstl::bit_vector<> mVisitedAreas;
  mutable rstl::bit_vector<> mMappedAreas;
  mutable rstl::vector< rstl::pair< TEditorId, bool > > mVisitedDoors;
  bool mMapStationUsed;
};
#if VERSION < VERSION_R3IJ_00
CHECK_SIZEOF(CMapWorldInfo, 0x3c)
#endif

#endif // _CMAPWORLDINFO
