#pragma once

#include "World/CPatterned.hpp"
#include "Weapon/CProjectileInfo.hpp"
#include "Particle/CElementGen.hpp"

namespace urde::MP1 {
class CPuddleSpore : public CPatterned {
  static constexpr u32 kEyeCount = 16;
  static const std::string_view kEyeLocators[16];
  float x568_ = 0.f;
  float x56c_ = 0.f;
  float x570_;
  float x574_;
  float x578_;
  float x57c_;
  float x580_;
  zeus::CVector3f x584_bodyOrigin;
  float x590_halfExtent;
  float x594_height;
  float x598_ = 0.f;
  float x59c_ = 1.f;
  CCollidableAABox x5a0_;
  u32 x5c8_ = 0;
  u32 x5cc_ = 0;
  TToken<CGenDescription> x5d0_;
  std::vector<std::unique_ptr<CElementGen>> x5dc_; // originally a vector of CElementGen
  CProjectileInfo x5ec_projectileInfo;
  bool x614_24 : 1;
  bool x614_25 : 1;
public:
  DEFINE_PATTERNED(PuddleSpore)

  CPuddleSpore(TUniqueId, std::string_view, EFlavorType, const CEntityInfo&, const zeus::CTransform&,
               CModelData&&, const CPatternedInfo&, EColliderType, CAssetId, float, float, float, float, float,
               const CActorParameters&, CAssetId, const CDamageInfo&);

  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr);
  zeus::CAABox CalculateBoundingBox() const;
  CProjectileInfo* GetProjectileInfo() { return &x5ec_projectileInfo; }
};
}