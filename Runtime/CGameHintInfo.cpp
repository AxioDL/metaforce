#include "CGameHintInfo.hpp"
#include "CToken.hpp"
#include "CMemoryCardSys.hpp"
#include "GameGlobalObjects.hpp"

namespace urde {

CGameHintInfo::CGameHintInfo(CInputStream& in, s32 version) {
  u32 hintCount = in.readUint32Big();
  x0_hints.reserve(hintCount);
  for (u32 i = 0; i < hintCount; ++i)
    x0_hints.emplace_back(in, version);
}

CGameHintInfo::CGameHint::CGameHint(CInputStream& in, s32 version)
: x0_name(in.readString())
, x10_immediateTime(in.readFloatBig())
, x14_normalTime(in.readFloatBig())
, x18_stringId(in.readUint32Big())
, x1c_textTime(3.f * float(version <= 0 ? 1 : in.readUint32Big())) {
  u32 locationCount = in.readUint32Big();
  x20_locations.reserve(locationCount);
  for (u32 i = 0; i < locationCount; ++i)
    x20_locations.emplace_back(in, version);
}

CGameHintInfo::SHintLocation::SHintLocation(CInputStream& in, s32)
: x0_mlvlId(in.readUint32Big())
, x4_mreaId(in.readUint32Big())
, x8_areaId(in.readUint32Big())
, xc_stringId(in.readUint32Big()) {}

int CGameHintInfo::FindHintIndex(const char* str) {
  const std::vector<CGameHint>& gameHints = g_MemoryCardSys->GetHints();
  const auto& it = std::find_if(gameHints.begin(), gameHints.end(),
                                [&str](const CGameHint& gh) -> bool { return gh.GetName() == str; });

  return (it != gameHints.end() ? it - gameHints.begin() : -1);
}

CFactoryFnReturn FHintFactory(const SObjectTag&, CInputStream& in, const CVParamTransfer, CObjectReference*) {
  in.readUint32Big();
  s32 version = in.readInt32Big();

  return TToken<CGameHintInfo>::GetIObjObjectFor(std::make_unique<CGameHintInfo>(in, version));
}
} // namespace urde
