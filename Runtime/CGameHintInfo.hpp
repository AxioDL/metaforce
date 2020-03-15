#pragma once

#include <string_view>
#include <vector>

#include "Runtime/IFactory.hpp"
#include "Runtime/RetroTypes.hpp"

namespace urde {
class CGameHintInfo {
public:
  struct SHintLocation {
    CAssetId x0_mlvlId;
    CAssetId x4_mreaId;
    TAreaId x8_areaId = kInvalidAreaId;
    CAssetId xc_stringId;
    SHintLocation(CInputStream&, s32);
  };

  class CGameHint {
    std::string x0_name;
    float x10_immediateTime;
    float x14_normalTime;
    CAssetId x18_stringId;
    float x1c_textTime;
    std::vector<SHintLocation> x20_locations;

  public:
    CGameHint(CInputStream&, s32);

    float GetNormalTime() const { return x14_normalTime; }
    float GetImmediateTime() const { return x10_immediateTime; }
    float GetTextTime() const { return x1c_textTime; }
    std::string_view GetName() const { return x0_name; }
    CAssetId GetStringID() const { return x18_stringId; }
    const std::vector<SHintLocation>& GetLocations() const { return x20_locations; }
  };

private:
  std::vector<CGameHint> x0_hints;

public:
  CGameHintInfo(CInputStream&, s32);
  const std::vector<CGameHint>& GetHints() const { return x0_hints; }
  static int FindHintIndex(std::string_view str);
};

CFactoryFnReturn FHintFactory(const SObjectTag&, CInputStream&, const CVParamTransfer, CObjectReference*);
} // namespace urde
