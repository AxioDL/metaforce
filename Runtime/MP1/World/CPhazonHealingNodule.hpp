#pragma once

#include "Runtime/World/CPatterned.hpp"
#include "Runtime/Particle/CParticleElectric.hpp"

namespace urde::MP1 {
class CPhazonHealingNodule : public CPatterned {
private:
  int x568_ = 0;
  u8 x56c_ = 0;
  TUniqueId x56e_ = kInvalidUniqueId;
  TCachedToken<CElectricDescription> x570_;
  std::unique_ptr<CParticleElectric> x57c_; // was rstl::rc_ptr
  CHealthInfo x580_;
  int x588_ = 0; // not init in ctr
  std::string x58c_;
  // u32 x59c_;

public:
  CPhazonHealingNodule(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                       CModelData&& mData, const CActorParameters& actParams, const CPatternedInfo& pInfo,
                       CAssetId particleDescId, std::string str);
};
} // namespace urde::MP1
