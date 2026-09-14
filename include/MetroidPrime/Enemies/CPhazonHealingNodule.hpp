#ifndef _CPHAZONHEALINGNODULE
#define _CPHAZONHEALINGNODULE

#include "types.h"

#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"

#include "Kyoto/TToken.hpp"

#include "rstl/rc_ptr.hpp"
#include "rstl/string.hpp"

class CParticleElectric;
class CElectricDescription;

class CPhazonHealingNodule : public CPatterned {
public:
  CPhazonHealingNodule(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                       const CTransform4f& xf, const CModelData& mData,
                       const CActorParameters& actParams, const CPatternedInfo& pInfo,
                       CAssetId particleDescId, const rstl::string& actorLctr);

  // CEntity
  ~CPhazonHealingNodule() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CActor
  void Render(const CStateManager& mgr) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CAi
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;
  void KnockBack(const CVector3f&, CStateManager&, const CDamageInfo& info, float magnitude,
                 bool direct, const bool inDeferred) override;

  // State functions
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Lurk(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Growth(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Faint(CStateManager& mgr, EStateMsg msg, float dt) override;

  // State predicates
  bool InRange(CStateManager& mgr, float arg) override;
  bool InDetectionRange(CStateManager& mgr, float arg) override;
  bool AnimOver(CStateManager& mgr, float arg) override;

  // CPatterned
  void MassiveDeath(CStateManager& mgr) override;
  void MassiveFrozenDeath(CStateManager& mgr) override;
  void PhazeOut(CStateManager& mgr) override;

private:
  int x568_active;
  bool x56c_emitting;
  // 1 byte pad
  TUniqueId x56e_connId;
  TLockedToken< CElectricDescription > x570_electricDesc;
  rstl::ncrc_ptr< CParticleElectric > x57c_particleElectric;
  CHealthInfo x580_initialHealthInfo;
  int x588_state;
  rstl::string x58c_actorLctr;
  uint x59c_;

  void UpdateParticleElectric(CStateManager& mgr);
};
CHECK_SIZEOF(CPhazonHealingNodule, (VERSION >= VERSION_GM8P_00 ? 0x5b0 : 0x5A0))

#endif // _CPHAZONHEALINGNODULE
