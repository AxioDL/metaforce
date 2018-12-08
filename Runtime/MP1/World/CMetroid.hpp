#pragma once

#include "World/CPatterned.hpp"
#include "World/CAnimationParameters.hpp"

namespace urde::MP1 {

class CMetroidData {
  CDamageVulnerability x0_dVuln1;
  CDamageVulnerability x68_dVuln2;
  float xd0_;
  float xd4_;
  float xd8_;
  float xdc_;
  float xe0_;
  float xe4_;
  std::experimental::optional<CAnimationParameters> xe8_animParms1;
  std::experimental::optional<CAnimationParameters> xf8_animParms2;
  std::experimental::optional<CAnimationParameters> x108_animParms3;
  std::experimental::optional<CAnimationParameters> x118_animParms4;
  bool x128_24_ : 1;

public:
  CMetroidData(CInputStream& in);
};

class CMetroid : public CPatterned {
public:
  DEFINE_PATTERNED(Metroid)
  CMetroid(TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo& info,
           const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& aParms,
           const CMetroidData& metroidData);
};

} // namespace urde::MP1
