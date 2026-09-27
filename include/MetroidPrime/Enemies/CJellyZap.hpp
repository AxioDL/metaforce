#ifndef _CJELLYZAP
#define _CJELLYZAP

#include "MetroidPrime/Enemies/CPatterned.hpp"

class CJellyZap : public CPatterned {
public:
  CJellyZap(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
            const CTransform4f& xf, const CModelData& mData, const CDamageInfo& attackDamage,
            bool b1, float attackRadius, float f2, float f3, float f4, float attackDelay, float f6,
            float f7, float f8, float priority, float repulseRadius, float attractRadius, float f12,
            const CPatternedInfo& pInfo, const CActorParameters& actParms);

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  bool InDetectionRange(CStateManager& mgr, float arg) override;
  bool InAttackPosition(CStateManager& mgr, float arg) override;
  bool ShouldSpecialAttack(CStateManager& mgr, float arg) override;
  bool ShouldAttack(CStateManager& mgr, float arg) override;
  void InActive(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Active(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Suck(CStateManager& mgr, EStateMsg msg, float arg) override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f&, const CVector3f&,
                                                         const CWeaponMode&, int) const override;
  const CDamageVulnerability* GetDamageVulnerability(const CVector3f& pos, const CVector3f& dir,
                                                     const CDamageInfo& info) const override;
  void Attack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Flinch(CStateManager& mgr, EStateMsg msg, float arg) override;
  void KnockBack(const CVector3f&, CStateManager&, const CDamageInfo& info, float magnitude,
                 bool direct, const bool inDeferred) override;

private:
  void AddAttractor(CStateManager& mgr);
  void RemoveAllAttractors(CStateManager& mgr);
  void AddRepulsor(CStateManager& mgr);
  void AddSelfToFishCloud(CStateManager& mgr, float radius, float priority, bool repulsor);
  void RemoveSelfFromFishCloud(CStateManager& mgr);
  bool ClosestToPlayer(CStateManager& mgr) const;
  bool HitShell(const CVector3f&) const;
  uint x568_;
  CDamageInfo mAttackDamage;
  float mAttackRadius;
  float x58c_;
  float x590_;
  float x594_;
  float x598_;
  float mPriority;
  float mRepulseRadius;
  float mAttractRadius;
  float mAttackDelay;
  float x5ac_;
  float x5b0_;
  float x5b4_;
  bool x5b8_24_ : 1;
  bool x5b8_25_ : 1;
  bool x5b8_26_ : 1;
};
CHECK_SIZEOF(CJellyZap, (VERSION >= VERSION_GM8E_02 ? 0x5d0 : 0x5c0))

#endif // _CJELLYZAP
