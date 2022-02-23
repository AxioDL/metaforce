#include "Runtime/CGameHintInfo.hpp"

#include "Runtime/CMemoryCardSys.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/GameGlobalObjects.hpp"

namespace metaforce {

CGameHintInfo::CGameHintInfo(CInputStream& in, s32 version) {
  u32 hintCount = in.ReadLong();
  x0_hints.reserve(hintCount);
  for (u32 i = 0; i < hintCount; ++i)
    x0_hints.emplace_back(in, version);
}

CGameHintInfo::CGameHint::CGameHint(CInputStream& in, s32 version)
: x0_name(in.Get<std::string>())
, x10_immediateTime(in.ReadFloat())
, x14_normalTime(in.ReadFloat())
, x18_stringId(in.Get<CAssetId>())
, x1c_textTime(3.f * float(version <= 0 ? 1 : in.ReadLong())) {
  u32 locationCount = in.ReadLong();
  x20_locations.reserve(locationCount);
  for (u32 i = 0; i < locationCount; ++i)
    x20_locations.emplace_back(in, version);
}

CGameHintInfo::SHintLocation::SHintLocation(CInputStream& in, s32)
: x0_mlvlId(in.Get<CAssetId>())
, x4_mreaId(in.Get<CAssetId>())
, x8_areaId(in.ReadLong())
, xc_stringId(in.Get<CAssetId>()) {}

int CGameHintInfo::FindHintIndex(std::string_view str) {
  const std::vector<CGameHint>& gameHints = g_MemoryCardSys->GetHints();
  const auto it =
      std::find_if(gameHints.cbegin(), gameHints.cend(), [&str](const CGameHint& gh) { return gh.GetName() == str; });

  return it != gameHints.cend() ? it - gameHints.cbegin() : -1;
}

CFactoryFnReturn FHintFactory(const SObjectTag&, CInputStream& in, const CVParamTransfer&, CObjectReference*) {
  in.ReadLong();
  s32 version = in.ReadInt32();

  return TToken<CGameHintInfo>::GetIObjObjectFor(std::make_unique<CGameHintInfo>(in, version));
}
} // namespace metaforce
