#ifndef _CMAGDOLITE
#define _CMAGDOLITE

#include "types.h"

#include "MetroidPrime/CBoneTracking.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Weapons/CFlameInfo.hpp"

#include "Kyoto/TToken.hpp"

#include "rstl/rc_ptr.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CCollisionActorManager;
class CSkinnedModel;
class CWeaponDescription;

class CMagdolite : public CPatterned {
public:
  CMagdolite(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
             const CTransform4f& xf, const CModelData& mData, const CPatternedInfo& pInfo,
             const CActorParameters& actParms, float headTrackAngle, float losAngle,
             const CDamageInfo& flameThrowerDamage, const CDamageInfo& headContactDamage,
             const CDamageVulnerability& headVuln, const CDamageVulnerability& bodyVuln,
             CAssetId headlessModel, CAssetId headlessSkin, float minHp, float initialDelay,
             float minDelay, float maxDelay, const CFlameInfo& flameInfo, float f7, float f8,
             float f9);

  // CEntity
  ~CMagdolite() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CActor
  void Render(const CStateManager& mgr) const override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  const CDamageVulnerability* GetDamageVulnerability(const CVector3f&, const CVector3f&,
                                                     const CDamageInfo&) const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  void FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr) override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CAi
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;
  void KnockBack(const CVector3f&, CStateManager&, const CDamageInfo& info, float magnitude,
                 bool direct, const bool inDeferred) override;

  // CPatterned (state functions)
  void SelectTarget(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Generate(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Deactivate(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Attack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Active(CStateManager& mgr, EStateMsg msg, float arg) override;
  void InActive(CStateManager& mgr, EStateMsg msg, float arg) override;
  void GetUp(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Taunt(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Lurk(CStateManager& mgr, EStateMsg msg, float arg) override;
  void ProjectileAttack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Flinch(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Retreat(CStateManager& mgr, EStateMsg msg, float arg) override;

  // CPatterned (trigger functions)
  bool InAttackPosition(CStateManager& mgr, float arg) override;
  bool Leash(CStateManager& mgr, float arg) override;
  bool HasAttackPattern(CStateManager& mgr, float arg) override;
  bool LineOfSight(CStateManager& mgr, float arg) override;
  bool ShouldRetreat(CStateManager& mgr, float arg) override;

private:
  void DoContactDamage(TUniqueId uid, CStateManager& mgr);
  void CreateCollisionActors(CStateManager& mgr);
  void CreateFlameThrower(CStateManager& mgr);
  void SetFlameThrowerActive(CStateManager& mgr, bool fire);
  void SnapToFacePlayer(CStateManager& mgr);
  TUniqueId GetBestConnectedObject(CStateManager& mgr, EScriptObjectState state,
                                   EScriptObjectMessage msg);

  float x568_initialDelay;
  float x56c_minDelay;
  float x570_maxDelay;
  float x574_minHp;
  float x578_losMaxDistance;
  float x57c_headTrackAngle;
  rstl::single_ptr< CCollisionActorManager > x580_collisionManager;
  CBoneTracking x584_boneTracker;
  CDamageVulnerability x5bc_instaKillVulnerability;
  CDamageVulnerability x624_normalVulnerability;
  rstl::ncrc_ptr< CModelData > x68c_;
  TLockedToken< CSkinnedModel > x690_headlessModel;
  rstl::reserved_vector< TUniqueId, 4 > x69c_headCollisionActors;
  CFlameInfo x6a8_flameInfo;
  TUniqueId x6c8_flameThrowerId;
  TToken< CWeaponDescription > x6cc_flameThrowerDesc;
  CDamageInfo x6d4_flameThrowerDamage;
  CDamageInfo x6f0_headContactDamage;
  float x70c_curHealth;
  CVector3f x710_attackOffset;
  CVector3f x71c_attackTarget;
  CVector3f x728_cachedTarget;
  float x734_;
  float x738_;
  float x73c_;
  float x740_;
  float x744_;
  float x748_;
  float x74c_;
  uint x750_aiStage;
  bool x754_24_retreat : 1;
  bool x754_25_up : 1;
  bool x754_26_lostMyHead : 1;
  bool x754_27_flameThrowerActive : 1;
  bool x754_28_alert : 1;
  bool x754_29_useDetectionRange : 1;
  bool x754_30_inProjectileAttack : 1;
  float x758_;
};
CHECK_SIZEOF(CMagdolite, (VERSION >= VERSION_GM8P_00 ? 0x770 : 0x760))

#endif // _CMAGDOLITE
