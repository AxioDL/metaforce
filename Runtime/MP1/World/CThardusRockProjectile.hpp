#pragma once

#include "Runtime/World/CPatterned.hpp"

namespace urde::MP1 {
class CThardusRockProjectile : public CPatterned {
  float x568_;
  u32 x56c_ = 0;
  TUniqueId x570_ = kInvalidUniqueId;
  bool x572_ = false;
  s32 x574_ = -1;
  s32 x578_ = 0;
  std::vector<std::unique_ptr<CModelData>> x57c_;
  u32 x590_ = 0;

public:
  DEFINE_PATTERNED(ThardusRockProjectile)
  CThardusRockProjectile(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                         CModelData&& modelData, const CActorParameters& aParms, const CPatternedInfo& patternedInfo,
                         std::vector<std::unique_ptr<CModelData>>&& mDataVec, CAssetId, float);

  void sub80203d58() {
    x328_25_verticalMovement = false;
    x150_momentum = {0.f, 0.f, 2.f * -GetWeight()};
    x56c_ = 3;
  }
};
} // namespace urde::MP1
