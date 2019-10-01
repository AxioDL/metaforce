#pragma once

#include "Runtime/Character/CBoneTracking.hpp"
#include "Runtime/World/CPatterned.hpp"

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
  float x568_;
  float x56c_;
  float x570_;
  float x574_;
  float x578_;
  float x57c_;
  u32 x580_;
  CBoneTracking x584_boneTracker;
public:
  DEFINE_PATTERNED(Magdolite)
  CMagdolite(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
             const CPatternedInfo&, const CActorParameters&, float, float, const CDamageInfo&, const CDamageInfo&,
             const CDamageVulnerability&, const CDamageVulnerability&, CAssetId, CAssetId, float, float, float, float,
             const CMagdoliteData&, float, float, float);

  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
};
} // namespace urde::MP1
