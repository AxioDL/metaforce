#pragma once

#include "Runtime/World/CPatterned.hpp"

namespace urde::MP1 {
class CJellyZap : public CPatterned {
  u32 x568_ = 0;
  CDamageInfo x56c_attackDamage;
  float x588_attackRadius;
  float x58c_;
  float x590_;
  float x594_;
  float x598_;
  float x59c_priority;
  float x5a0_repulseRadius;
  float x5a4_attractRadius;
  float x5a8_attackDelay;
  float x5ac_;
  float x5b0_;
  float x5b4_;
  bool x5b8_24_ : 1 = false;
  bool x5b8_25_ : 1 = false;
  bool x5b8_26_ : 1;

  void AddSelfToFishCloud(CStateManager&, float, float, bool);
  void AddRepulsor(CStateManager&);
  void AddAttractor(CStateManager&);
  void RemoveSelfFromFishCloud(CStateManager&);
  void RemoveAllAttractors(CStateManager&);
  bool ClosestToPlayer(const CStateManager&) const;
  bool sub801d8190() const { return x568_ != 1; }

public:
  DEFINE_PATTERNED(JellyZap)

  CJellyZap(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&, const CDamageInfo&,
            bool, float, float, float, float, float, float, float, float, float, float, float, float,
            const CPatternedInfo&, const CActorParameters&);

  void Accept(IVisitor&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void Think(float, CStateManager&) override;
  void DoUserAnimEvent(CStateManager&, const CInt32POINode&, EUserEventType, float dt) override;
  void KnockBack(const zeus::CVector3f &, CStateManager &, const CDamageInfo &info, EKnockBackType type, bool inDeferred, float magnitude) override;
  const CDamageVulnerability* GetDamageVulnerability() const override { return CAi::GetDamageVulnerability(); }
  const CDamageVulnerability* GetDamageVulnerability(const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                                                     const CDamageInfo& info) const override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&,
                                                         const CWeaponMode&, EProjectileAttrib) const override {
    return sub801d8190() ? EWeaponCollisionResponseTypes::Unknown89 : EWeaponCollisionResponseTypes::Unknown39;
  }
  void Attack(CStateManager&, EStateMsg, float) override;
  void Suck(CStateManager&, EStateMsg, float) override;
  void Active(CStateManager&, EStateMsg, float) override;
  void InActive(CStateManager&, EStateMsg, float) override;
  void Flinch(CStateManager&, EStateMsg, float) override;
  bool ShouldAttack(CStateManager&, float) override { return x330_stateMachineState.GetTime() > x5a8_attackDelay; }
  bool ShouldSpecialAttack(CStateManager& mgr, float) override { return ClosestToPlayer(mgr); }
  bool InAttackPosition(CStateManager&, float) override;
  bool InDetectionRange(CStateManager&, float) override;
};
} // namespace urde::MP1