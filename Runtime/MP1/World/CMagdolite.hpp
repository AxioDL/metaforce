#pragma once

#include "World/CPatterned.hpp"

namespace urde::MP1 {
class CMagdolite : public CPatterned {
public:
  class CMagdoliteData {
    u32 x0_propertyCount;
    u32 x4_;
    CAssetId x8_;
    u32 xc_;
    float x10_;
    float x18_;
    float x1c_;

  public:
    CMagdoliteData(CInputStream&);
  };

private:
public:
  DEFINE_PATTERNED(Magdolite)
  CMagdolite(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
             const CPatternedInfo&, const CActorParameters&, float, float, const CDamageInfo&, const CDamageInfo&,
             const CDamageVulnerability&, const CDamageVulnerability&, CAssetId, CAssetId, float, float, float, float,
             const CMagdoliteData&, float, float, float);
};
} // namespace urde::MP1
