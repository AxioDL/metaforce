#include "Runtime/Character/CCharacterInfo.hpp"

namespace urde {

CCharacterInfo::CParticleResData::CParticleResData(CInputStream& in, u16 tableCount) {
  const u32 partCount = in.readUint32Big();
  x0_part.reserve(partCount);
  for (u32 i = 0; i < partCount; ++i) {
    x0_part.emplace_back(in.readUint32Big());
  }

  const u32 swhcCount = in.readUint32Big();
  x10_swhc.reserve(swhcCount);
  for (u32 i = 0; i < swhcCount; ++i) {
    x10_swhc.emplace_back(in.readUint32Big());
  }

  const u32 unkCount = in.readUint32Big();
  x20_elsc.reserve(unkCount);
  for (u32 i = 0; i < unkCount; ++i) {
    x20_elsc.emplace_back(in.readUint32Big());
  }

  if (tableCount > 5) {
    const u32 elscCount = in.readUint32Big();
    x30_elsc.reserve(elscCount);
    for (u32 i = 0; i < elscCount; ++i) {
      x30_elsc.emplace_back(in.readUint32Big());
    }
  }
}

static std::vector<std::pair<s32, std::pair<std::string, std::string>>> MakeAnimInfoVector(CInputStream& in) {
  std::vector<std::pair<s32, std::pair<std::string, std::string>>> ret;
  const u32 animInfoCount = in.readUint32Big();
  ret.reserve(animInfoCount);
  for (u32 i = 0; i < animInfoCount; ++i) {
    const s32 idx = in.readInt32Big();
    std::string a = in.readString();
    std::string b = in.readString();
    ret.emplace_back(idx, std::make_pair(std::move(a), std::move(b)));
  }
  return ret;
}

CCharacterInfo::CCharacterInfo(CInputStream& in)
: x0_tableCount(in.readUint16Big())
, x4_name(in.readString())
, x14_cmdl(in.readUint32Big())
, x18_cskr(in.readUint32Big())
, x1c_cinf(in.readUint32Big())
, x20_animInfo(MakeAnimInfoVector(in))
, x30_pasDatabase(in)
, x44_partRes(in, x0_tableCount)
, x84_unk(in.readUint32Big()) {
  if (x0_tableCount > 1) {
    const u32 aabbCount = in.readUint32Big();
    x88_aabbs.reserve(aabbCount);
    for (u32 i = 0; i < aabbCount; ++i) {
      std::string name = in.readString();
      x88_aabbs.emplace_back(std::move(name), zeus::CAABox());
      x88_aabbs.back().second.readBoundingBoxBig(in);
    }
  }

  if (x0_tableCount > 2) {
    const u32 effectCount = in.readUint32Big();
    x98_effects.reserve(effectCount);
    for (u32 i = 0; i < effectCount; ++i) {
      std::string name = in.readString();
      x98_effects.emplace_back(std::move(name), std::vector<CEffectComponent>());
      std::vector<CEffectComponent>& comps = x98_effects.back().second;
      const u32 compCount = in.readUint32Big();
      comps.reserve(compCount);
      for (u32 j = 0; j < compCount; ++j) {
        comps.emplace_back(in);
      }
    }
  }

  if (x0_tableCount > 3) {
    xa8_cmdlOverlay = in.readUint32Big();
    xac_cskrOverlay = in.readUint32Big();
  }

  if (x0_tableCount > 4) {
    const u32 aidxCount = in.readUint32Big();
    xb0_animIdxs.reserve(aidxCount);
    for (u32 i = 0; i < aidxCount; ++i) {
      xb0_animIdxs.push_back(in.readInt32Big());
    }
  }
}

s32 CCharacterInfo::GetAnimationIndex(std::string_view name) const {
  for (const auto& pair : x20_animInfo) {
    if (pair.second.second == name)
      return pair.first;
  }

  return -1;
}

} // namespace urde
