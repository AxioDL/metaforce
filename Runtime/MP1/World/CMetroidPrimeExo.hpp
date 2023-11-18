#pragma once

#include "Runtime/Camera/CCameraShakeData.hpp"
#include "Runtime/Character/CBoneTracking.hpp"
#include "Runtime/MP1/World/CMetroidPrimeProjectile.hpp"
#include "Runtime/Weapon/CBeamInfo.hpp"
#include "Runtime/Weapon/CPlasmaProjectile.hpp"
#include "Runtime/Weapon/CProjectileInfo.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CPatterned.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/rstl.hpp"

#include <zeus/CColor.hpp>

namespace metaforce {
class CCameraShakeData;
class CCollisionActorManager;
class CGenDescription;
class CElementGen;
class CProjectedShadow;

namespace MP1 {

struct SPrimeStruct2B {
  u32 x0_propertyCount;
  CAssetId x4_particle1;
  CAssetId x8_particle2;
  CAssetId xc_particle3;
  CDamageInfo x10_dInfo;
  float x2c_;
  float x30_;
  CAssetId x34_texture;
  u16 x38_;
  u16 x3a_;
  explicit SPrimeStruct2B(CInputStream& in);
};

struct SPrimeStruct4 {
  CBeamInfo x0_beamInfo;
  u32 x44_;
  CDamageInfo x48_dInfo1;
  CPlasmaProjectile::PlayerEffectResources x64_struct5;
  float x88_;
  CDamageInfo x8c_dInfo2;
  explicit SPrimeStruct4(CInputStream& in);
};

struct SPrimeStruct6 {
  u32 x0_propertyCount;
  CDamageVulnerability x4_damageVulnerability;
  zeus::CColor x6c_color;
  std::array<u32, 2> x70_;
  explicit SPrimeStruct6(CInputStream& in);
};

struct SPrimeExoParameters {
  u32 x0_propertyCount;
  CPatternedInfo x4_patternedInfo;
  CActorParameters x13c_actorParms;
  u32 x1a4_;
  CCameraShakeData x1a8_;
  CCameraShakeData x27c_;
  CCameraShakeData x350_;
  SPrimeStruct2B x424_;
  CAssetId x460_particle1;
  rstl::reserved_vector<SPrimeStruct4, 4> x464_;
  CAssetId x708_wpsc1;
  CDamageInfo x70c_dInfo1;
  CCameraShakeData x728_shakeData1;
  CAssetId x7fc_wpsc2;
  CDamageInfo x800_dInfo2;
  CCameraShakeData x81c_shakeData2;
  SPrimeProjectileInfo x8f0_;
  CDamageInfo x92c_;
  CCameraShakeData x948_;
  CAssetId xa1c_particle2;
  CAssetId xa20_swoosh;
  CAssetId xa24_particle3;
  CAssetId xa28_particle4;
  rstl::reserved_vector<SPrimeStruct6, 4> xa2c_;
  explicit SPrimeExoParameters(CInputStream& in);
};

struct CMetroidPrimeAttackWeights {
  rstl::reserved_vector<float, 14> x0_;
  explicit CMetroidPrimeAttackWeights(CInputStream& in);

  float GetFloatValue(size_t idx) const { return x0_[idx]; }
};

class CMetroidPrimeExo : public CPatterned {
  class CPhysicsDummy : public CPhysicsActor {
  public:
    DEFINE_ENTITY
    CPhysicsDummy(TUniqueId uid, bool active, std::string_view name, const CEntityInfo& info)
    : CPhysicsActor(uid, active, name, info, {}, CModelData::CModelDataNull(),
                    CMaterialList(EMaterialTypes::Target, EMaterialTypes::ExcludeFromRadar), zeus::CAABox{-1.f, 1.f},
                    SMoverData(1.f), CActorParameters::None(), 0.3f, 0.1f) {}
    void Accept(IVisitor& visitor) override;
  };
  TUniqueId x568_relayId = kInvalidUniqueId;
  std::unique_ptr<CCollisionActorManager> x56c_collisionManager;
  s32 x570_ = 1;
  u32 x574_ = 1;
  u32 x578_ = 0;
  u32 x57c_ = 0;
  u32 x580_ = 0;
  bool x584_ = false;
  rstl::reserved_vector<SPrimeStruct6, 4> x588_;
  rstl::reserved_vector<CBoneTracking, 6> x76c_;
  CHealthInfo x8c0_ = CHealthInfo(150.f, 0.f);
  float x8c8_ = 0.f;
  TUniqueId x8cc_headColActor = kInvalidUniqueId;
  u32 x8d0_ = 3;
  u32 x8d4_ = 3;
  zeus::CColor x8d8_beamColor = zeus::skBlack;
  zeus::CColor x8dc_ = zeus::skBlack;
  zeus::CColor x8e0_ = zeus::skBlack;
  float x8e4_ = 0.f;
  s32 x8e8_headUpAdditiveBodyAnimIndex;
  float x8ec_ = 0.f;
  float x8f0_ = 0.f;
  bool x8f4_24_ : 1 = false;
  bool x8f4_25_ : 1 = false;
  bool x8f4_26_ : 1 = false;
  bool x8f4_27_ : 1 = false;
  bool x8f4_28_ : 1 = false;
  zeus::CAABox x8f8_;
  float x910_ = 5.f;
  bool x914_24_ : 1 = false;
  s32 x918_ = -1;
  s32 x91c_;
  float x920_ = 0.f;
  float x924_ = 4.f;
  float x928_ = 5.f;
  u32 x92c_ = 0;
  SPrimeStruct2B x930_;
  rstl::reserved_vector<CBeamInfo, 4> x96c_;
  rstl::reserved_vector<CProjectileInfo, 4> xa80_;
  rstl::reserved_vector<TUniqueId, 4> xb24_plasmaProjectileIds = {
      {kInvalidUniqueId, kInvalidUniqueId, kInvalidUniqueId, kInvalidUniqueId}};
  rstl::reserved_vector<CPlasmaProjectile::PlayerEffectResources, 4> xb30_;
  rstl::reserved_vector<CDamageInfo, 4> xbc4_;
  TLockedToken<CGenDescription> xc48_;
  std::unique_ptr<CElementGen> xc50_;
  s32 xc58_curPlasmaProjectile = -1;
  float xc5c_ = 0.f;
  zeus::CVector3f xc60_;
  zeus::CVector3f xc6c_;
  CProjectileInfo xc78_;
  CCameraShakeData xca0_;
  CProjectileInfo xd74_;
  CCameraShakeData xd9c_;
  SPrimeProjectileInfo xe70_;
  TUniqueId xeac_ = kInvalidUniqueId;
  u32 xeb0_ = 0;
  CDamageInfo xeb4_;
  CCameraShakeData xed0_;
  TLockedToken<CElectricDescription> xfa4_;
  std::unique_ptr<CParticleElectric> xfac_;
  float xfb4_ = 0.f;
  float xfb8_ = 0.f;
  CSfxHandle xfbc_;
  bool xfc0_ = false;
  bool xfc1_ = false;
  rstl::reserved_vector<TToken<CGenDescription>, 2> xfc4_;
  rstl::reserved_vector<TToken<CSwooshDescription>, 2> xfd8_;
  rstl::reserved_vector<std::unique_ptr<CElementGen>, 2> xfec_;
  rstl::reserved_vector<std::unique_ptr<CParticleSwoosh>, 2> x1000_;
  TToken<CGenDescription> x1014_;
  TToken<CGenDescription> x101c_;
  std::unique_ptr<CElementGen> x1024_;
  rstl::reserved_vector<float, 2> x102c_;
  rstl::reserved_vector<float, 2> x1038_;
  TUniqueId x1044_billboardId = kInvalidUniqueId;
  TUniqueId x1046_ = kInvalidUniqueId;
  float x1048_ = 0.f;
  float x104c_ = 75.f;
  float x1050_ = 0.f;
  bool x1054_24_ : 1 = false;
  bool x1054_25_ : 1 = false;
  bool x1054_26_ : 1 = false;
  bool x1054_27_ : 1 = false;
  rstl::reserved_vector<TEditorId, 4> x1058_;
  rstl::reserved_vector<TUniqueId, 2> x106c_energyBallIds;
  float x1074_ = 0.f;
  s32 x1078_ = -1;
  float x107c_ = 0.f;
  float x1080_;
  float x1084_ = 0.f;
  float x1088_ = 0.f;
  CCameraShakeData x108c_;
  rstl::reserved_vector<CMetroidPrimeAttackWeights, 4> x1160_;
  s32 x1254_ = -1;
  rstl::reserved_vector<float, 14> x1258_;
  CCameraShakeData x1294_;
  CCameraShakeData x1368_;
  std::unique_ptr<CProjectedShadow> x143c_;
  s32 x1440_ = 0;
  bool x1444_24_ : 1 = false;
  bool x1444_25_ : 1 = false;

  void sub802738d4(CStateManager& mgr);
  void UpdateEnergyBalls(float dt, CStateManager& mgr);
  u32 CountEnergyBalls(CStateManager& mgr);
  void DeactivatePatrolObjects(CStateManager& mgr);
  void UpdatePhysicsDummy(CStateManager& mgr);
  void sub80274054(CStateManager& mgr);
  void sub802740cc(CStateManager& mgr);
  void CreatePhysicsDummy(CStateManager& mgr);
  void SetBillboardEmission(CStateManager& mgr, bool emission);
  void FreeBillboard(CStateManager& mgr);
  zeus::CVector3f sub8027464c(CStateManager& mgr);
  void CreateHUDBillBoard(CStateManager& mgr);
  void sub802747b8(float f1, CStateManager& mgr, const zeus::CVector3f& vec);
  void sub802749e8(float f1, float f2, float f3, const zeus::CVector3f& vec1, const zeus::CVector3f& vec2, s32 idx);
  void UpdateParticles(float f1, CStateManager& mgr);
  void EnableParticles(CStateManager& mgr, bool b1);
  void EnableHeadOrbitAndTarget(CStateManager& mgr);
  void DisableHeadOrbitAndTarget(CStateManager& mgr);
  void UpdateTimers(float mgr);
  void sub80275800(CStateManager& mgr);
  void sub802759a8(CStateManager& mgr, u32 w1);
  float sub80275b04(const CMetroidPrimeAttackWeights& roomParms, int w2);
  void sub80275b68();
  void sub80275c60(CStateManager& mgr, int w1);
  bool sub80275d68(int w1);
  pas::ELocomotionType sub80275e14(int w1);
  u32 sub80275e34(int w1) const;
  void UpdateElectricEffect(float dt, CStateManager& mgr);
  void UpdateSfxEmitter(float f1, CStateManager& mgr);
  void sub80276204(CStateManager& mgr, bool b1);
  void sub8027639c(CStateManager& mgr, bool b1);
  void SetActorAreaId(CStateManager& mgr, TUniqueId uid, TAreaId aid);
  void UpdateAreaId(CStateManager& mgr);
  void SendStateToRelay(EScriptObjectState state, CStateManager& mgr);
  void GetRelayState(CStateManager& mgr);
  TUniqueId GetNextAttackWaypoint(CStateManager& mgr, bool b1);
  TUniqueId GetWaypointForBehavior(CStateManager& mgr, EScriptObjectState state, EScriptObjectMessage msg);
  void UpdateRelay(CStateManager& mgr, TAreaId areaId);
  bool IsRelayValid(CStateManager& mgr, TAreaId w2);
  bool sub80277224(float f1, CStateManager& mgr);
  void FirePlasmaProjectile(CStateManager& mgr, bool b1);
  void UpdatePlasmaProjectile(float dt, CStateManager& mgr);
  zeus::CVector3f GetTargetVector(CStateManager& mgr);
  void FreePlasmaProjectiles(CStateManager& mgr);
  void CreatePlasmaProjectiles(CStateManager& mgr);
  void UpdateContactDamage(CStateManager& mgr);
  void UpdateColorChange(float f1, CStateManager& mgr);
  void sub80278130(const zeus::CColor& col);
  void UpdateHeadAnimation(float f1);
  void sub8027827c(TUniqueId uid, CStateManager& mgr);
  void sub80278508(CStateManager& mgr, int w1, bool b1);
  void sub802786fc(CStateManager& mgr);
  void SetEyesParticleEffectState(CStateManager& mgr, bool b);
  void UpdateHeadHealthInfo(CStateManager& mgr);
  void UpdateHealthInfo(CStateManager& mgr);
  void SetBoneTrackingTarget(CStateManager& mgr, bool active);
  void UpdateBoneTracking(float f1, CStateManager& mgr);
  void DoContactDamage(TUniqueId uid, CStateManager& mgr);
  void UpdateCollision(float dt, CStateManager& mgr);
  void SetupBoneTracking();
  void SetupCollisionActorManager(CStateManager& mgr);

public:
  DEFINE_PATTERNED(MetroidPrimeExo);
  CMetroidPrimeExo(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                   CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& aParms, u32 pw1,
                   const CCameraShakeData& shakeData1, const CCameraShakeData& shakeData2,
                   const CCameraShakeData& shakeData3, const SPrimeStruct2B& struct2b, CAssetId particle1,
                   const rstl::reserved_vector<SPrimeStruct4, 4>& struct4s, CAssetId wpsc1, const CDamageInfo& dInfo1,
                   const CCameraShakeData& shakeData4, CAssetId wpsc2, const CDamageInfo& dInfo2,
                   const CCameraShakeData& shakeData5, const SPrimeProjectileInfo& projectileInfo,
                   const CDamageInfo& dInfo3, const CCameraShakeData& shakeData6, CAssetId particle2, CAssetId swoosh,
                   CAssetId particle3, CAssetId particle4, const rstl::reserved_vector<SPrimeStruct6, 4>& struct6s);

  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId other, CStateManager& mgr) override;
  void PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) override;
  void AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) override;
  void Render(CStateManager& mgr) override;
  bool CanRenderUnsorted(const CStateManager& mgr) const override;
  void Touch(CActor& act, CStateManager& mgr) override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) override;
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
  bool ShouldJumpBack(CStateManager& mgr, float arg) override { return x1254_ == 11; }
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
  CProjectileInfo* GetProjectileInfo() override;
};

} // namespace MP1
} // namespace metaforce
