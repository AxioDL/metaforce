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
  CPathFindSearch* GetSearchPath() override { return &x574_searchPath; }

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

  TLockedToken< CGenDescription > x568_particleDesc;
  CPathFindSearch x574_searchPath;
  rstl::single_ptr< CCollisionActorManager > x658_collisionManager;
  rstl::single_ptr< CElementGen > x65c_elementGen;
  CAssetId x660_particle1;
  CAssetId x664_electric;
  CTransform4f x668_spawnXf;
  CDamageInfo x698_damageInfo;
  CVector3f x6b4_spaceWarpPos;
  float x6c0_hpThreshold;
  float x6c4_hpFraction;
  float x6c8_phaseAlpha;
  float x6cc_baseScale;
  float x6d0_maxScale;
  float x6d4_morphT;
  int x6d8_damagePhase;
  int x6dc_currentVisorPhase;
  int x6e0_previousVisorPhase;
  int x6e4_spawnedAiCount;
  int x6e8_minAttackInterval;
  int x6ec_maxAttackInterval;
  int x6f0_attackCounter;
  int x6f4_nextAttackThreshold;
  int x6f8_maxSpawnedCount;
  int x6fc_materialSetIdx;
  uint x700_stepDirection;
  TUniqueId x704_bossUtilityWaypointId;
  TUniqueId x706_lockOnTargetCollider;
  CSfxHandle x708_sfxHandle;
  ushort x70c_sfxId;
  bool x70e_24_isProjectileAttacking : 1;
  bool x70e_25_canAttack : 1;
  bool x70e_26_isPhaseTransitioning : 1;
  bool x70e_27_isSpaceWarping : 1;
  bool x70e_28_isVisible : 1;
  bool x70e_29_canSpawnAi : 1;
  bool x70e_30_isMorphing : 1;
  bool x70e_31_hasEventStarted : 1;
};
CHECK_SIZEOF(CMetroidPrimeStage2, (VERSION >= VERSION_GM8P_00 ? 0x720 : 0x710))

#endif // _CMETROIDPRIMESTAGE2
