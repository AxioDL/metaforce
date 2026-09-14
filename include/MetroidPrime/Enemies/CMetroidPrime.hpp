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
  float x0_attackTime;
  float x4_sustainTime;
  float x8_duration;
  float xc_magnitude;

  explicit inline SPrimeCameraShakePoint(CInputStream& in);
};
CHECK_SIZEOF(SPrimeCameraShakePoint, 0x10)

struct SPrimeCameraShakerComponent {
  bool x0_useModulation;
  SPrimeCameraShakePoint x4_am;
  SPrimeCameraShakePoint x14_fm;

  explicit SPrimeCameraShakerComponent(CInputStream& in);
};
CHECK_SIZEOF(SPrimeCameraShakerComponent, 0x24)

struct SPrimeCameraShakeData {
  bool x0_useSfx;
  float x4_duration;
  float x8_sfxDist;
  SPrimeCameraShakerComponent xc_shakerX;
  SPrimeCameraShakerComponent x30_shakerY;
  SPrimeCameraShakerComponent x54_shakerZ;

  explicit SPrimeCameraShakeData(CInputStream& in);
};
CHECK_SIZEOF(SPrimeCameraShakeData, 0x78)

SCameraShakePoint BuildCameraShakePoint(const SPrimeCameraShakePoint& point);
CCameraShakerComponent BuildCameraShakerComponent(const SPrimeCameraShakerComponent& component);
CCameraShakeData BuildCameraShakeData(const SPrimeCameraShakeData& data);

struct CMetroidPrimeIceAttack {
  uint x0_propertyCount;
  CAssetId x4_particle1;
  CAssetId x8_particle2;
  CAssetId xc_particle3;
  CDamageInfo x10_dInfo;
  float x2c_;
  float x30_;
  CAssetId x34_texture;
  ushort x38_;
  ushort x3a_;

  explicit CMetroidPrimeIceAttack(CInputStream& in);
};
CHECK_SIZEOF(CMetroidPrimeIceAttack, 0x3C)

struct CMetroidPrimeParasiteQueenAttack {
  CBeamInfo x0_beamInfo;
  uint x44_;
  CDamageInfo x48_dInfo1;
  CWeaponAssetInfo x64_struct5;
  float x88_;
  CDamageInfo x8c_dInfo2;

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

  enum EVulnerabilities {};

  enum EAttackType {

  };
  struct CMetroidPrimeAttackWeights {
    rstl::reserved_vector< float, 14 > mAttackWeights;

    explicit CMetroidPrimeAttackWeights(CInputStream& in);

    float GetAttackWeight(EAttackType attack) const;
    static rstl::reserved_vector< float, 14 > LoadAttackWeights(CInputStream& in);
  };

  struct CVulnerabilityEntry {
    uint x0_propertyCount;
    CDamageVulnerability x4_damageVulnerability;
    CColor x6c_color;
    uint x70_[2];

    explicit CVulnerabilityEntry(CInputStream& in);
  };

  CMetroidPrime(const TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                const CTransform4f& xf, const CModelData& mData, const CPatternedInfo& pInfo,
                const CActorParameters& actorParms, const int pw1,
                const CCameraShakeData& shakeData1, const CCameraShakeData& shakeData2,
                const CCameraShakeData& shakeData3, const CMetroidPrimeIceAttack& iceAttack,
                const CAssetId particle1,
                const rstl::reserved_vector< CMetroidPrimeParasiteQueenAttack, 4 >& breathAttacks,
                const CAssetId weaponDesc1, const CDamageInfo&, const CCameraShakeData& shakeData4,
                const CAssetId weaponDesc2, const CDamageInfo& dInfo2,
                const CCameraShakeData& shakeData5, const CPoisonInfo& poisonInfo,
                const CDamageInfo& dInfo3, const CCameraShakeData& shakeData6,
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

  TUniqueId x568_relayId;
  rstl::single_ptr< CCollisionActorManager > x56c_collisionManager;
  int x570_;
  uint x574_;
  uint x578_;
  uint x57c_;
  uint x580_;
  bool x584_;
  rstl::reserved_vector< CVulnerabilityEntry, 4 > x588_;
  rstl::reserved_vector< CBoneTracking, 6 > x76c_;
  CHealthInfo x8c0_;
  float x8c8_;
  TUniqueId x8cc_headColActor;
  int x8d0_;
  int x8d4_;
  CColor x8d8_beamColor;
  CColor x8dc_;
  CColor x8e0_;
  float x8e4_;
  int x8e8_headUpAdditiveBodyAnimIndex;
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
  rstl::reserved_vector< TUniqueId, 4 > xb24_plasmaProjectileIds;
  rstl::reserved_vector< CWeaponAssetInfo, 4 > xb30_;
  rstl::reserved_vector< rstl::pair< float, CDamageInfo >, 4 > xbc4_;
  TToken< CGenDescription > xc48_;
  rstl::auto_ptr< CElementGen > xc50_;
  int xc58_curPlasmaProjectile;
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
  TUniqueId x1044_billboardId;
  TUniqueId x1046_;
  float x1048_;
  CHealthInfo x104c_;
  bool x1054_24_ : 1;
  bool x1054_25_ : 1;
  bool x1054_26_ : 1;
  bool x1054_27_ : 1;
  rstl::reserved_vector< TEditorId, 4 > x1058_;
  rstl::reserved_vector< TUniqueId, 2 > x106c_energyBallIds;
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
CHECK_SIZEOF(CMetroidPrime, (VERSION >= VERSION_GM8P_00 ? 0x1458 : 0x1448))
NESTED_CHECK_SIZEOF(CMetroidPrime, CMissileTarget, (VERSION >= VERSION_GM8P_00 ? 0x268 : 0x258))
NESTED_CHECK_SIZEOF(CMetroidPrime, CMetroidPrimeAttackWeights, 0x3C)
NESTED_CHECK_SIZEOF(CMetroidPrime, CVulnerabilityEntry, 0x78)

struct CMetroidPrimeData {
  uint x0_propertyCount;
  CPatternedInfo x4_patternedInfo;
  CActorParameters x13c_actorParms;
  int x1a4_;
  CCameraShakeData x1a8_;
  CCameraShakeData x27c_;
  CCameraShakeData x350_;
  CMetroidPrimeIceAttack x424_;
  CAssetId x460_particle1;
  rstl::reserved_vector< CMetroidPrimeParasiteQueenAttack, 4 > x464_;
  CAssetId x708_wpsc1;
  CDamageInfo x70c_dInfo1;
  CCameraShakeData x728_shakeData1;
  CAssetId x7fc_wpsc2;
  CDamageInfo x800_dInfo2;
  CCameraShakeData x81c_shakeData2;
  CPoisonInfo x8f0_;
  CDamageInfo x92c_;
  CCameraShakeData x948_;
  CAssetId xa1c_particle2;
  CAssetId xa20_swoosh;
  CAssetId xa24_particle3;
  CAssetId xa28_particle4;
  rstl::reserved_vector< CMetroidPrime::CVulnerabilityEntry, 4 > xa2c_;

  explicit CMetroidPrimeData(CInputStream& in);
  static uint VerifyExportCount(CInputStream& in);

  const CPatternedInfo& GetPatternedInfo() const { return x4_patternedInfo; }

  static rstl::reserved_vector< CMetroidPrimeParasiteQueenAttack, 4 >
  LoadParasiteQueenBeams(CInputStream& in);
  static rstl::reserved_vector< CMetroidPrime::CVulnerabilityEntry, 4 >
  LoadVulnerabilities(CInputStream& in);
};
CHECK_SIZEOF(CMetroidPrimeData, 0xC10)

#endif // _CMETROIDPRIME
