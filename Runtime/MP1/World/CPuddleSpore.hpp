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
  std::vector<std::unique_ptr<CElementGen>> x5dc_elemGens; // originally a vector of CElementGen
  CProjectileInfo x5ec_projectileInfo;
  bool x614_24 : 1;
  bool x614_25 : 1;

  bool HitShell(const zeus::CVector3f&) const;
  void KnockPlayer(CStateManager&, float);
  void UpdateBoundingState(const zeus::CAABox&, CStateManager&, float);

public:
  DEFINE_PATTERNED(PuddleSpore)

  CPuddleSpore(TUniqueId, std::string_view, EFlavorType, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
               const CPatternedInfo&, EColliderType, CAssetId, float, float, float, float, float,
               const CActorParameters&, CAssetId, const CDamageInfo&);

  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr);
  void PreThink(float, CStateManager&);
  void Think(float, CStateManager&);
  void Render(const CStateManager&) const;
  void Touch(CActor&, CStateManager&);
  void FluidFXThink(EFluidState, CScriptWater&, CStateManager&);
  void KnockBack(const zeus::CVector3f& dir, CStateManager& mgr, const CDamageInfo& dInfo, EKnockBackType type,
                 bool inDeferred, float dt) {
    if (x5c8_ == 1)
      return;
    CPatterned::KnockBack(dir, mgr, dInfo, type, inDeferred, dt);
  }
  EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f& point, const zeus::CVector3f&,
                                                         const CWeaponMode&, EProjectileAttrib) const {
    return HitShell(point) ? EWeaponCollisionResponseTypes::Unknown84 : EWeaponCollisionResponseTypes::Unknown34;
  }
  void DoUserAnimEvent(CStateManager&, const CInt32POINode&, EUserEventType, float);
  void CollidedWith(TUniqueId uid, const CCollisionInfoList& colList, CStateManager& mgr) {
    if (x5c8_ == 2)
      return;
    CPatterned::CollidedWith(uid, colList, mgr);
  }
  const CCollisionPrimitive* GetCollisionPrimitive() const { return &x5a0_; }
  zeus::CAABox CalculateBoundingBox() const;
  CProjectileInfo* GetProjectileInfo() { return &x5ec_projectileInfo; }

  bool InAttackPosition(CStateManager&, float) { return x568_ >= x570_; }
  bool ShouldAttack(CStateManager&, float) { return x568_ >= x574_; }
  bool ShouldTurn(CStateManager&, float);
  bool AnimOver(CStateManager&, float) { return x5cc_ == 2; }

  void InActive(CStateManager&, EStateMsg, float);
  void Active(CStateManager&, EStateMsg, float);
  void Run(CStateManager&, EStateMsg, float);
  void TurnAround(CStateManager&, EStateMsg, float);
  void GetUp(CStateManager&, EStateMsg, float);
  void Attack(CStateManager&, EStateMsg, float);
};
} // namespace urde::MP1