#pragma once

#include "Runtime/World/CPatterned.hpp"
#include "Runtime/Particle/CParticleElectric.hpp"

namespace urde::MP1 {
class CPhazonHealingNodule : public CPatterned {
private:
  int x568_active = 0;
  bool x56c_emitting = false;
  TUniqueId x56e_connId = kInvalidUniqueId;
  TCachedToken<CElectricDescription> x570_electricDesc;
  std::unique_ptr<CParticleElectric> x57c_particleElectric; // was rstl::rc_ptr
  CHealthInfo x580_initialHealthInfo;
  int x588_state = 0; // not init in ctr; same as CElitePirate::EState?
  std::string x58c_actorLctr;
  // u32 x59c_;

public:
  DEFINE_PATTERNED(PhazonHealingNodule);

  CPhazonHealingNodule(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                       CModelData&& mData, const CActorParameters& actParams, const CPatternedInfo& pInfo,
                       CAssetId particleDescId, std::string actorLctr);

  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) override;
  void Render(CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;

  void Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state) override;
  void Faint(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Growth(CStateManager& mgr, EStateMsg msg, float dt) override;
  void KnockBack(const zeus::CVector3f& vec, CStateManager& mgr, const CDamageInfo& info, EKnockBackType type,
                 bool inDeferred, float magnitude) override;
  void Lurk(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt) override;

  bool InRange(CStateManager &, float arg) override;
  bool InDetectionRange(CStateManager &, float arg) override;
  bool AnimOver(CStateManager &, float arg) override;

  void MassiveDeath(CStateManager &mgr) override;
  void MassiveFrozenDeath(CStateManager &mgr) override;
  void PhazeOut(CStateManager &) override;

private:
  void UpdateParticleElectric(CStateManager& mgr);
};
} // namespace urde::MP1
