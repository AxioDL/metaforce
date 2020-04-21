#pragma once

#include "Runtime/Weapon/CGameProjectile.hpp"
#include "Runtime/IOStreams.hpp"

namespace urde {
class CFlameInfo {
  friend class CFlameThrower;
  s32 x0_propertyCount;
  s32 x4_attributes;
  CAssetId x8_flameFxId;
  s32 xc_length;
  float x10_;
  // float x14_;
  float x18_;
  float x1c_;

public:
  CFlameInfo(s32, s32, CAssetId, s32, float, float, float);
  CFlameInfo(CInputStream& in);

  s32 GetAttributes() const { return x4_attributes; }
  s32 GetLength() const { return xc_length; }
  CAssetId GetFlameFxId() const { return x8_flameFxId; }
};
} // namespace urde
