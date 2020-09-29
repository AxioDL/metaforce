#pragma once

#include "Runtime/Collision/CJointCollisionDescription.hpp"
#include "Runtime/Character/CBoneTracking.hpp"
#include "Runtime/Weapon/CProjectileInfo.hpp"
#include "Runtime/World/CPathFindSearch.hpp"
#include "Runtime/World/CPatterned.hpp"

namespace urde {
class CCollisionActorManager;
class CElementGen;
class CParticleElectric;
namespace MP1 {
class CIceSheegothData {
  float x0_;
  float x4_;
  zeus::CVector3f x8_;
  float x14_;
  CDamageVulnerability x18_;
  CDamageVulnerability x80_;
  CDamageVulnerability xe8_;
  CAssetId x150_;
  CDamageInfo x154_;
  float x170_;
  float x174_;
  CAssetId x178_;
  CAssetId x17c_fireBreathResId;
  CDamageInfo x180_fireBreathDamage;
  CAssetId x19c_;
  CAssetId x1a0_;
  CAssetId x1a4_;
  CAssetId x1a8_;
  CAssetId x1ac_;
  float x1b0_;
  float x1b4_;
  CDamageInfo x1b8_;
  s16 x1d4_;
  float x1d8_;
  float x1dc_;
  float x1e0_maxInterestTime;
  CAssetId x1e4_;
  s16 x1e8_;
  CAssetId x1ec_;
  bool x1f0_24_ : 1;
  bool x1f0_25_ : 1;

public:
  CIceSheegothData(CInputStream& in, s32 propertyCount);
  [[nodiscard]] float Get_x0() const { return x0_; }
  [[nodiscard]] float Get_x4() const { return x4_; }
  [[nodiscard]] zeus::CVector3f Get_x8() const { return x8_; }
  [[nodiscard]] float Get_x14() const { return x14_; }
  [[nodiscard]] CDamageVulnerability Get_x18() const { return x18_; }
  [[nodiscard]] CDamageVulnerability Get_x80() const { return x80_; }
  [[nodiscard]] CDamageVulnerability Get_xe8() const { return xe8_; }
  [[nodiscard]] CAssetId Get_x150() const { return x150_; }
  [[nodiscard]] CDamageInfo Get_x154() const { return x154_; }
  [[nodiscard]] float Get_x170() const { return x170_; }
  [[nodiscard]] float Get_x174() const { return x174_; }
  [[nodiscard]] CAssetId Get_x178() const { return x178_; }
  [[nodiscard]] CAssetId GetFireBreathResId() const { return x17c_fireBreathResId; }
  [[nodiscard]] CDamageInfo GetFireBreathDamage() const { return x180_fireBreathDamage; }
  [[nodiscard]] CAssetId Get_x19c() const { return x19c_; }
  [[nodiscard]] CAssetId Get_x1a0() const { return x1a0_; }
  [[nodiscard]] CAssetId Get_x1a4() const { return x1a4_; }
  [[nodiscard]] CAssetId Get_x1a8() const { return x1a8_; }
  [[nodiscard]] CAssetId Get_x1ac() const { return x1ac_; }
  [[nodiscard]] float Get_x1b0() const { return x1b0_; }
  [[nodiscard]] CDamageInfo Get_x1b8() const { return x1b8_; }
  [[nodiscard]] s16 Get_x1d4() const { return x1d4_; }
  [[nodiscard]] float Get_x1d8() const { return x1d8_; }
  [[nodiscard]] float Get_x1dc() const { return x1dc_; }
  [[nodiscard]] float GetMaxInterestTime() const { return x1e0_maxInterestTime; }
  [[nodiscard]] CAssetId Get_x1e4() const { return x1e4_; }
  [[nodiscard]] s16 Get_x1e8() const { return x1e8_; }
  [[nodiscard]] CAssetId Get_x1ec() const { return x1ec_; }
  [[nodiscard]] bool Get_x1f0_24() const { return x1f0_24_; }
  [[nodiscard]] bool Get_x1f0_25() const { return x1f0_25_; }
};

class CIceSheegoth : public CPatterned {
  enum class EPathFindMode { Normal, Approach };
  s32 x568_ = -1;
  CIceSheegothData x56c_sheegothData;
  CPathFindSearch x760_pathSearch;
  CPathFindSearch x844_approachSearch;
  EPathFindMode x928_pathFindMode = EPathFindMode::Normal;
  zeus::CVector3f x92c_lastDest = zeus::skZero3f;
  zeus::CVector3f x938_ = zeus::skZero3f;
  float x944_ = 1.f;
  float x948_ = 1.f;
  float x94c_;
  float x950_ = 0.f;
  float x954_attackTimeLeft = 0.f;
  float x958_ = 0.f;
  float x95c_ = 0.f;
  float x960_ = 0.f;
  /* x964_ */
  float x968_interestTimer = 0.f;
  float x96c_ = 2.f;
  float x970_maxHp = 0.f;
  float x974_;
  float x978_ = 0.f;
  float x97c_ = 0.f;
  zeus::CVector3f x980_ = zeus::skZero3f;
  CDamageVulnerability x98c_mouthVulnerability;
  CBoneTracking x9f4_boneTracking;
  std::unique_ptr<CCollisionActorManager> xa2c_collisionManager;
  CCollidableAABox xa30_;
  CProjectileInfo xa58_;
  TUniqueId xa80_flameThrowerId = kInvalidUniqueId;
  TToken<CWeaponDescription> xa84_;
  TCachedToken<CGenDescription> xa8c_;
  // bool xa98_;
  std::unique_ptr<CElementGen> xa9c_;
  TCachedToken<CGenDescription> xaa0_;
  // bool xaac_;
  std::unique_ptr<CElementGen> xab0_;
  TCachedToken<CGenDescription> xab4_;
  // bool xac0_;
  std::unique_ptr<CElementGen> xac4_;
  TCachedToken<CElectricDescription> xac8_;
  // bool xad4_;
  std::unique_ptr<CParticleElectric> xad8_;
  TCachedToken<CGenDescription> xadc_;
  // bool xae8_;
  std::unique_ptr<CElementGen> xaec_;
  CSfxHandle xaf0_crackleSfx;
  CSegId xaf4_mouthLocator;
  TUniqueId xaf6_iceShardsCollider = kInvalidUniqueId;
  TUniqueId xaf8_mouthCollider = kInvalidUniqueId;
  rstl::reserved_vector<TUniqueId, 2> xafc_gillColliders;
  rstl::reserved_vector<TUniqueId, 10> xb04_;
  rstl::reserved_vector<CSegId, 7> xb1c_;
  bool xb28_24_shotAt : 1 = false;
  bool xb28_25_ : 1 = false;
  bool xb28_26_ : 1 = false;
  bool xb28_27_ : 1 = false;
  bool xb28_28_ : 1 = false;
  bool xb28_29_ : 1 = false;
  bool xb28_30_ : 1 = false;
  bool xb28_31_ : 1 = false;
  bool xb29_24_ : 1 = false;
  bool xb29_25_ : 1 = false;
  bool xb29_26_ : 1 = false;
  bool xb29_27_ : 1 = false;
  bool xb29_28_ : 1 = false;
  bool xb29_29_scanned : 1 = false;

  void UpdateTouchBounds();
  [[nodiscard]] bool IsMouthCollider(TUniqueId uid) const { return xaf8_mouthCollider == uid; }
  [[nodiscard]] bool IsGillCollider(const CEntity* ent) const {
    return std::find_if(xafc_gillColliders.cbegin(), xafc_gillColliders.cend(),
                        [&ent](TUniqueId uid) { return uid == ent->GetUniqueId(); }) != xafc_gillColliders.cend();
  }
  void sub_8019ebf0(CStateManager& mgr, float damage);
  void ApplyWeaponDamage(CStateManager& mgr, TUniqueId sender);
  void CreateFlameThrower(CStateManager& mgr);
  void ApplyContactDamage(TUniqueId sender, CStateManager& mgr);
  void AddSphereCollisionList(const SSphereJointInfo* info, size_t count,
                              std::vector<CJointCollisionDescription>& vecOut);
  void AddCollisionList(const SJointInfo* info, size_t count, std::vector<CJointCollisionDescription>& vecOut);
  void SetupCollisionActorManager(CStateManager& mgr);
  void SetupHealthInfo(CStateManager& mgr);
  void AttractProjectiles(CStateManager& mgr);
  void UpdateTimers(float dt);
  void UpdateScanState(CStateManager& mgr);
  void SetPassthroughVulnerability(CStateManager& mgr);
  void PreventWorldCollisions(float dt, CStateManager& mgr);
  void UpdateHealthInfo(CStateManager& mgr);
  void SetSteeringSpeed(float dt, CStateManager& mgr);
  void SetPathFindMode(EPathFindMode mode) { x928_pathFindMode = mode; }
  void UpdateParticleEffects(float dt, CStateManager& mgr);
  bool sub_801a1794(CStateManager& mgr) const {
    const CHealthInfo* hInfo = GetHealthInfo(mgr);
    return hInfo != nullptr && hInfo->GetHP() < 0.3f * x970_maxHp;
  }

  bool sub_8019ecbc() const { return xb28_27_ || xb29_26_; }
  bool sub_8019ecdc(CStateManager& mgr, float minAngle);
  void SetMouthVulnerability(CStateManager& mgr, bool isVulnerable);
  void SetGillVulnerability(CStateManager& mgr, bool isVulnerable);
  void ShakePlayer(CStateManager& mgr);
  void SetCollisionActorExtendedTouchBounds(CStateManager& mgr, const zeus::CVector3f& extents);

  void UpdateAttackPosition(CStateManager& mgr, zeus::CVector3f& attackPos);
  zeus::CVector3f GetEnergyAttractionPos(CStateManager& mgr) const;
  bool ShouldAttractProjectile(const CGameProjectile& proj, CStateManager& mgr) const;
  bool IsClosestSheegoth(CStateManager& mgr, const rstl::reserved_vector<TUniqueId, 1024>& nearList,
                         const zeus::CVector3f& projectileOffset) const;

public:
  DEFINE_PATTERNED(IceSheeegoth);
  CIceSheegoth(TUniqueId uid, std::string_view name, const CEntityInfo& info, zeus::CTransform& xf, CModelData&& mData,
               const CPatternedInfo& pInfo, const CActorParameters& actorParameters,
               const CIceSheegothData& sheegothData);

  void Accept(IVisitor& visitor) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) override;
  void AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) override;
  [[nodiscard]] const CDamageVulnerability* GetDamageVulnerability() const override {
    return &CDamageVulnerability::PassThroughVulnerabilty();
  }
  [[nodiscard]] const CDamageVulnerability* GetDamageVulnerability(const zeus::CVector3f&, const zeus::CVector3f&,
                                                                   const CDamageInfo&) const override {
    return &CDamageVulnerability::PassThroughVulnerabilty();
  }

  [[nodiscard]] zeus::CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  [[nodiscard]] EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&,
                                                                       const CWeaponMode&,
                                                                       EProjectileAttrib) const override;
  [[nodiscard]] zeus::CAABox GetSortingBounds(const CStateManager& mgr) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) override;
  [[nodiscard]] const CCollisionPrimitive* GetCollisionPrimitive() const override { return &xa30_; }
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Generate(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Deactivate(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Attack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void DoubleSnap(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TurnAround(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Crouch(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Taunt(CStateManager& mgr, EStateMsg msg, float dt) override;
  void ProjectileAttack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Flinch(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Approach(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Enraged(CStateManager& mgr, EStateMsg msg, float dt) override;
  void SpecialAttack(CStateManager& mgr, EStateMsg msg, float dt) override;
  bool Leash(CStateManager& mgr, float arg) override;
  bool OffLine(CStateManager& mgr, float arg) override;
  bool TooClose(CStateManager& mgr, float arg) override;
  bool InMaxRange(CStateManager& mgr, float arg) override;
  bool InDetectionRange(CStateManager& mgr, float arg) override;
  bool SpotPlayer(CStateManager& mgr, float arg) override;
  bool AnimOver(CStateManager& mgr, float arg) override;
  bool ShouldAttack(CStateManager& mgr, float arg) override;
  bool ShouldDoubleSnap(CStateManager& mgr, float arg) override;
  bool InPosition(CStateManager& mgr, float arg) override;
  bool ShouldTurn(CStateManager& mgr, float arg) override;
  bool AggressionCheck(CStateManager& mgr, float arg) override;
  bool ShouldFire(CStateManager& mgr, float arg) override;
  bool ShouldFlinch(CStateManager& mgr, float arg) override;
  bool ShotAt(CStateManager& mgr, float arg) override;
  bool ShouldSpecialAttack(CStateManager& mgr, float arg) override;
  bool LostInterest(CStateManager& mgr, float arg) override;
  CPathFindSearch* GetSearchPath() override {
    return x928_pathFindMode == EPathFindMode::Normal ? &x760_pathSearch : &x844_approachSearch;
  }
  [[nodiscard]] float GetGravityConstant() const override { return 10.f * 24.525f; }
  CProjectileInfo* GetProjectileInfo() override { return &xa58_; }
};
} // namespace MP1
} // namespace urde
