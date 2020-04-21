#pragma once

#include <memory>
#include <string_view>
#include <vector>

#include "Runtime/CToken.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/Weapon/CProjectileInfo.hpp"
#include "Runtime/World/CPatterned.hpp"

namespace urde::MP1 {
class CPuddleSpore : public CPatterned {
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
  bool x614_24 : 1 = false;
  bool x614_25 : 1 = false;

  bool HitShell(const zeus::CVector3f&) const;
  void KnockPlayer(CStateManager&, float);
  void UpdateBoundingState(const zeus::CAABox&, CStateManager&, float);

public:
  DEFINE_PATTERNED(PuddleSpore)

  CPuddleSpore(TUniqueId, std::string_view, EFlavorType, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
               const CPatternedInfo&, EColliderType, CAssetId, float, float, float, float, float,
               const CActorParameters&, CAssetId, const CDamageInfo&);

  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void PreThink(float, CStateManager&) override;
  void Think(float, CStateManager&) override;
  void Render(CStateManager&) override;
  void Touch(CActor&, CStateManager&) override;
  void FluidFXThink(EFluidState, CScriptWater&, CStateManager&) override;
  void KnockBack(const zeus::CVector3f& dir, CStateManager& mgr, const CDamageInfo& dInfo, EKnockBackType type,
                 bool inDeferred, float dt) override {
    if (x5c8_ == 1)
      return;
    CPatterned::KnockBack(dir, mgr, dInfo, type, inDeferred, dt);
  }
  EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f& point, const zeus::CVector3f&,
                                                         const CWeaponMode&, EProjectileAttrib) const override {
    return HitShell(point) ? EWeaponCollisionResponseTypes::Unknown84 : EWeaponCollisionResponseTypes::Unknown34;
  }
  void DoUserAnimEvent(CStateManager&, const CInt32POINode&, EUserEventType, float) override;
  void CollidedWith(TUniqueId uid, const CCollisionInfoList& colList, CStateManager& mgr) override {
    if (x5c8_ == 2)
      return;
    CPatterned::CollidedWith(uid, colList, mgr);
  }
  const CCollisionPrimitive* GetCollisionPrimitive() const override { return &x5a0_; }
  zeus::CAABox CalculateBoundingBox() const;
  CProjectileInfo* GetProjectileInfo() override { return &x5ec_projectileInfo; }

  bool InAttackPosition(CStateManager&, float) override { return x568_ >= x570_; }
  bool ShouldAttack(CStateManager&, float) override { return x568_ >= x574_; }
  bool ShouldTurn(CStateManager&, float) override;
  bool AnimOver(CStateManager&, float) override { return x5cc_ == 2; }

  void InActive(CStateManager&, EStateMsg, float) override;
  void Active(CStateManager&, EStateMsg, float) override;
  void Run(CStateManager&, EStateMsg, float) override;
  void TurnAround(CStateManager&, EStateMsg, float) override;
  void GetUp(CStateManager&, EStateMsg, float) override;
  void Attack(CStateManager&, EStateMsg, float) override;
};
} // namespace urde::MP1