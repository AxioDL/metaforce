#ifndef _CWORLDSAVEGAMEINFO
#define _CWORLDSAVEGAMEINFO

#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "rstl/pair.hpp"
#include "rstl/vector.hpp"

class CWorldSaveGameInfo {
public:
  enum EScanCategory { kSC_None, kSC_Data, kSC_Lore, kSC_Creature, kSC_Research, kSC_Artifact };

  struct SLayerState {
    SLayerState(CInputStream& in);

    TAreaId mArea;
    uint mLayer;
  };

  explicit CWorldSaveGameInfo(CInputStream& in);

  int GetCinematicCount() const { return mCinematics.size(); }
  const rstl::vector< TEditorId >& GetCinematics() const { return mCinematics; }

  const rstl::vector< TEditorId >& GetDoors() const { return mDoors; }

  typedef rstl::pair< CAssetId, uint > ScanState;
  uint GetAreaCount() const { return mAreaCount; }
  const rstl::vector< ScanState >& GetScans() const { return mScans; }

  int GetRelayIndex(const TEditorId&) const;

  const rstl::vector< TEditorId >& GetRelays() const { return mRelays; }

private:
  uint mAreaCount;
  rstl::vector< TEditorId > mCinematics;
  rstl::vector< TEditorId > mRelays;
  rstl::vector< SLayerState > mLayers;
  rstl::vector< TEditorId > mDoors;
  rstl::vector< ScanState > mScans;
};

CHECK_SIZEOF(CWorldSaveGameInfo, 0x54)

#endif // _CWORLDSAVEGAMEINFO
