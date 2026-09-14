#ifndef _CDRONE
#define _CDRONE

#include "Collision/CCollidableSphere.hpp"
#include "Kyoto/Animation/CharacterCommon.hpp"
#include "MetroidPrime/CAxisAngle.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CVisorFlare.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "rstl/vector.hpp"

class CCollisionResponseData;

class CDrone : public CPatterned {
public:
  ~CDrone() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) override;
  void AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const override;
  void PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) override;
  void Render(const CStateManager& mgr) const override;
  bool CanRenderUnsorted(const CStateManager& mgr) const override;
  const CDamageVulnerability* GetDamageVulnerability(const CVector3f& pos, const CVector3f& dir,
                                                     const CDamageInfo& info) const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f& pos, const CVector3f& dir,
                                                         const CWeaponMode& mode,
                                                         int attrib) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;
  const CCollisionPrimitive* GetCollisionPrimitive() const override;
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;
  void KnockBack(const CVector3f& dir, CStateManager& mgr, const CDamageInfo& info, float magnitude,
                 bool direct, const bool inDeferred) override;
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt) override;
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TargetPlayer(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TargetCover(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Deactivate(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Attack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Active(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Flee(CStateManager& mgr, EStateMsg msg, float dt) override;
  void ProjectileAttack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TelegraphAttack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Dodge(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Retreat(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Cover(CStateManager& mgr, EStateMsg msg, float dt) override;
  void SpecialAttack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void PathFindEx(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Dead(CStateManager& mgr, EStateMsg msg, float dt) override;
  bool Leash(CStateManager& mgr, float arg) override;
  bool InRange(CStateManager& mgr, float arg) override;
  bool SpotPlayer(CStateManager& mgr, float arg) override;
  bool AnimOver(CStateManager& mgr, float arg) override;
  bool AttackOver(CStateManager& mgr, float arg) override;
  bool ShouldAttack(CStateManager& mgr, float arg) override;
  bool ShouldFire(CStateManager& mgr, float arg) override;
  bool HearShot(CStateManager& mgr, float arg) override;
  bool CoverCheck(CStateManager& mgr, float arg) override;
  bool LineOfSight(CStateManager& mgr, float arg) override;
  bool ShouldMove(CStateManager& mgr, float arg) override;
  bool CodeTrigger(CStateManager& mgr, float arg) override;
  void Burn(float duration, float damage) override;
  CPathFindSearch* GetSearchPath() override { return &x6b0_pathFind; }
  virtual void BuildNearList(EMaterialTypes includeMat, EMaterialTypes excludeMat, float radius,
                             rstl::reserved_vector< TUniqueId, 1024 >& list, CStateManager& mgr);
  virtual void SetLightEnabled(CStateManager& mgr, bool active);
  virtual void SetVisorFlareEnabled(CStateManager& mgr, bool active);
  virtual void UpdateVisorFlare(CStateManager& mgr);
  virtual int PathFindGetFlags() const { return 3; }

  CDrone(TUniqueId uid, const rstl::string& name, EFlavorType flavor, const CEntityInfo& info,
         const CTransform4f& xf, float f1, const CModelData& mData, const CPatternedInfo& pInfo,
         const CActorParameters& aParms, EMovementType moveType, EColliderType collider,
         EBodyType bodyType, const CDamageInfo& dInfo1, CAssetId aId1, const CDamageInfo& dInfo2,
         CAssetId aId2, rstl::vector< CVisorFlare::CFlareDef > flares, float f2, float f3, float f4,
         float f5, float f6, float f7, float f8, float f9, float f10, float f11, float f12,
         float f13, float f14, float f15, float f16, float f17, float f18, float f19, float f20,
         CAssetId crscId, float f21, float f22, float f23, float f24, int soundId, bool b1);

private:
  void UpdateTouchBounds(float radius);
  bool HitShield(const CVector3f& dir) const;
  void SquadAdd(CStateManager& mgr);
  void SquadRemove(CStateManager& mgr);
  void SetLaserActive(CStateManager& mgr, int laserIdx, bool active);
  void FireProjectile(CStateManager& mgr, const CTransform4f& xf);
  void LaunchProjectile(const CTransform4f& xf, CStateManager& mgr);
  void AvoidOtherAIs(CStateManager& mgr);
  void UpdateScanner(CStateManager& mgr, float dt);
  void UpdateLaser(CStateManager& mgr, float dt);
  void UpdateWaterRipples(CStateManager& mgr);
  void UpdatePitchBend(float dt);
  void UpdateThermal(CStateManager& mgr, float dt);

  CAssetId x568_laserParticlesId;
  TLockedToken< CCollisionResponseData > x56c_collisionResponse;
  TUniqueId x578_lightId;
  TUniqueId x57a_visorFlareId;
  rstl::vector< CVisorFlare::CFlareDef > x57c_flares;
  pas::EStepDirection x58c_prevDodgeDir;
  CDamageInfo x590_damageInfo;
  CDamageInfo x5ac_laserDamageInfo;
  float x5c8_;
  float x5cc_;
  float x5d0_;
  float x5d4_;
  float x5d8_;
  float x5dc_;
  float x5e0_;
  float x5e4_;
  mutable float x5e8_shieldTime;
  float x5ec_turnSpeed;
  float x5f0_;
  float x5f4_;
  float x5f8_;
  float x5fc_;
  float x600_;
  float x604_;
  float x608_;
  float x60c_;
  float x610_;
  float x614_;
  float x618_;
  float x61c_;
  float x620_;
  float x624_;
  float x628_;
  float x62c_;
  float x630_;
  float x634_;
  float x638_;
  float x63c_;
  float x640_;
  float x644_;
  float x648_;
  float x64c_;
  float x650_;
  float x654_;
  float x658_;
  float x65c_;
  float x660_;
  float x664_;
  float x668_;
  float x66c_;
  CVector3f x670_;
  CVector3f x67c_;
  TUniqueId x688_teamMgr;
  CCollidableSphere x690_colSphere;
  CPathFindSearch x6b0_pathFind;
  CAxisAngle x794_;
  CVector3f x7a0_;
  CVector3f x7ac_lightPos;
  float x7b8_;
  float x7bc_;
  float x7c0_;
  float x7c4_;
  int x7c8_;
  ushort x7cc_laserSfx;
  CSfxHandle x7d0_laserSfxHandle;
  uint x7d4_;
  rstl::reserved_vector< TUniqueId, 2 > x7d8_laserIds;
  rstl::reserved_vector< CVector3f, 2 > x7e0_lasersStart;
  rstl::reserved_vector< CVector3f, 2 > x7fc_lasersEnd;
  rstl::reserved_vector< float, 2 > x818_lasersTime;
  rstl::reserved_vector< bool, 2 > x824_activeLasers;
  rstl::single_ptr< CModelData > x82c_shieldModel;
  short x830_;
  int x832_24_ : 3;
  int x832_27_ : 3;
  bool x834_24_waveHit : 1;
  bool x834_25_ : 1;
  bool x834_26_ : 1;
  bool x834_27_ : 1;
  bool x834_28_ : 1;
  bool x834_29_codeTrigger : 1;
  bool x834_30_visible : 1;
  bool x834_31_attackOver : 1;
  bool x835_24_ : 1;
  bool x835_25_ : 1;
  bool x835_26_ : 1;
};
CHECK_SIZEOF(CDrone, (VERSION >= VERSION_GM8P_00 ? 0x848 : 0x838))

#endif // _CDRONE
