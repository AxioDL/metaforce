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

  const CDamageVulnerability& GetFrozenVulnerability() const { return mFrozenVulnerability; }
  const CDamageVulnerability& GetEnergyDrainVulnerability() const {
    return mEnergyDrainVulnerability;
  }
  float GetEnergyDrainPerSec() const { return mEnergyDrainPerSec; }
  float GetMaxEnergyDrainAllowed() const { return mMaxEnergyDrainAllowed; }
  float GetSpecialAttackTime() const { return mSpecialAttackTime; }
  float GetTelegraphAttackTime() const { return mTelegraphAttackTime; }
  float GetStage2GrowthEnergy() const { return mStage2GrowthEnergy; }
  float GetStage3GrowthEnergy() const { return mStage3GrowthEnergy; }
  float GetMaxGrowthScale() const { return mMaxGrowthScale; }
  float GetGrowthEnergy() const { return mStage2GrowthEnergy; }
  float GetBreakLeashEnergyDrain() const { return mBreakLeashEnergyDrain; }
  CAssetId GetElectricParticleId() const { return mElectricParticleId; }
  CAssetId GetElectricSwooshEffectId() const { return mElectricSwooshEffectId; }
  CAssetId GetElectricMorphBallHitEffectId() const { return mElectricMorphBallHitEffectId; }
  CAssetId GetElectricGunHitEffectId() const { return mElectricGunHitEffectId; }
  CAssetId GetElectricGunFeedbackId() const { return mElectricGunFeedbackId; }

  bool StartsInWall() const { return mStartsInWall; }

  static int GetNumProperties() { return skNumProperties; }

private:
  CDamageVulnerability mFrozenVulnerability;
  CDamageVulnerability mEnergyDrainVulnerability;
  float mEnergyDrainPerSec;
  float mMaxEnergyDrainAllowed;
  float mBreakLeashEnergyDrain;
  float mTelegraphAttackTime;
  float mSpecialAttackTime;
  float xe4_;
  float mMaxGrowthScale;
  float mStage2GrowthEnergy;
  float mStage3GrowthEnergy;
  CAssetId mElectricParticleId;
  CAssetId mElectricSwooshEffectId;
  CAssetId mElectricMorphBallHitEffectId;
  CAssetId mElectricGunHitEffectId;
  CAssetId mElectricGunFeedbackId;
  bool mStartsInWall : 1;

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
  const CCollisionPrimitive* GetCollisionPrimitive() const override { return &mColPrim; }
  CPathFindSearch* GetSearchPath() override { return &mPathFind; }
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

  int mProgState;
  CMetroidBetaData mData;
  TUniqueId mTeamMgr;
  CPathFindSearch mPathFind;
  CSegId mLeftClaw;
  CSegId mRightClaw;
  rstl::single_ptr< CCollisionActorManager > mCollisionManager;
  CCollidableAABox mColPrim;
  TUniqueId mPelvisCollisionId;
  float mAttackCooldown;
  float mTelegraphTimer;
  float mDrainedEnergy;
  float mInterferenceTime;
  float mSpecialAttackTime;
  CVector3f mTargetPos;
  CVector3f mGrowthScale;
  CVector3f mGenerateStartScale;
  CVector3f mInitialScale;
  float mGenerateDuration;
  float mGrowthEnergy;
  float mPreviousGrowthEnergy;
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
  float mParticlePhase;
  CRandom16 mRandom;
  float mShotSeverity;
  bool x840_24_ : 1;
  bool x840_25_ : 1;
  bool x840_26_ : 1;
  bool x840_27_ : 1;
  bool x840_28_ : 1;
  bool x840_29_ : 1;
  bool x840_30_ : 1;
  bool x840_31_ : 1;
};
CHECK_SIZEOF(CMetroidBeta, (VERSION >= VERSION_GM8E_02 ? 0x858 : 0x848))

#endif // _CMETROIDBETA
