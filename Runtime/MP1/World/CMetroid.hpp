#pragma once

#include "World/CPatterned.hpp"
#include "World/CAnimationParameters.hpp"

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
  rstl::optional<CAnimationParameters> xe8_animParms1;
  rstl::optional<CAnimationParameters> xf8_animParms2;
  rstl::optional<CAnimationParameters> x108_animParms3;
  rstl::optional<CAnimationParameters> x118_animParms4;
  bool x128_24_ : 1;

public:
  CMetroidData(CInputStream& in);
  static u32 GetNumProperties() { return skNumProperties; }
};

class CMetroid : public CPatterned {
  union {
    struct {
      bool x9bf_29_ : 1;
    };
    u32 _dummy = 0;
  };

public:
  DEFINE_PATTERNED(Metroid)
  CMetroid(TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo& info,
           const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& aParms,
           const CMetroidData& metroidData, TUniqueId);
  bool GetX9BF_29() const { return x9bf_29_; }
};

} // namespace urde::MP1
