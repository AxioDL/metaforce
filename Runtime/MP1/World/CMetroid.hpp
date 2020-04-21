#pragma once

#include <optional>
#include <string_view>

#include "Runtime/World/CAnimationParameters.hpp"
#include "Runtime/World/CPatterned.hpp"

namespace urde::MP1 {

class CMetroidData {
  static constexpr u32 skNumProperties = 20;
  CDamageVulnerability x0_dVuln1;
  CDamageVulnerability x68_dVuln2;
  float xd0_;
  float xd4_;
  float xd8_;
  float xdc_;
  float xe0_;
  float xe4_;
  std::optional<CAnimationParameters> xe8_animParms1;
  std::optional<CAnimationParameters> xf8_animParms2;
  std::optional<CAnimationParameters> x108_animParms3;
  std::optional<CAnimationParameters> x118_animParms4;
  bool x128_24_ : 1;

public:
  explicit CMetroidData(CInputStream& in);
  static u32 GetNumProperties() { return skNumProperties; }
};

class CMetroid : public CPatterned {
  bool x9bf_29_ : 1 = false;

public:
  DEFINE_PATTERNED(Metroid)
  CMetroid(TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo& info,
           const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& aParms,
           const CMetroidData& metroidData, TUniqueId);
  bool GetX9BF_29() const { return x9bf_29_; }
};

} // namespace urde::MP1
