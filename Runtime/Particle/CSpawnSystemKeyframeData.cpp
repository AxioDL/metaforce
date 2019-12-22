#include "Runtime/Particle/CSpawnSystemKeyframeData.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/Graphics/CModel.hpp"
#include "Runtime/Particle/CElectricDescription.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/Particle/CSwooshDescription.hpp"

namespace urde {

CSpawnSystemKeyframeData::CSpawnSystemKeyframeData(CInputStream& in) {
  x0 = in.readUint32Big();
  x4 = in.readUint32Big();
  x8_endFrame = in.readUint32Big();
  xc = in.readUint32Big();

  u32 count = in.readUint32Big();
  x10_spawns.reserve(count);
  for (u32 i = 0; i < count; ++i) {
    u32 v1 = in.readUint32Big();
    x10_spawns.emplace_back(v1, std::vector<CSpawnSystemKeyframeInfo>());
    std::vector<CSpawnSystemKeyframeInfo>& v2 = x10_spawns.back().second;
    u32 v2c = in.readUint32Big();
    v2.reserve(v2c);
    for (u32 j = 0; j < v2c; ++j)
      v2.emplace_back(in);
  }
}

CSpawnSystemKeyframeData::CSpawnSystemKeyframeInfo::CSpawnSystemKeyframeInfo(CInputStream& in) {
  x0_id = in.readUint32Big();
  x4 = in.readUint32Big();
  x8 = in.readUint32Big();
  xc = in.readUint32Big();
}

void CSpawnSystemKeyframeData::LoadAllSpawnedSystemTokens(CSimplePool* pool) {
  for (auto& spawn : x10_spawns)
    for (CSpawnSystemKeyframeInfo& elem : spawn.second)
      elem.LoadToken(pool);
}

void CSpawnSystemKeyframeData::CSpawnSystemKeyframeInfo::LoadToken(CSimplePool* pool) {
  x10_token = std::move(pool->GetObj({FOURCC('PART'), x0_id}));
  x18_found = true;
}

std::vector<CSpawnSystemKeyframeData::CSpawnSystemKeyframeInfo>&
CSpawnSystemKeyframeData::GetSpawnedSystemsAtFrame(u32 frame) {
  static std::vector<CSpawnSystemKeyframeData::CSpawnSystemKeyframeInfo> emptyReturn;
  if (frame >= x8_endFrame)
    return emptyReturn;
  for (auto& spawn : x10_spawns)
    if (spawn.first == frame)
      return spawn.second;
  return emptyReturn;
}

} // namespace urde
