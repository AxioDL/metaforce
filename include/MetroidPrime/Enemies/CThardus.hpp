#ifndef _CTHARDUS
#define _CTHARDUS

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "MetroidPrime/Collision/CPatternedCollisionUtils.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"

#include "Kyoto/Math/CVector2f.hpp"

class CJointCollisionDescription;

class CCollisionActorManager;

class CDestroyableRock : public CAi {
public:
  CDestroyableRock(TUniqueId id, bool active, const rstl::string& name, const CEntityInfo& info,
                   const CTransform4f& xf, const CModelData& modelData, float mass,
                   const CHealthInfo& health, const CDamageVulnerability& vulnerability,
                   const CMaterialList& matList, CAssetId fsm, const CActorParameters& actParams,
                   const CModelData& phazonModel, int w1);

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) override;
  void Render(const CStateManager& mgr) const override;
  bool CanRenderUnsorted(const CStateManager&) const override { return true; }
  CVector3f GetAimPosition(const CStateManager&, float) const override { return GetTranslation(); }
  CVector3f GetOrbitPosition(const CStateManager&) const override { return GetTranslation(); }
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Death(CStateManager&, const CVector3f&, EScriptObjectState) override { x334_isCold = true; }
  void KnockBack(const CVector3f&, CStateManager&, const CDamageInfo&, float, bool,
                 const bool) override {}
  void TakeDamage(const CVector3f&, float) override {
    x324_ = 1.f;
    x328_ = 2.f;
  }
  float GetDamageFlashTimer() const { return x324_; }
  void UsePhazonModel();
  void SetThermalMag(float mag) { x32c_thermalMag = mag; }
  void SetX340(bool value) { x340_ = value; }
  bool IsUsingPhazonModel() const { return x335_usePhazonModel; }
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;

private:
  CModelData x2d8_phazonModel;
  float x324_;
  float x328_;
  float x32c_thermalMag;
  CColor x330_;
  bool x334_isCold;
  bool x335_usePhazonModel;
  CHealthInfo x338_healthInfo;
  bool x340_;
  bool x341_;
};

CHECK_SIZEOF(CDestroyableRock, (VERSION >= VERSION_GM8P_00 ? 0x358 : 0x348))

class CThardus : public CPatterned {
public:
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) override;
  void Render(const CStateManager& mgr) const override;
  bool CanRenderUnsorted(const CStateManager&) const override { return false; }
  void Touch(CActor&, CStateManager&) override {}
  CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  CAABox GetSortingBounds(const CStateManager& mgr) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;
  void Patrol(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Dead(CStateManager& mgr, EStateMsg msg, float arg) override;
  void PathFind(CStateManager& mgr, EStateMsg msg, float arg) override;
  void TargetPatrol(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Generate(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Attack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void LoopedAttack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void DoubleSnap(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Shuffle(CStateManager& mgr, EStateMsg msg, float arg) override;
  void GetUp(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Taunt(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Suck(CStateManager& mgr, EStateMsg msg, float arg) override;
  void ProjectileAttack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Flinch(CStateManager& mgr, EStateMsg msg, float arg) override;
  void TelegraphAttack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Explode(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Cover(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Enraged(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Growth(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Faint(CStateManager& mgr, EStateMsg msg, float arg) override;
  bool PathFound(CStateManager& mgr, float arg) override;
  bool InRange(CStateManager& mgr, float arg) override;
  bool PatternOver(CStateManager& mgr, float arg) override;
  bool HasAttackPattern(CStateManager& mgr, float arg) override;
  bool AnimOver(CStateManager& mgr, float arg) override;
  bool InPosition(CStateManager& mgr, float arg) override;
  bool ShouldTurn(CStateManager& mgr, float arg) override;
  bool HitSomething(CStateManager& mgr, float arg) override;
  bool HearPlayer(CStateManager& mgr, float arg) override;
  bool CoverBlown(CStateManager& mgr, float arg) override;
  bool CoveringFire(CStateManager& mgr, float arg) override;
  bool AggressionCheck(CStateManager& mgr, float arg) override;
  bool AttackOver(CStateManager& mgr, float arg) override;
  bool ShouldTaunt(CStateManager& mgr, float arg) override;
  bool ShouldMove(CStateManager& mgr, float arg) override;
  bool StartAttack(CStateManager& mgr, float arg) override;
  bool CodeTrigger(CStateManager& mgr, float arg) override;
  bool IsDizzy(CStateManager& mgr, float arg) override;
  bool ShouldCallForBackup(CStateManager& mgr, float arg) override;
  CPathFindSearch* GetSearchPath() override { return &x7f0_pathFindSearch; }

  CThardus(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
           const CModelData& mData, const CActorParameters& actParms, const CPatternedInfo& pInfo,
           const rstl::vector< CModelData >& models1, const rstl::vector< CModelData >& models2,
           uint particle1, uint particle2, uint particle3, float f1, float f2, float f3, float f4,
           float f5, float f6, uint stateMachine, uint particle4, uint particle5, uint particle6,
           uint particle7, uint particle8, uint particle9, uint texture, uint sfxID1,
           uint particle10, uint sfxID2, uint sfxID3, uint sfxID4);

  void RenderThermalSpot(float dt) const;
  void ShakeScreen(CStateManager& mgr, const CVector3f& position, const float, const float,
                   const float);
  void StartTimedThermalFlash(CStateManager& mgr, float, const CActor& actor);
  bool CanLockOnToRockProjectiles(const CStateManager& mgr) const;
  int GetThermalFlashState() const { return x7c4_; }

private:
  enum EThardusState { kTS_Invalid = -1, kTS_Zero, kTS_Retreat, kTS_Patrol };
  void SetThardusState(EThardusState state, CStateManager& mgr);
  void ComputeNumberOfRangedAttacks(CStateManager& mgr);
  void UpdateTotalHealth(CStateManager& mgr);
  CVector2f GetRepulsorSteering(CStateManager& mgr);
  CVector2f GetSteeringVector(CStateManager& mgr, float dt);
  void ShakePlayer(CStateManager& mgr, float intensity);
  void AddParticleEffect(CStateManager& mgr, const CVector3f& pos, CAssetId particle);
  static const char* const skHeadRockNameStr;

  bool IsEnraged() const;
  void BeginFlash(CStateManager& mgr, const CActor& actor);
  void UpdateThermalFlash(CStateManager& mgr, float dt);
  void MinorKnockBackNow(CStateManager& mgr);
  void BecomeEnraged(CStateManager& mgr);
  void KnockBackNow(CStateManager& mgr);
  void SummonIceStorm(CStateManager& mgr);
  CVector3f GetNextPatrolDest(const CStateManager& mgr);
  CVector3f ComputePatrolDest(CStateManager& mgr);
  void GetWaypoints(CScriptWaypoint& waypoint, CStateManager& mgr,
                    rstl::reserved_vector< TUniqueId, 16 >& ids);
  void ComputeNewRangedAttack(CStateManager& mgr);
  void ComputeNextRangedAttack(CStateManager& mgr);
  void StartBigThermalFlash(CStateManager& mgr, const CActor& actor);
  void EndBigThermalFlash(CStateManager& mgr);
  enum EUpdateMaterialMode { kUMM_Add, kUMM_Remove };
  void AddAABoxCollisionList(const CPatternedCollisionUtils::SAABoxJointInfo* joints, int count,
                             rstl::vector< CJointCollisionDescription >& list);
  void AddSphereCollisionList(const CPatternedCollisionUtils::SSphereJointInfo* joints, int count,
                              rstl::vector< CJointCollisionDescription >& list);
  void SetMaterialProperties(rstl::single_ptr< CCollisionActorManager >& colMgr,
                             CStateManager& mgr);
  void InitializeCollisionManagers(CStateManager& mgr);
  void UpdateDestroyableRockPositions(CStateManager& mgr);
  void ExposeDestroyableRock(CStateManager& mgr, uint index);
  void UpdateDestroyableRockCollisionActors(CStateManager& mgr);
  void CacheNonDestroyableCollisionActorIds(const CCollisionActorManager& colMgr);
  void UpdateExcludeList(const CCollisionActorManager& colMgr, EUpdateMaterialMode mode,
                         EMaterialTypes material, CStateManager& mgr);
  void UpdateNonDestroyableCollisionActorMaterials(EUpdateMaterialMode mode,
                                                   EMaterialTypes material, CStateManager& mgr);
  static const char* const skDamageableRockJointNameList[7];
  struct SRockProjectileOffset {
    const char* name;
    float x;
    float y;
    float z;
  };
  static const SRockProjectileOffset skRockProjectileForwardOffsets[6];
  static const char* const skDamageableRockCollisionJointNameList[7];
  static const CPatternedCollisionUtils::SSphereJointInfo skDamageableSphereJointInfoList[7];
  static const CPatternedCollisionUtils::SSphereJointInfo skNonDamageableSphereJointInfoList[5];
  static const CPatternedCollisionUtils::SAABoxJointInfo skAABoxJointInfoList[2];
  static const CVector3f skThardusRayCastOffset;
  static const CHealthInfo skCollisionActorHealthInfo;
  class CThardusSomething {
  public:
    CThardusSomething() : x0_(kInvalidUniqueId), x4_(CVector3f::Zero()), x10_24_(false) {}

  private:
    TUniqueId x0_;
    CVector3f x4_;
    bool x10_24_ : 1;
  };
  uint x568_;
  TUniqueId x56c_;
  uint x570_;
  uint x574_;
  rstl::reserved_vector< rstl::reserved_vector< TUniqueId, 16 >, 2 > x578_waypoints;
  int x5c4_;
  bool x5c8_heardPlayer;
  rstl::vector< CModelData > x5cc_;
  rstl::vector< CModelData > x5dc_;
  int x5ec_stateProg;
  rstl::single_ptr< CCollisionActorManager > x5f0_rockColliders;
  rstl::single_ptr< CCollisionActorManager > x5f4_;
  rstl::single_ptr< CCollisionActorManager > x5f8_;
  TUniqueId x5fc_projectileId;
  CAssetId x600_;
  CAssetId x604_;
  CAssetId x608_;
  TEditorId x60c_projectileEditorId;
  rstl::vector< TUniqueId > x610_destroyableRocks;
  rstl::vector< TUniqueId > x620_;
  CAssetId x630_;
  rstl::vector< TUniqueId > x634_nonDestroyableActors;
  EThardusState x644_;
  uint x648_currentRock;
  TUniqueId x64c_fog;
  CVector2f x650_;
  int x658_;
  int x65c_;
  uint x660_;
  rstl::reserved_vector< TUniqueId, 16 > x664_repulsors;
  bool x688_;
  bool x689_;
  uint x68c_;
  float x690_;
  float x694_;
  float x698_;
  float x69c_;
  float x6a0_;
  float x6a4_;
  float x6a8_;
  float x6ac_;
  rstl::vector< bool > x6b0_destroyedRocks;
  rstl::vector< TUniqueId > x6c0_rockLights;
  CAssetId x6d0_;
  CAssetId x6d4_;
  CAssetId x6d8_;
  CAssetId x6dc_;
  CAssetId x6e0_;
  CAssetId x6e4_;
  CAssetId x6e8_;
  ushort x6ec_;
  CAssetId x6f0_;
  uint x6f4_;
  float x6f8_;
  CThardusSomething x6fc_[4];
  CVector3f x74c_;
  int x758_;
  int x75c_;
  int x760_;
  CTransform4f x764_startTransform;
  uint x794_;
  rstl::vector< TUniqueId > x798_;
  rstl::vector< TUniqueId > x7a8_timers;
  float x7b8_;
  float x7bc_;
  float x7c0_;
  int x7c4_;
  bool x7c8_;
  CVector3f x7cc_;
  CVector3f x7d8_;
  CVector3f x7e4_;
  CPathFindSearch x7f0_pathFindSearch;
  bool x8d4_;
  CVector3f x8d8_;
  CVector3f x8e4_;
  bool x8f0_;
  char x8f1_curPatrolPath;
  char x8f2_curPatrolPathWaypoint;
  rstl::vector< TUniqueId > x8f4_waypoints;
  CSfxHandle x904_;
  bool x908_;
  bool x909_;
  rstl::vector< float > x90c_rockHealths;
  TCachedToken< CTexture > x91c_flareTexture;
  TUniqueId x928_currentRockId;
  CVector3f x92c_currentRockPos;
  bool x938_;
  bool x939_;
  bool x93a_;
  bool x93b_;
  bool x93c_;
  bool x93d_;
  uint x940_;
  float x944_;
  uint x948_;
  bool x94c_initialized;
  bool x94d_;
  CVector3f x950_;
  bool x95c_doCodeTrigger;
  uchar x95d_;
  bool x95e_;
};
CHECK_SIZEOF(CThardus, (VERSION >= VERSION_GM8P_00 ? 0x970 : 0x960))

#endif // _CTHARDUS
