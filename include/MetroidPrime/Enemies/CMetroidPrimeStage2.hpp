#ifndef _CMETROIDPRIMESTAGE2
#define _CMETROIDPRIMESTAGE2

#include "types.h"

#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/single_ptr.hpp"

class CCollisionActorManager;
class CElementGen;
class CGenDescription;
struct SSphereJointInfo;
class CJointCollisionDescription;
class CShockWaveInfo;
class CRayCastResult;

class CMetroidPrimeStage2 : public CPatterned {
public:
  CMetroidPrimeStage2(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                      const CTransform4f& xf, const CModelData& mData, const CPatternedInfo& pInfo,
                      const CActorParameters& actParms, CAssetId particle1, CDamageInfo dInfo,
                      CAssetId electric, uint sfxId, CAssetId particle2);

  // CEntity
  ~CMetroidPrimeStage2() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CActor
  void PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) override;
  void AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CAi
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;

  // State functions
  void Dead(CStateManager& mgr, EStateMsg msg, float dt) override;
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Halt(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Generate(CStateManager& mgr, EStateMsg msg, float dt) override;
  void JumpBack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Skid(CStateManager& mgr, EStateMsg msg, float dt) override;
  void FadeIn(CStateManager& mgr, EStateMsg msg, float dt) override;
  void FadeOut(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Taunt(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TelegraphAttack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Dodge(CStateManager& mgr, EStateMsg msg, float dt) override;
  void PathFindEx(CStateManager& mgr, EStateMsg msg, float dt) override;

  // State predicates
  bool HasPatrolPath(CStateManager& mgr, float arg) override;
  bool ShouldAttack(CStateManager& mgr, float arg) override;
  bool InPosition(CStateManager& mgr, float arg) override;
  bool CoverFind(CStateManager& mgr, float arg) override;
  bool ShouldTaunt(CStateManager& mgr, float arg) override;
  bool ShouldCrouch(CStateManager& mgr, float arg) override;
  bool ShouldMove(CStateManager& mgr, float arg) override;

  // CPatterned
  CPathFindSearch* GetSearchPath() override { return &mSearchPath; }

private:
  bool CanSummonToPosition(const CTransform4f& xf, CStateManager& mgr);
  void PlayPainSound(CStateManager& mgr);
  void UpdateNumActiveMetroids(CStateManager& mgr);
  void UpdateVisibleSpectrum(float dt, CStateManager& mgr);
  CRayCastResult GetGroundContactPoint(CStateManager& mgr);
  void SpawnPhazonPool(CStateManager& mgr);
  void BlastShake(float magnitude, CStateManager& mgr);
  void CreateShockWave(CStateManager& mgr, const CShockWaveInfo& info);
  void KillActiveMetroids(CStateManager& mgr);
  void UpdateHealthInfo(CStateManager& mgr);
  void SetupHealthInfo(CStateManager& mgr);
  void SetupCollisionManager(CStateManager& mgr);
  void UpdateSummonType(CStateManager& mgr);
  void AddSphereCollisionList(const SSphereJointInfo* joints, int count,
                              rstl::vector< CJointCollisionDescription >& descs);
  const CTransform4f& GetCreatureTransform(CStateManager& mgr) const;
  void StepToPosition(const CVector3f& position);
  uint GetAvoidanceStep(CStateManager& mgr, bool allowBackward);
  void StartSpectralFade(CStateManager& mgr);
  int GetMaxSpawnCount(CStateManager& mgr);
  void UpdateMaterialSet(float t);
  void ActivateHeadFx(CStateManager& mgr, bool active);

  TLockedToken< CGenDescription > mParticleDesc;
  CPathFindSearch mSearchPath;
  rstl::single_ptr< CCollisionActorManager > mCollisionManager;
  rstl::single_ptr< CElementGen > mElementGen;
  CAssetId mParticle1;
  CAssetId mElectric;
  CTransform4f mSpawnXf;
  CDamageInfo mDamageInfo;
  CVector3f mSpaceWarpPos;
  float mHpThreshold;
  float mHpFraction;
  float mPhaseAlpha;
  float mBaseScale;
  float mMaxScale;
  float mMorphT;
  int mDamagePhase;
  int mCurrentVisorPhase;
  int mPreviousVisorPhase;
  int mSpawnedAiCount;
  int mMinAttackInterval;
  int mMaxAttackInterval;
  int mAttackCounter;
  int mNextAttackThreshold;
  int mMaxSpawnedCount;
  int mMaterialSetIdx;
  uint mStepDirection;
  TUniqueId mBossUtilityWaypointId;
  TUniqueId mLockOnTargetCollider;
  CSfxHandle mSfxHandle;
  ushort mSfxId;
  bool mIsProjectileAttacking : 1;
  bool mCanAttack : 1;
  bool mIsPhaseTransitioning : 1;
  bool mIsSpaceWarping : 1;
  bool mIsVisible : 1;
  bool mCanSpawnAi : 1;
  bool mIsMorphing : 1;
  bool mHasEventStarted : 1;
};
CHECK_SIZEOF(CMetroidPrimeStage2, (VERSION >= VERSION_GM8E_02 ? 0x720 : 0x710))

#endif // _CMETROIDPRIMESTAGE2
