#include "Runtime/CWorldSaveGameInfo.hpp"

#include "Runtime/CToken.hpp"

namespace metaforce {
CWorldSaveGameInfo::CWorldSaveGameInfo(CInputStream& in) {
  in.ReadLong();
  const u32 version = in.ReadLong();
  if (version > 1) {
    x0_areaCount = in.ReadLong();
  }

  if (version > 2) {
    const u32 cinematicCount = in.ReadLong();
    x4_cinematics.reserve(cinematicCount);
    for (u32 i = 0; i < cinematicCount; ++i) {
      x4_cinematics.emplace_back(in.ReadLong());
    }

    const u32 relayCount = in.ReadLong();
    x14_relays.reserve(relayCount);
    for (u32 i = 0; i < relayCount; ++i) {
      x14_relays.emplace_back(in.ReadLong());
    }
  }

  const u32 layerCount = in.ReadLong();
  x24_layers.reserve(layerCount);
  for (u32 i = 0; i < layerCount; ++i) {
    SLayerState& st = x24_layers.emplace_back();
    st.x0_area = in.ReadLong();
    st.x4_layer = in.ReadLong();
  }

  const u32 doorCount = in.ReadLong();
  x34_doors.reserve(doorCount);
  for (u32 i = 0; i < doorCount; ++i) {
    x34_doors.emplace_back(in.ReadLong());
  }

  if (version <= 0) {
    return;
  }

  const u32 scanCount = in.ReadLong();
  x44_scans.reserve(scanCount);
  for (u32 i = 0; i < scanCount; ++i) {
    SScanState& st = x44_scans.emplace_back();
    st.x0_id = in.Get<CAssetId>();
    st.x4_category = EScanCategory(in.ReadLong());
  }
}

u32 CWorldSaveGameInfo::GetAreaCount() const { return x0_areaCount; }

u32 CWorldSaveGameInfo::GetCinematicCount() const { return x4_cinematics.size(); }

s32 CWorldSaveGameInfo::GetCinematicIndex(const TEditorId& id) const {
  auto it = std::find(x4_cinematics.begin(), x4_cinematics.end(), id);
  if (it == x4_cinematics.end())
    return -1;
  return it - x4_cinematics.begin();
}

u32 CWorldSaveGameInfo::GetRelayCount() const { return x14_relays.size(); }

s32 CWorldSaveGameInfo::GetRelayIndex(const TEditorId& id) const {
  auto it = std::find(x14_relays.begin(), x14_relays.end(), id);
  if (it == x14_relays.end())
    return -1;
  return it - x14_relays.begin();
}

TEditorId CWorldSaveGameInfo::GetRelayEditorId(u32 idx) const { return x14_relays[idx]; }

u32 CWorldSaveGameInfo::GetDoorCount() const { return x34_doors.size(); }

s32 CWorldSaveGameInfo::GetDoorIndex(const TEditorId& id) const {
  auto it = std::find(x34_doors.begin(), x34_doors.end(), id);
  if (it == x34_doors.end())
    return -1;
  return it - x34_doors.begin();
}

CFactoryFnReturn FWorldSaveGameInfoFactory([[maybe_unused]] const SObjectTag& tag, CInputStream& in,
                                           [[maybe_unused]] const CVParamTransfer& param,
                                           [[maybe_unused]] CObjectReference* selfRef) {
  return TToken<CWorldSaveGameInfo>::GetIObjObjectFor(std::make_unique<CWorldSaveGameInfo>(in));
}

} // namespace metaforce
