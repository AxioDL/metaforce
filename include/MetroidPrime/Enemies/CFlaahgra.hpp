#ifndef _CFLAAHGRA
#define _CFLAAHGRA

#include "types.h"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CAnimationParameters.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

#include "Kyoto/CDependencyGroup.hpp"
#include "Kyoto/TToken.hpp"

#include "MetroidPrime/Collision/CJointCollisionDescription.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CBoneTracking;
class CCollisionActorManager;
class CGenDescription;
class CFlaahgraProjectile;

class CFlaahgraData {
public:
  CFlaahgraData(CInputStream& in, int propCount);

  float GetSmallScale() const { return x0_; }
  float GetLargeScale() const { return x4_; }
  float GetRetreatTime() const { return x8_; }
  float GetDizzyDuration() const { return x144_; }
  float GetCoverCooldown() const { return x148_; }
  float GetRetreatHP() const { return x140_; }
  const CActorParameters& GetActorParameters() const { return xd8_actorParameters; }
  float GetFaintToSmallHP() const { return xc_faintToSmallHP; }

  const CDamageVulnerability& GetSnakeVulnerability() const { return x10_snakeVulnerability; }

  CAssetId GetProjectileRes() const { return x78_projectileRes; }
  const CDamageInfo& GetProjectileDamage() const { return x7c_projectileDamage; }
  CAssetId GetChargedProjectileRes() const { return x98_chargedProjectileRes; }
  const CDamageInfo& GetChargedProjectileDamage() const { return x9c_chargedProjectileDamage; }
  CAssetId GetGrowingPlantsRes() const { return xb8_growingPlantsRes; }
  const CDamageInfo& GetBombSlotDamage() const { return xbc_bombSlotDamage; }

  CAssetId GetDependencyGroup() const { return x158_; }

  CAnimationParameters GetAnimationParameters() const { return x14c_animationParameters; }

  static int GetNumProperties() { return skNumProperties; }

private:
  float x0_;
  float x4_;
  float x8_;
  float xc_faintToSmallHP;
  CDamageVulnerability x10_snakeVulnerability;
  CAssetId x78_projectileRes;
  CDamageInfo x7c_projectileDamage;
  CAssetId x98_chargedProjectileRes;
  CDamageInfo x9c_chargedProjectileDamage;
  CAssetId xb8_growingPlantsRes;
  CDamageInfo xbc_bombSlotDamage;
  CActorParameters xd8_actorParameters;
  float x140_;
  float x144_;
  float x148_;
  CAnimationParameters x14c_animationParameters;
  CAssetId x158_;

  static const int skNumProperties;
};
CHECK_SIZEOF(CFlaahgraData, 0x15c)

class CFlaahgraRenderer : public CActor {
public:
  CFlaahgraRenderer(TUniqueId uid, TUniqueId owner, const rstl::string& name,
                    const CEntityInfo& info, const CTransform4f& xf);

  // CEntity
  ~CFlaahgraRenderer() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;

  // CActor
  void AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;

private:
  TUniqueId xe8_owner;
};
CHECK_SIZEOF(CFlaahgraRenderer, 0xf0)

class CFlaahgra : public CPatterned {
public:
  CFlaahgra(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
            const CTransform4f& xf, const CAnimRes& animRes, const CPatternedInfo& pInfo,
            const CActorParameters& actParms, const CFlaahgraData& data);

  // CEntity
  ~CFlaahgra() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CActor
  void AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const override;
  bool CanRenderUnsorted(const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CAi
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;

  bool AnimOver(CStateManager& mgr, float arg) override;
  bool AIStage(CStateManager& mgr, float arg) override;
  bool HitSomething(CStateManager& mgr, float arg) override;
  bool OffLine(CStateManager& mgr, float arg) override;
  bool ShouldTurn(CStateManager& mgr, float arg) override;
  bool ShouldAttack(CStateManager& mgr, float arg) override;
  bool BreakAttack(CStateManager& mgr, float arg) override;
  bool IsDizzy(CStateManager& mgr, float arg) override;
  bool CoverCheck(CStateManager& mgr, float arg) override;
  bool ShouldSpecialAttack(CStateManager& mgr, float arg) override;
  bool ShouldFire(CStateManager& mgr, float arg) override;

  void FadeIn(CStateManager& mgr, EStateMsg msg, float arg) override;
  void FadeOut(CStateManager& mgr, EStateMsg msg, float arg) override;
  void TurnAround(CStateManager& mgr, EStateMsg msg, float arg) override;
  void GetUp(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Growth(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Generate(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Faint(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Dead(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Attack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Dizzy(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Suck(CStateManager& mgr, EStateMsg msg, float arg) override;
  void ProjectileAttack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Cover(CStateManager& mgr, EStateMsg msg, float arg) override;
  void SpecialAttack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Enraged(CStateManager& mgr, EStateMsg msg, float arg) override;

  // CPatterned
  CProjectileInfo* ProjectileInfo() override;

private:
  struct SJointInfo {
    const char* from;
    const char* to;
    float radius;
    float separation;
  };
  struct SSphereJointInfo {
    const char* name;
    float radius;
  };

  void LoadDependencies(CAssetId id);
  void ResetModelDataAndBodyController();
  void GatherAssets(CStateManager& mgr);
  void LoadTokens(CStateManager& mgr);
  void FinalizeLoad(CStateManager& mgr);
  void GetMirrorWaypoints(CStateManager& mgr);
  void SetupCollisionManager(CStateManager& mgr);
  void AddCollisionList(const SJointInfo* list, int count,
                        rstl::vector< CJointCollisionDescription >& out);
  void AddSphereCollisionList(const SSphereJointInfo* list, int count,
                              rstl::vector< CJointCollisionDescription >& out);
  void UpdateCollisionManagers(float dt, CStateManager& mgr);
  void UpdateSmallScaleReGrowth(float dt);
  void UpdateHealthInfo(CStateManager& mgr);
  void UpdateAimPosition(CStateManager& mgr, float dt);
  void SetMaterialProperties(rstl::single_ptr< CCollisionActorManager >& colMgr,
                             CStateManager& mgr);
  void SetCollisionActorBounds(CStateManager& mgr,
                               const rstl::single_ptr< CCollisionActorManager >& colMgr,
                               const CVector3f& extendedBounds);
  void UpdateScale(float t, float minScale, float maxScale);
  void SetupHealthInfo(CStateManager& mgr);
  CVector3f GetAttackTargetPos(CStateManager& mgr) const;
  void RattlePlayer(CStateManager& mgr, const CVector3f& vec);
  void CalculateFallDirection();
  void UpdateHeadDamageVulnerability(CStateManager& mgr, bool vulnerable);
  void ApplyBombSlotDamage(CStateManager& mgr);
  CVector3f GetAttackTargetVector(CStateManager& mgr) const;
  uint FindBestMeleeAttackType(CStateManager& mgr) const;
  bool IsSwipeAttack() const;
  bool IsPlantStrikeAttack() const;
  float GetEndActionTime() const;
  bool IsFiringProjectile() const;
  bool IsSphereCollider(TUniqueId uid) const;
  TUniqueId GetMirrorNearestPlayer(const CStateManager& mgr) const;
  CFlaahgraProjectile* CreateProjectile(const CTransform4f& xf, CStateManager& mgr);

  int x568_state;
  CFlaahgraData x56c_data;
  rstl::auto_ptr< CBoneTracking > x6c8_boneTracking;
  TUniqueId x6d0_rendererId;
  TToken< CGenDescription > x6d4_plantsParticleGenDesc;
  CProjectileInfo x6dc_normalProjectileInfo;
  CProjectileInfo x704_bigStrikeProjectileInfo;
  int x72c_projectilesCreated;
  rstl::reserved_vector< CVector3f, 5 > x730_projectileDirs;
  rstl::reserved_vector< TUniqueId, 4 > x770_mirrorWaypoints;
  TUniqueId x77c_targetMirrorWaypointId;
  int x780_;
  int x784_;
  int x788_stage;
  CVector3f x78c_;
  int x798_meleeInitialAnimState;
  rstl::single_ptr< CCollisionActorManager > x79c_leftArmCollision;
  rstl::single_ptr< CCollisionActorManager > x7a0_rightArmCollision;
  rstl::single_ptr< CCollisionActorManager > x7a4_sphereCollision;
  int x7a8_;
  int x7ac_;
  uint x7b0_;
  int x7b4_;
  float x7b8_;
  float x7bc_;
  float x7c0_;
  float x7c4_actionDuration;
  float x7c8_;
  float x7cc_generateEndCooldown;
  float x7d0_hitSomethingTime;
  float x7d4_faintTime;
  float x7d8_;
  CDamageInfo x7dc_halfContactDamage;
  int x7f8_;
  rstl::reserved_vector< TUniqueId, 6 > x7fc_sphereColliders;
  TUniqueId x80c_headActor;
  float x810_;
  float x814_;
  float x818_curHp;
  float x81c_;
  CVector3f x820_aimPosition;
  rstl::reserved_vector< CVector3f, 4 > x82c_;
  rstl::reserved_vector< CVector3f, 4 > x860_;
  CVector3f x894_fallDirection;
  CVector3f x8a0_;
  CAnimRes x8ac_;
  rstl::optional_object< CToken > x8c8_depGroup;
  rstl::vector< CToken > x8d4_tokens;
  bool x8e4_24_loaded : 1;
  bool x8e4_25_loading : 1;
  bool x8e4_26_ : 1;
  bool x8e4_27_ : 1;
  bool x8e4_28_ : 1;
  bool x8e4_29_getup : 1;
  bool x8e4_30_bigStrike : 1;
  bool x8e4_31_ : 1;
  bool x8e5_24_ : 1;
  bool x8e5_25_ : 1;
  bool x8e5_26_ : 1;
  bool x8e5_27_ : 1;
  bool x8e5_28_ : 1;
  bool x8e5_29_ : 1;
  bool x8e5_30_ : 1;

  static const SJointInfo skLeftArmJointList[];
  static const SJointInfo skRightArmJointList[];
  static const SSphereJointInfo skSphereJointList[];
  static const pas::ESeverity skpAttackTypeLookup[];
  static const int skpComboChain[];
};
CHECK_SIZEOF(CFlaahgra, 0x8e8)

#endif // _CFLAAHGRA
