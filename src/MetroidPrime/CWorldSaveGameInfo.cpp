#include "MetroidPrime/CWorldSaveGameInfo.hpp"

#include "Kyoto/CFactoryFnReturn.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

inline CWorldSaveGameInfo::SLayerState::SLayerState(CInputStream& in)
: mArea(in.ReadLong()), mLayer(in.ReadLong()) {}

CWorldSaveGameInfo::CWorldSaveGameInfo(CInputStream& in) : mAreaCount(0) {
  in.ReadLong();
  const uint version = in.ReadLong();
  if (version > 1) {
    mAreaCount = in.ReadLong();
  }
  if (version > 2) {
    mCinematics = rstl::vector< TEditorId >(in);
  }
  if (version > 2) {
    mRelays = rstl::vector< TEditorId >(in);
  }
  mLayers = rstl::vector< SLayerState >(in);
  mDoors = rstl::vector< TEditorId >(in);
  if (version > 0) {
    mScans = rstl::vector< ScanState >(in);
  }
}

int CWorldSaveGameInfo::GetRelayIndex(const TEditorId& id) const {
  for (int i = 0; i < mRelays.size(); ++i) {
    if (mRelays[i] == id) {
      return i;
    }
  }
  return -1;
}

const CFactoryFnReturn FSaveWorldFactory(const SObjectTag& tag, CInputStream& in,
                                  const CVParamTransfer& params) {
  return rs_new CWorldSaveGameInfo(in);
}
