#ifndef _CRIDLEY
#define _CRIDLEY

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "MetroidPrime/CBoneTracking.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/Cameras/CCameraShakeData.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Weapons/CBeamInfo.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CCollisionActorManager;
class CParticleElectric;
class CElectricDescription;
class CProjectedShadow;

class CRidleyData {
  friend class CRidley;
#if VERSION < VERSION_GM8P_00
  CAssetId x0_;
  CAssetId x4_;
  CAssetId x8_;
  CAssetId xc_;
  CAssetId x10_;
  CAssetId x14_;
  CAssetId x18_;
  CAssetId x1c_;
  CAssetId x20_;
  CAssetId x24_;
#endif
  CAssetId x28_;
  CAssetId x2c_;
  CAssetId x30_;
  float x34_;
  float x38_;
  float x3c_;
  float x40_;
  CAssetId x44_;
  CDamageInfo x48_;
  CBeamInfo x64_;
  ushort xa8_;
  CAssetId xac_;
#if VERSION >= VERSION_GM8P_00
  CAssetId x88_;
#endif
  CDamageInfo xb0_;
  CCameraShakeData xcc_;
  CAssetId x1a0_;
  CDamageInfo x1a4_;
  CCameraShakeData x1c0_;
  ushort x294_;
  CDamageInfo x298_;
  CCameraShakeData x2b4_;
  float x388_;
  float x38c_;
  CDamageInfo x390_;
  float x3ac_;
  CDamageInfo x3b0_;
  float x3cc_;
  CDamageInfo x3d0_;
  float x3ec_;
  CAssetId x3f0_;
  float x3f4_;
  ushort x3f8_;
  CDamageInfo x3fc_;
#if VERSION >= VERSION_GM8P_00
  CDamageInfo mBounceDamage;
#endif

public:
  CRidleyData(CInputStream& in, int propCount);
};

CHECK_SIZEOF(CRidleyData,
             (VERSION >= VERSION_GM8P_00 ? 0x410 : 0x418))

class CRidley : public CPatterned {
public:
  ~CRidley() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  void PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) override;
  void Render(const CStateManager& mgr) const override;
  void AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const override;
  CAABox GetSortingBounds(const CStateManager& mgr) const override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  float GetGravityConstant() const override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f&, const CVector3f&,
                                                         const CWeaponMode&, int) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;
  void Patrol(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Dead(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Generate(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Attack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void LoopedAttack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void JumpBack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void DoubleSnap(CStateManager& mgr, EStateMsg msg, float arg) override;
  void CoverAttack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Crouch(CStateManager& mgr, EStateMsg msg, float arg) override;
  void FadeOut(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Taunt(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Flee(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Lurk(CStateManager& mgr, EStateMsg msg, float arg) override;
  void ProjectileAttack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Flinch(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Hurled(CStateManager& mgr, EStateMsg msg, float arg) override;
  void TelegraphAttack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Jump(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Explode(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Dodge(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Retreat(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Approach(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Enraged(CStateManager& mgr, EStateMsg msg, float arg) override;
  void SpecialAttack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Land(CStateManager& mgr, EStateMsg msg, float arg) override;
  bool Attacked(CStateManager& mgr, float arg) override;
  bool TooClose(CStateManager& mgr, float arg) override;
  bool InRange(CStateManager& mgr, float arg) override;
  bool ShouldAttack(CStateManager& mgr, float arg) override;
  bool ShouldDoubleSnap(CStateManager& mgr, float arg) override;
  bool ShouldTurn(CStateManager& mgr, float arg) override;
  bool HitSomething(CStateManager& mgr, float arg) override;
  bool AttackOver(CStateManager& mgr, float arg) override;
  bool ShouldTaunt(CStateManager& mgr, float arg) override;
  bool ShouldFire(CStateManager& mgr, float arg) override;
  bool ShouldDodge(CStateManager& mgr, float arg) override;
  bool ShouldRetreat(CStateManager& mgr, float arg) override;
  bool ShouldCrouch(CStateManager& mgr, float arg) override;
  bool ShouldMove(CStateManager& mgr, float arg) override;
  bool ShotAt(CStateManager& mgr, float arg) override;
  bool SetAIStage(CStateManager& mgr, float arg) override;
  bool AIStage(CStateManager& mgr, float arg) override;
  bool ShouldStrafe(CStateManager& mgr, float arg) override;
  bool IsDizzy(CStateManager& mgr, float arg) override;
#if VERSION >= VERSION_GM8P_00
  void TurnAround(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Bounce(CStateManager& mgr, EStateMsg msg, float arg) override;
  bool BounceFind(CStateManager& mgr, float arg) override;
  bool SpotPlayer(CStateManager& mgr, float arg) override;
  bool AggressionCheck(CStateManager& mgr, float arg) override;
#endif

  CRidley(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
          const CModelData& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms,
          CInputStream& in, uint propCount);
  static int GetNumProperties() { return skNumProperties; }

private:
  void SetupCollisionManagers(CStateManager& mgr);
  void SetStage2Vulnerability(CStateManager& mgr);
  void SetStage3Immunity(CStateManager& mgr);
  void SetStage3BreastVulnerability(CStateManager& mgr);
  void SetStage3ThroatVulnerability(CStateManager& mgr);
  void ScaleCollisionSpheres(CStateManager& mgr, float scale) const;
  void ActivateBeam(CStateManager& mgr);
  void DeactivateBeam(CStateManager& mgr, bool reset);
  void UpdateBeam(CStateManager& mgr, float dt);
  void ActivateWingElectricity(float duration, const bool spread);
  void UpdateWingElectricity(CStateManager& mgr, float dt);
  void RandomSpeedUp(CStateManager& mgr);
  void RandomSlowDown() const;
  void ConstrainToHenge(float dt);
  void UpdateFlight(float dt);
  void Fly(const CVector3f& direction, float speed, float dt);
  void FacePlayer(CStateManager& mgr, float dt);
  void PushPlayer(CStateManager& mgr) const;
  void ChooseStage2Attack(CStateManager& mgr);
  void ChooseStage3Attack(CStateManager& mgr);
  bool CanJumpAttack() const;

  CRidleyData mData;
  rstl::single_ptr< CCollisionActorManager > mTailCollision;
  rstl::single_ptr< CCollisionActorManager > mBodyCollision;
  TUniqueId mHeadId;
  TUniqueId mBreastPlateId;
  TLockedToken< CGenDescription > x98c_;
  CModelData x998_;
  CModelData x9e4_;
  CSegId mBreastPlateSegId;
  bool xa31_24_ : 1;
  bool xa31_25_ : 1;
  bool xa31_26_ : 1;
  bool xa31_27_ : 1;
  bool xa31_28_ : 1;
  bool xa31_29_ : 1;
  bool xa31_30_ : 1;
  bool xa31_31_ : 1;
  bool xa32_24_ : 1;
  bool xa32_25_ : 1;
  bool xa32_26_ : 1;
  bool xa32_27_ : 1;
  bool mShotAt : 1;
  bool xa32_29_ : 1;
  bool xa32_30_ : 1;
  bool xa32_31_ : 1;
  bool xa33_24_ : 1;
  bool xa33_25_ : 1;
  bool xa33_26_ : 1;
  bool xa33_27_ : 1;
  bool xa33_28_ : 1;
  bool mDoStrafe : 1;
  bool xa33_30_ : 1;
  bool xa33_31_ : 1;
  bool xa34_24_ : 1;
  bool xa34_25_ : 1;
  bool xa34_26_ : 1;
#if VERSION >= VERSION_GM8P_00
  bool xa34_27_ : 1;
  bool xa34_28_ : 1;
  bool xa34_29_ : 1;
  bool mJumpCanBeInterrupted : 1;
  bool mCanBreakLockOn : 1;
  bool mHasPreviousBeamPos : 1;
  bool mInFlinch : 1;
#else
  CModelData xa38_;
#endif
  CTransform4f xa84_;
  float xab4_;
  float xab8_;
  float xabc_;
  float xac0_;
  CAABox xac4_;
  float xadc_;
  float xae0_;
  float xae4_;
  float xae8_;
  CVector3f xaec_;
  CVector3f xaf8_;
  int xb04_;
  int xb08_;
  int xb0c_;
  float xb10_;
  float xb14_;
  float xb18_;
  float xb1c_;
  float xb20_;
  float xb24_;
  CSegId xb28_;
  CBoneTracking xb2c_;
  TUniqueId mPlasmaProjectile;
  CProjectileInfo xb68_;
  CSegId mHeadSegId;
  CSegId mMouthSegId;
  uchar xb92_;
  uchar xb93_;
  CTransform4f xb94_;
  CVector3f xbc4_;
  CVector3f xbd0_;
  float xbdc_;
  float xbe0_;
  CVector3f xbe4_;
  CVector3f xbf0_;
  CVector3f xbfc_;
  float xc08_;
  float xc0c_;
  float xc10_;
#if VERSION >= VERSION_GM8P_00
  CProjectileInfo mProjectileInfo;
#endif
  CProjectileInfo xc14_;
  CProjectileInfo xc3c_;
  int mAiStage;
  CVector3f xc68_;
  uint xc74_;
  float xc78_;
  float xc7c_;
  float xc80_;
  uint xc84_;
  uint xc88_;
#if VERSION >= VERSION_GM8P_00
  int mMeleeAttack;
#endif
  CDamageInfo xc8c_;
  CSfxHandle xca8_;
  CSfxHandle xcac_;
#if VERSION >= VERSION_GM8P_00
  int xcb0_;
#else
  uint xcb0_;
#endif
  uint xcb4_;
  float xcb8_;
  float xcbc_;
  uint xcc0_;
  uint xcc4_;
  float xcc8_;
  float xccc_;
  TLockedToken< CElectricDescription > xcd0_;
  rstl::auto_ptr< CParticleElectric > mElectric;
  rstl::reserved_vector< CSegId, 30 > mWingBoneIds;
  float xd08_;
  CSfxHandle xd0c_;
  rstl::single_ptr< CProjectedShadow > xd10_;
#if VERSION >= VERSION_GM8P_00
  CVector3f mPreviousBeamPos;
#else
  uint xd14_;
#endif

  static const CDamageVulnerability skDirectNormal;
  static const CDamageVulnerability skIceWeakness;
  static const int skNumProperties;
  static const rstl::string skBreastPlate;
  static const rstl::string skMouth;
  static const rstl::string skHead;
  static const rstl::string skRoot;
};
CHECK_SIZEOF(CRidley,
             (VERSION < VERSION_GM8E_02 ? 0xd18 : (VERSION == VERSION_GM8E_02 ? 0xd28 : 0xd08)))

#endif // _CRIDLEY
