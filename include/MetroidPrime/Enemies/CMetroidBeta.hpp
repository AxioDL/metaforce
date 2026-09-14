#ifndef _CMETROIDBETA
#define _CMETROIDBETA

#include "types.h"

#include "Collision/CCollidableAABox.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/single_ptr.hpp"

class CCollisionActorManager;
class CJointCollisionDescription;
struct SSphereJointInfo;
class CElementGen;
class CGenDescription;
class CParticleSwoosh;
class CSwooshDescription;

class CMetroidBetaData {
public:
  CMetroidBetaData(CInputStream& in, int propCount);

  const CDamageVulnerability& GetFrozenVulnerability() const { return x0_frozenVulnerability; }
  const CDamageVulnerability& GetEnergyDrainVulnerability() const {
    return x68_energyDrainVulnerability;
  }
  float GetEnergyDrainPerSec() const { return xd0_energyDrainPerSec; }
  float GetMaxEnergyDrainAllowed() const { return xd4_maxEnergyDrainAllowed; }
  float GetSpecialAttackTime() const { return xe0_specialAttackTime; }
  float GetTelegraphAttackTime() const { return xdc_telegraphAttackTime; }
  float GetStage2GrowthEnergy() const { return xec_stage2GrowthEnergy; }
  float GetStage3GrowthEnergy() const { return xf0_stage3GrowthEnergy; }
  float GetMaxGrowthScale() const { return xe8_maxGrowthScale; }
  float GetGrowthEnergy() const { return xec_stage2GrowthEnergy; }
  float GetBreakLeashEnergyDrain() const { return xd8_breakLeashEnergyDrain; }
  CAssetId GetElectricParticleId() const { return xf4_electricParticleId; }
  CAssetId GetElectricSwooshEffectId() const { return xf8_electricSwooshEffectId; }
  CAssetId GetElectricMorphBallHitEffectId() const { return xfc_electricMorphBallHitEffectId; }
  CAssetId GetElectricGunHitEffectId() const { return x100_electricGunHitEffectId; }
  CAssetId GetElectricGunFeedbackId() const { return x104_electricGunFeedbackId; }

  bool StartsInWall() const { return x108_24_startsInWall; }

  static int GetNumProperties() { return skNumProperties; }

private:
  CDamageVulnerability x0_frozenVulnerability;
  CDamageVulnerability x68_energyDrainVulnerability;
  float xd0_energyDrainPerSec;
  float xd4_maxEnergyDrainAllowed;
  float xd8_breakLeashEnergyDrain;
  float xdc_telegraphAttackTime;
  float xe0_specialAttackTime;
  float xe4_;
  float xe8_maxGrowthScale;
  float xec_stage2GrowthEnergy;
  float xf0_stage3GrowthEnergy;
  CAssetId xf4_electricParticleId;
  CAssetId xf8_electricSwooshEffectId;
  CAssetId xfc_electricMorphBallHitEffectId;
  CAssetId x100_electricGunHitEffectId;
  CAssetId x104_electricGunFeedbackId;
  bool x108_24_startsInWall : 1;

  static const uint skNumProperties;
};
CHECK_SIZEOF(CMetroidBetaData, 0x10C)

class CMetroidData;

class CMetroidBeta : public CPatterned {
public:
  CMetroidBeta(const TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
               const CTransform4f& xf, const CModelData& mData, const CPatternedInfo& pInfo,
               const CActorParameters& aParms, const CMetroidBetaData& metroidData);
  ~CMetroidBeta();

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void AddToRenderer(const CFrustumPlanes&, const CStateManager&) const override;
  void Render(const CStateManager&) const override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  const CDamageVulnerability* GetDamageVulnerability(const CVector3f& pos, const CVector3f& dir,
                                                     const CDamageInfo& dInfo) const override;
  void Touch(CActor& act, CStateManager& mgr) override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;
  bool IsListening() const override { return true; }
  const CCollisionPrimitive* GetCollisionPrimitive() const override { return &x768_colPrim; }
  CPathFindSearch* GetSearchPath() override { return &x67c_pathFind; }
  void CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                    CStateManager& mgr) override;
  CVector3f GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                      const CVector3f& aimPos) const override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f&, const CVector3f&,
                                                         const CWeaponMode&, int) const override;

  void Patrol(CStateManager& mgr, EStateMsg msg, float dt) override;
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt) override;
  void SelectTarget(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Generate(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Attack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TurnAround(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TelegraphAttack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void WallHang(CStateManager& mgr, EStateMsg msg, float dt) override;
  void SpecialAttack(CStateManager& mgr, EStateMsg msg, float dt) override;

  bool InAttackPosition(CStateManager& mgr, float arg) override;
  bool Attacked(CStateManager& mgr, float arg) override;
  bool PathShagged(CStateManager& mgr, float arg) override;
  bool InDetectionRange(CStateManager& mgr, float arg) override;
  bool AnimOver(CStateManager& mgr, float arg) override;
  bool ShouldAttack(CStateManager& mgr, float arg) override;
  bool InPosition(CStateManager& mgr, float arg) override;
  bool ShouldTurn(CStateManager& mgr, float arg) override;
  bool AttackOver(CStateManager& mgr, float arg) override;
  bool ShotAt(CStateManager& mgr, float arg) override;
  bool ShouldWallHang(CStateManager& mgr, float arg) override;
  bool StartAttack(CStateManager& mgr, float arg) override;
  bool BreakAttack(CStateManager& mgr, float arg) override;
  bool ShouldSpecialAttack(CStateManager& mgr, float arg) override;

  void RenderHitGunEffect() const;
  void RenderHitBallEffect() const;

private:
  void SwarmAdd(CStateManager& mgr);
  void SwarmRemove(CStateManager& mgr);
  void SetupHealthInfo(CStateManager& mgr);
  void SetupCollisionManager(CStateManager& mgr);
  void AddSphereCollisionList(const SSphereJointInfo* joints, int count,
                              rstl::vector< CJointCollisionDescription >& list);
  void UpdateHealthInfo(CStateManager& mgr);
  void UpdateAILogicTimers(float dt, CStateManager& mgr);
  void UpdateParticleEffects(float dt, CStateManager& mgr);
  void UpdateHitTargetParticleEffect(float dt, CStateManager& mgr, const CVector3f& target);
  CVector3f GetTentacleSourcePos() const;
  CVector3f ComputeTargetPos(CStateManager& mgr) const;
  void UpdateSoundVolume();
  void AdjustPathFindSteering(CStateManager& mgr);
  void ApplyForwardSteering(CStateManager& mgr, const CVector3f& destination);
  void ApplySeparationBehavior(CStateManager& mgr);
  void UpdateModelScale(const CVector3f& scale);
  void ApplyBreakLeashDamage(CStateManager& mgr);
  void SuckEnergyFromTarget(float dt, CStateManager& mgr);
  bool ShouldReleaseFromTarget(CStateManager& mgr);
  void ApplyGrowth(float amount, CStateManager& mgr);
  float GetGrowthStage() const;
  float GetDamageMultiplier() const;
  void ExtendTouchBounds(CStateManager& mgr, const CVector3f& bounds);
  bool IsSuckingEnergy() const;

  int x568_progState;
  CMetroidBetaData x56c_data;
  TUniqueId x678_teamMgr;
  CPathFindSearch x67c_pathFind;
  CSegId x760_leftClaw;
  CSegId x761_rightClaw;
  rstl::single_ptr< CCollisionActorManager > x764_collisionManager;
  CCollidableAABox x768_colPrim;
  TUniqueId x790_pelvisCollisionId;
  float x794_attackCooldown;
  float x798_telegraphTimer;
  float x79c_drainedEnergy;
  float x7a0_interferenceTime;
  float x7a4_specialAttackTime;
  CVector3f x7a8_targetPos;
  CVector3f x7b4_growthScale;
  CVector3f x7c0_generateStartScale;
  CVector3f x7cc_initialScale;
  float x7d8_generateDuration;
  float x7dc_growthEnergy;
  float x7e0_previousGrowthEnergy;
  TLockedToken< CGenDescription > x7e4_;
  TLockedToken< CSwooshDescription > x7f0_;
  TLockedToken< CGenDescription > x7fc_;
  TLockedToken< CGenDescription > x808_;
  TLockedToken< CGenDescription > x814_;
  rstl::single_ptr< CElementGen > x820_;
  rstl::single_ptr< CParticleSwoosh > x824_;
  rstl::single_ptr< CElementGen > x828_;
  rstl::single_ptr< CElementGen > x82c_;
  rstl::single_ptr< CElementGen > x830_;
  float x834_particlePhase;
  CRandom16 x838_random;
  float x83c_shotSeverity;
  bool x840_24_ : 1;
  bool x840_25_ : 1;
  bool x840_26_ : 1;
  bool x840_27_ : 1;
  bool x840_28_ : 1;
  bool x840_29_ : 1;
  bool x840_30_ : 1;
  bool x840_31_ : 1;
};
CHECK_SIZEOF(CMetroidBeta, (VERSION >= VERSION_GM8P_00 ? 0x858 : 0x848))

#endif // _CMETROIDBETA
