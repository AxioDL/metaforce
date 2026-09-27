#ifndef _CMETROIDPRIME
#define _CMETROIDPRIME

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CBoneTracking.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/Cameras/CCameraShakeData.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/Enemies/CPoisonProjectile.hpp"
#include "MetroidPrime/Weapons/CBeamInfo.hpp"
#include "MetroidPrime/Weapons/CPlasmaProjectile.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

#include "Kyoto/Audio/CSfxHandle.hpp"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/SObjectTag.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CCollisionActorManager;
class CElementGen;
class CGenDescription;
class CParticleElectric;
class CParticleSwoosh;
class CProjectedShadow;
class CSwooshDescription;
class CInputStream;
struct CMetroidPrimeData;

struct SPrimeCameraShakePoint {
  float mAttackTime;
  float mSustainTime;
  float mDuration;
  float mMagnitude;

  explicit inline SPrimeCameraShakePoint(CInputStream& in);
};
CHECK_SIZEOF(SPrimeCameraShakePoint, 0x10)

struct SPrimeCameraShakerComponent {
  bool mUseModulation;
  SPrimeCameraShakePoint mAm;
  SPrimeCameraShakePoint mFm;

  explicit SPrimeCameraShakerComponent(CInputStream& in);
};
CHECK_SIZEOF(SPrimeCameraShakerComponent, 0x24)

struct SPrimeCameraShakeData {
  bool mUseSfx;
  float mDuration;
  float mSfxDist;
  SPrimeCameraShakerComponent mShakerX;
  SPrimeCameraShakerComponent mShakerY;
  SPrimeCameraShakerComponent mShakerZ;

  explicit SPrimeCameraShakeData(CInputStream& in);
};
CHECK_SIZEOF(SPrimeCameraShakeData, 0x78)

SCameraShakePoint BuildCameraShakePoint(const SPrimeCameraShakePoint& point);
CCameraShakerComponent BuildCameraShakerComponent(const SPrimeCameraShakerComponent& component);
CCameraShakeData BuildCameraShakeData(const SPrimeCameraShakeData& data);

struct CMetroidPrimeIceAttack {
  uint mPropertyCount;
  CAssetId mParticle1;
  CAssetId mParticle2;
  CAssetId mParticle3;
  CDamageInfo mDInfo;
  float x2c_;
  float x30_;
  CAssetId mTexture;
  ushort x38_;
  ushort x3a_;

  explicit CMetroidPrimeIceAttack(CInputStream& in);
};
CHECK_SIZEOF(CMetroidPrimeIceAttack, 0x3C)

struct CMetroidPrimeParasiteQueenAttack {
  CBeamInfo mBeamInfo;
  uint x44_;
  CDamageInfo mDInfo1;
  CWeaponAssetInfo mStruct5;
  float x88_;
  CDamageInfo mDInfo2;

  explicit CMetroidPrimeParasiteQueenAttack(CInputStream& in);
};
CHECK_SIZEOF(CMetroidPrimeParasiteQueenAttack, 0xA8)

class CMetroidPrime : public CPatterned {
public:
  ~CMetroidPrime() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId other, CStateManager& mgr) override;
  void PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) override;
  void AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  bool CanRenderUnsorted(const CStateManager& mgr) const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;
  void SelectTarget(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Run(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Attack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void TurnAround(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Active(CStateManager& mgr, EStateMsg msg, float arg) override;
  void InActive(CStateManager& mgr, EStateMsg msg, float arg) override;
  void CoverAttack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Crouch(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Taunt(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Suck(CStateManager& mgr, EStateMsg msg, float arg) override;
  void ProjectileAttack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Flinch(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Dodge(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Retreat(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Cover(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Approach(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Enraged(CStateManager& mgr, EStateMsg msg, float arg) override;
  void SpecialAttack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Growth(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Land(CStateManager& mgr, EStateMsg msg, float arg) override;
  bool TooClose(CStateManager& mgr, float arg) override;
  bool InMaxRange(CStateManager& mgr, float arg) override;
  bool PlayerSpot(CStateManager& mgr, float arg) override;
  bool ShouldAttack(CStateManager& mgr, float arg) override;
  bool ShouldDoubleSnap(CStateManager& mgr, float arg) override;
  bool InPosition(CStateManager& mgr, float arg) override;
  bool ShouldTurn(CStateManager& mgr, float arg) override;
  bool ShouldJumpBack(CStateManager& mgr, float arg) override;
  bool Stuck(CStateManager& mgr, float arg) override;
  bool CoverCheck(CStateManager& mgr, float arg) override;
  bool CoverFind(CStateManager& mgr, float arg) override;
  bool CoveringFire(CStateManager& mgr, float arg) override;
  bool AggressionCheck(CStateManager& mgr, float arg) override;
  bool AttackOver(CStateManager& mgr, float arg) override;
  bool ShouldFire(CStateManager& mgr, float arg) override;
  bool ShouldFlinch(CStateManager& mgr, float arg) override;
  bool ShouldRetreat(CStateManager& mgr, float arg) override;
  bool ShouldCrouch(CStateManager& mgr, float arg) override;
  bool ShouldMove(CStateManager& mgr, float arg) override;
  bool AIStage(CStateManager& mgr, float arg) override;
  bool StartAttack(CStateManager& mgr, float arg) override;
  bool ShouldSpecialAttack(CStateManager& mgr, float arg) override;
  bool CodeTrigger(CStateManager& mgr, float arg) override;
  CProjectileInfo* ProjectileInfo() override;

  enum EVulnerabilities {
    kVuln_Zero,
    kVuln_One,
    kVuln_Two,
    kVuln_Three,
    kVuln_Count,
  };

  enum EAttackType {
    kAT_Zero,
    kAT_One,
    kAT_Two,
    kAT_Three,
    kAT_Four,
    kAT_Five,
    kAT_Six,
    kAT_Seven,
    kAT_Eight,
    kAT_Nine,
    kAT_Ten,
    kAT_Eleven,
    kAT_Twelve,
    kAT_Thirteen,
    kAT_Count,
  };
  struct CMetroidPrimeAttackWeights {
    rstl::reserved_vector< float, kAT_Count > mAttackWeights;

    explicit CMetroidPrimeAttackWeights(CInputStream& in);

    float GetAttackWeight(EAttackType attack) const;
    static rstl::reserved_vector< float, kAT_Count > LoadAttackWeights(CInputStream& in);
  };

  struct CVulnerabilityEntry {
    uint mPropertyCount;
    CDamageVulnerability mDamageVulnerability;
    CColor mColor;
    uint x70_[2];

    explicit CVulnerabilityEntry(CInputStream& in);
  };

  CMetroidPrime(
      const TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
      const CTransform4f& xf, const CModelData& mData, const CPatternedInfo& pInfo,
      const CActorParameters& actorParms, const int pw1, const CCameraShakeData& shakeData1,
      const CCameraShakeData& shakeData2, const CCameraShakeData& shakeData3,
      const CMetroidPrimeIceAttack& iceAttack, const CAssetId particle1,
      const rstl::reserved_vector< CMetroidPrimeParasiteQueenAttack, kVuln_Count >& breathAttacks,
      const CAssetId weaponDesc1, const CDamageInfo&, const CCameraShakeData& shakeData4,
      const CAssetId weaponDesc2, const CDamageInfo& dInfo2, const CCameraShakeData& shakeData5,
      const CPoisonInfo& poisonInfo, const CDamageInfo& dInfo3, const CCameraShakeData& shakeData6,
      const CAssetId particle2, const CAssetId swoosh, const CAssetId particle3,
      const CAssetId particle4,
      const rstl::reserved_vector< CVulnerabilityEntry, 4 >& vulnerabilities);

public:
  class CMissileTarget : public CPhysicsActor {
  public:
    CMissileTarget(TUniqueId uid, bool active, const rstl::string& name, const CEntityInfo& info);
    ~CMissileTarget() override;
    DECLARE_TYPES_MATCH_OR_ACCEPT;
  };

private:
  enum ETractorbeamPosition {};
  void UpdateTractorBeamFX(const CVector3f& from, const CVector3f& to, ETractorbeamPosition index,
                           float width, float height, float phase);
  void UpdateRoomTransition(CStateManager& mgr);
  void SetActorAreaId(CStateManager& mgr, TUniqueId uid, TAreaId areaId);
  bool RoomHasRelay(const CStateManager& mgr, const TAreaId& areaId) const;
  void AttachToRelayInRoom(CStateManager& mgr, const TAreaId& areaId);
  void UpdateDataFromRelay(CStateManager& mgr);
  void UpdateTractorBeams(float dt, CStateManager& mgr);
  void UpdateUnderbodyDamage(CStateManager& mgr);
  void DoFaceHitCheck(TUniqueId uid, CStateManager& mgr);
  int SeverityForAttackType(EAttackType attack) const;
  pas::ELocomotionType SeverityForLocomotionType(EAttackType attack) const;
  bool CanTransitionFromReady(EAttackType attack);
  void PushPlayer(CStateManager& mgr, const CVector3f& direction, float dt);
  void PickNextMajorAttack(CStateManager& mgr);
  int GetActiveEnergyBallCount(CStateManager& mgr);
  void DoContactDamage(TUniqueId id, CStateManager& mgr);
  void EnableParasiteQueenProjectile(CStateManager& mgr, bool enable);
  CVector3f GetPlayerPosJumpCapped(CStateManager& mgr);
  void RemoveParasiteQueenProjectile(CStateManager& mgr);
  void CreateParasiteQueenProjectile(CStateManager& mgr);
  void UpdateParasiteQueenProjectile(CStateManager& mgr, float dt);
  void UpdateEnergyBall(CStateManager& mgr, float dt);
  void UpdateMissileTarget(CStateManager& mgr, float dt);
  void LaunchEnergyBall(CStateManager& mgr);
  void FindAttachedEnergyBalls(CStateManager& mgr);
  void CreateMissileTarget(CStateManager& mgr);
  void CreateTractorBeamVisorObject(CStateManager& mgr);
  void SetFaceVulnerability(CStateManager& mgr, EVulnerabilities vulnerability, const bool enable);
  void ResetAttackDistribution(CStateManager& mgr);
  void PickNextAttack(CStateManager& mgr, int attack);
  void NormalizeAttackDistribution();
  float GetDistributedAttackWeight(const CMetroidPrimeAttackWeights& weights, int attack) const;
  void UpdateEyeTracking(CStateManager& mgr, float dt);
  void UpdateFaceHealth(CStateManager& mgr);
  void TryScripted(CStateManager& mgr, int arg);
  void ChangeFaceVulnerability(CStateManager& mgr);
  void SetFaceVulnerable(CStateManager& mgr, bool enable);
  void ForwardScriptMsgs(EScriptObjectState state, CStateManager& mgr) const;
  void CreateCollisionActors(CStateManager& mgr);
  void SetupEyeTracking();
  void UpdateCollision(CStateManager& mgr, float dt);
  void ResetFaceHealth(CStateManager& mgr);
  void EnableEyeTracking(CStateManager& mgr, bool active);
  void UpdateAdditiveHeadAnimation(float dt);
  void SetTargetColor(const CColor& color, CStateManager& mgr);
  void UpdateTargetColor(CStateManager& mgr, float dt);
  void UpdateTimers(float dt);
  void RemoveFaceLockOn(CStateManager& mgr);
  void UpdateElectricity(CStateManager& mgr, float dt);
  void EnableFire(CStateManager& mgr, bool enable);
  void EnableElectrocution(CStateManager& mgr, bool enable);
  void UpdateFire(CStateManager& mgr, float dt);
  void EnableTractorBeams(CStateManager& mgr, bool enable);
  void ResetFaceLockOn(CStateManager& mgr);
  CVector3f GetRoomCenter(CStateManager& mgr);
  void DeleteTractorBeamVisorObject(CStateManager& mgr);
  void EnableTractorBeamVisorEffect(CStateManager& mgr, bool enable);
  void DeleteMissileTarget(CStateManager& mgr);
  void ResetMissileTarget(CStateManager& mgr);
  void ResetAttackTimeOut(CStateManager& mgr);
  TUniqueId GetConnectedRelayObject(CStateManager& mgr, EScriptObjectState state,
                                    EScriptObjectMessage msg);
  bool CanJump(CStateManager& mgr, float distance);
  TUniqueId FindBestAttackWaypoint(CStateManager& mgr, bool forward);

  TUniqueId mRelayId;
  rstl::single_ptr< CCollisionActorManager > mCollisionManager;
  int x570_;
  uint x574_;
  uint x578_;
  uint x57c_;
  uint x580_;
  bool x584_;
  rstl::reserved_vector< CVulnerabilityEntry, kVuln_Count > x588_;
  rstl::reserved_vector< CBoneTracking, 6 > x76c_;
  CHealthInfo x8c0_;
  float x8c8_;
  TUniqueId mHeadColActor;
  int x8d0_;
  int x8d4_;
  CColor mBeamColor;
  CColor x8dc_;
  CColor x8e0_;
  float x8e4_;
  int mHeadUpAdditiveBodyAnimIndex;
  float x8ec_;
  float x8f0_;
  bool x8f4_24_ : 1;
  bool x8f4_25_ : 1;
  bool x8f4_26_ : 1;
  bool x8f4_27_ : 1;
  bool x8f4_28_ : 1;
  CAABox x8f8_;
  float x910_;
  bool x914_24_ : 1;
  int x918_;
  int x91c_;
  float x920_;
  float x924_;
  float x928_;
  int x92c_;
  CMetroidPrimeIceAttack x930_;
  rstl::reserved_vector< CBeamInfo, 4 > x96c_;
  rstl::reserved_vector< CProjectileInfo, 4 > xa80_;
  rstl::reserved_vector< TUniqueId, 4 > mPlasmaProjectileIds;
  rstl::reserved_vector< CWeaponAssetInfo, 4 > xb30_;
  rstl::reserved_vector< rstl::pair< float, CDamageInfo >, 4 > xbc4_;
  TToken< CGenDescription > xc48_;
  rstl::auto_ptr< CElementGen > xc50_;
  int mCurPlasmaProjectile;
  float xc5c_;
  CVector3f xc60_;
  CVector3f xc6c_;
  CProjectileInfo xc78_;
  CCameraShakeData xca0_;
  CProjectileInfo xd74_;
  CCameraShakeData xd9c_;
  CPoisonInfo xe70_;
  TUniqueId xeac_;
  uint xeb0_;
  CDamageInfo xeb4_;
  CCameraShakeData xed0_;
  TToken< CElectricDescription > xfa4_;
  rstl::auto_ptr< CParticleElectric > xfac_;
  float xfb4_;
  float xfb8_;
  CSfxHandle xfbc_;
  bool xfc0_;
  bool xfc1_;
  rstl::reserved_vector< TToken< CGenDescription >, 2 > xfc4_;
  rstl::reserved_vector< TToken< CSwooshDescription >, 2 > xfd8_;
  rstl::reserved_vector< rstl::auto_ptr< CElementGen >, 2 > xfec_;
  rstl::reserved_vector< rstl::auto_ptr< CParticleSwoosh >, 2 > x1000_;
  TToken< CGenDescription > x1014_;
  TToken< CGenDescription > x101c_;
  rstl::auto_ptr< CElementGen > x1024_;
  rstl::reserved_vector< float, 2 > x102c_;
  rstl::reserved_vector< float, 2 > x1038_;
  TUniqueId mBillboardId;
  TUniqueId x1046_;
  float x1048_;
  CHealthInfo x104c_;
  bool x1054_24_ : 1;
  bool x1054_25_ : 1;
  bool x1054_26_ : 1;
  bool x1054_27_ : 1;
  rstl::reserved_vector< TEditorId, 4 > x1058_;
  rstl::reserved_vector< TUniqueId, 2 > mEnergyBallIds;
  float x1074_;
  int x1078_;
  float x107c_;
  float x1080_;
  float x1084_;
  float x1088_;
  CCameraShakeData x108c_;
  rstl::reserved_vector< CMetroidPrimeAttackWeights, 4 > x1160_;
  int x1254_;
  rstl::reserved_vector< float, 14 > x1258_;
  CCameraShakeData x1294_;
  CCameraShakeData x1368_;
  rstl::single_ptr< CProjectedShadow > x143c_;
  int x1440_;
  bool x1444_24_ : 1;
  bool x1444_25_ : 1;
};
CHECK_SIZEOF(CMetroidPrime, (VERSION >= VERSION_GM8E_02 ? 0x1458 : 0x1448))
NESTED_CHECK_SIZEOF(CMetroidPrime, CMissileTarget, (VERSION >= VERSION_GM8E_02 ? 0x268 : 0x258))
NESTED_CHECK_SIZEOF(CMetroidPrime, CMetroidPrimeAttackWeights, 0x3C)
NESTED_CHECK_SIZEOF(CMetroidPrime, CVulnerabilityEntry, 0x78)

struct CMetroidPrimeData {
  uint mPropertyCount;
  CPatternedInfo mPatternedInfo;
  CActorParameters mActorParms;
  int x1a4_;
  CCameraShakeData x1a8_;
  CCameraShakeData x27c_;
  CCameraShakeData x350_;
  CMetroidPrimeIceAttack x424_;
  CAssetId mParticle1;
  rstl::reserved_vector< CMetroidPrimeParasiteQueenAttack, 4 > x464_;
  CAssetId mWpsc1;
  CDamageInfo mDInfo1;
  CCameraShakeData mShakeData1;
  CAssetId mWpsc2;
  CDamageInfo mDInfo2;
  CCameraShakeData mShakeData2;
  CPoisonInfo x8f0_;
  CDamageInfo x92c_;
  CCameraShakeData x948_;
  CAssetId mParticle2;
  CAssetId mSwoosh;
  CAssetId mParticle3;
  CAssetId mParticle4;
  rstl::reserved_vector< CMetroidPrime::CVulnerabilityEntry, 4 > xa2c_;

  explicit CMetroidPrimeData(CInputStream& in);
  static uint VerifyExportCount(CInputStream& in);

  const CPatternedInfo& GetPatternedInfo() const { return mPatternedInfo; }

  static rstl::reserved_vector< CMetroidPrimeParasiteQueenAttack, 4 >
  LoadParasiteQueenBeams(CInputStream& in);
  static rstl::reserved_vector< CMetroidPrime::CVulnerabilityEntry, 4 >
  LoadVulnerabilities(CInputStream& in);
};
CHECK_SIZEOF(CMetroidPrimeData, 0xC10)

#endif // _CMETROIDPRIME
