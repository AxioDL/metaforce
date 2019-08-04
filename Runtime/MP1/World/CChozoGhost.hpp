#pragma once

#include "World/CPatterned.hpp"
#include "Weapon/CProjectileInfo.hpp"
#include "Character/CBoneTracking.hpp"

namespace urde::MP1 {
class CChozoGhost : public CPatterned {
public:
  class CBehaveChance {
    u32 x0_propertyCount;
    float x4_;
    float x8_;
    float xc_;
    float x10_;
    float x14_;
    float x18_;
    u32 x1c_;

  public:
    CBehaveChance(CInputStream&);
  };

private:
  float x568_;
  float x56c_;
  float x570_;
  float x574_;
  CProjectileInfo x578_;
  CProjectileInfo x5a0_;
  CBehaveChance x5c8_;
  CBehaveChance x5e8_;
  CBehaveChance x608_;
  s16 x628_;
  float x62c_;
  s16 x630_;
  s16 x632_;
  float x634_;
  float x638_;
  u32 x63c_;
  TLockedToken<CGenDescription> x640_;
  s16 x650_;
  float x654_;
  float x658_;
  u32 x65c_;
  u32 x660_;
  union {
    struct {
      bool x664_24_ : 1;
      bool x664_25_ : 1;
      bool x664_26_ : 1;
      bool x664_27_ : 1;
      bool x664_28_ : 1;
      bool x664_29_ : 1;
      bool x664_30_ : 1;
      bool x664_31_ : 1;
      bool x665_24_ : 1;
      bool x665_25_ : 1;
      bool x665_26_ : 1;
      bool x665_27_ : 1;
      bool x665_28_ : 1;
      bool x665_29_ : 1;
    };
    u32 _dummy = 0;
  };
  float x668_ = 0.f;
  float x66c_ = 0.f;
  float x670_ = 0.f;
  TUniqueId x674_ = kInvalidUniqueId;
  u32 x67c_ = -1;
  u32 x680_;
  float x684_ = 1.f;
  CSteeringBehaviors x688_;
  //CBoneTracking x68c_boneTracking;
  TUniqueId x6c4_ = kInvalidUniqueId;
  float x6c8_ = 0.f;
  zeus::CVector3f x6cc_;
  u32 x6d8_ = 1;
public:
  DEFINE_PATTERNED(ChozoGhost)

  CChozoGhost(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
              const CActorParameters&, const CPatternedInfo&, float, float, float, float, CAssetId, const CDamageInfo&,
              CAssetId, const CDamageInfo&, const CChozoGhost::CBehaveChance&, const CChozoGhost::CBehaveChance&,
              const CBehaveChance&, u16, float, u16, u16, u32, float, u32, float, CAssetId, s16, float, float, u32,
              u32);
};
} // namespace urde::MP1