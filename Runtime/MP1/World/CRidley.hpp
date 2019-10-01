#pragma once

#include <string_view>

#include "Runtime/Camera/CCameraShakeData.hpp"
#include "Runtime/Weapon/CBeamInfo.hpp"
#include "Runtime/World/CDamageInfo.hpp"
#include "Runtime/World/CPatterned.hpp"

namespace urde {
namespace MP1 {
class CRidleyData {
  CAssetId x0_;
  CAssetId x4_;
  CAssetId x8_;
  CAssetId xc_;
  CAssetId x10_;
  CAssetId x14_;
  CAssetId x18_;
  CAssetId x1c_;
  CAssetId x20_;
  CAssetId x24_;
  CAssetId x28_;
  CAssetId x2c_;
  CAssetId x30_;
  float x34_;
  float x38_;
  float x3c_;
  float x40_;
  CAssetId x44_;
  CDamageInfo x48_;
  CBeamInfo x64_;
  u16 xa8_;
  CAssetId xac_;
  CDamageInfo xb0_;
  CCameraShakeData xcc_;
  CAssetId x1a0_;
  CDamageInfo x1a4_;
  CCameraShakeData x1c0_;
  u16 x294_;
  CDamageInfo x298_;
  CCameraShakeData x2b4_;
  float x388_;
  float x38c_;
  CDamageInfo x390_;
  float x3ac_;
  CDamageInfo x3b0_;
  float x3cc_;
  CDamageInfo x3d0_;
  float x3ec_;
  CAssetId x3f0_;
  float x3f4_;
  u16 x3f8_;
  CDamageInfo x3fc_;

public:
  CRidleyData(CInputStream&, u32);
};

class CRidley : public CPatterned {
  CRidleyData x568_;

public:
  DEFINE_PATTERNED(Ridley)
  CRidley(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&, const CPatternedInfo&,
          const CActorParameters&, CInputStream&, u32);
};
} // namespace MP1
} // namespace urde
