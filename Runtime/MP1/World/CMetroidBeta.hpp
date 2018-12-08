#pragma once

#include "World/CPatterned.hpp"
#include "CMetroid.hpp"

namespace urde::MP1 {

class CMetroidBetaData {
  CDamageVulnerability x0_;
  CDamageVulnerability x68_;
  float xd0_;
  float xd4_;
  float xd8_;
  float xdc_;
  float xe0_;
  float xe4_;
  float xe8_;
  float xec_;
  float xf0_;
  CAssetId xf4_;
  CAssetId xf8_;
  CAssetId xfc_;
  CAssetId x100_;
  CAssetId x104_;
  bool x108_24_ : 1;

public:
  CMetroidBetaData(CInputStream&);
};
class CMetroidBeta : public CPatterned {
public:
  DEFINE_PATTERNED(MetroidBeta)
  CMetroidBeta(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
               CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& aParms,
               const CMetroidBetaData& metroidData);
  void RenderHitGunEffect() const;
  void RenderHitBallEffect() const;
};

} // namespace urde::MP1
