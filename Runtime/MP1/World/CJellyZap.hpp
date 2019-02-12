#pragma once

#include "World/CPatterned.hpp"

namespace urde::MP1 {
class CJellyZap : public CPatterned {
  static const CMaterialFilter kPlayerFilter;
  u32 x568_ = 0;
  CDamageInfo x56c_attackDamage;
  float x588_attackRadius;
  float x58c_;
  float x590_;
  float x594_;
  float x598_;
  float x59c_;
  float x5a0_;
  float x5a4_;
  float x5a8_attackDelay;
  float x5ac_;
  float x5b0_;
  float x5b4_;
  bool x5b8_24_ : 1;
  bool x5b8_25_ : 1;
  bool x5b8_26_ : 1;

  void AddSelfToFishCloud(CStateManager, float, bool);
  void AddRepulsor(CStateManager&);
  void AddAttractor(CStateManager&);
  void RemoveSelfFromFishCloud(CStateManager&);
  void RemoveAllAttractors(CStateManager&);
  bool ClosestToPlayer(CStateManager&);

public:
  DEFINE_PATTERNED(JellyZap)

  CJellyZap(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&, const CDamageInfo&,
            bool, float, float, float, float, float, float, float, float, float, float, float, float,
            const CPatternedInfo&, const CActorParameters&);

  void Accept(IVisitor&);
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
  void Think(float, CStateManager&);
  void DoUserAnimEvent(CStateManager&, const CInt32POINode&, EUserEventType, float dt);
  void Attack(CStateManager&, EStateMsg, float);
  void Suck(CStateManager&, EStateMsg, float);
  void Active(CStateManager&, EStateMsg, float);
  void InActive(CStateManager&, EStateMsg, float);
  void Flinch(CStateManager&, EStateMsg, float);
  bool ShouldAttack(CStateManager&, float) { return x330_stateMachineState.GetTime() > x5a8_attackDelay; }
  bool ShouldSpecialAttack(CStateManager& mgr, float) { return ClosestToPlayer(mgr); }
  bool InAttackPosition(CStateManager&, float);
  bool InDetectionRange(CStateManager&, float);
};
} // namespace urde::MP1