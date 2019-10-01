#pragma once

#include <utility>
#include <vector>

#include "Runtime/CToken.hpp"
#include "Runtime/IOStreams.hpp"
#include "Runtime/RetroTypes.hpp"

namespace urde {
class CSimplePool;
class CGenDescription;

class CSpawnSystemKeyframeData {
public:
  class CSpawnSystemKeyframeInfo {
    friend class CSpawnSystemKeyframeData;
    u32 x0_id;
    u32 x4;
    u32 x8;
    u32 xc;
    TLockedToken<CGenDescription> x10_token;
    bool x18_found = false;
    void LoadToken(CSimplePool* pool);

  public:
    CSpawnSystemKeyframeInfo(CInputStream& in);
    TLockedToken<CGenDescription>& GetToken() { return x10_token; }
  };

private:
  u32 x0;
  u32 x4;
  u32 x8_endFrame;
  u32 xc;
  std::vector<std::pair<u32, std::vector<CSpawnSystemKeyframeInfo>>> x10_spawns;

public:
  CSpawnSystemKeyframeData(CInputStream& in);
  void LoadAllSpawnedSystemTokens(CSimplePool* pool);
  std::vector<CSpawnSystemKeyframeInfo>& GetSpawnedSystemsAtFrame(u32 frame);
};

} // namespace urde
