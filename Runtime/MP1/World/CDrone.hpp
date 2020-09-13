#pragma once

#include "Runtime/Collision/CCollidableSphere.hpp"
#include "Runtime/World/CPatterned.hpp"
#include "Runtime/World/CPathFindSearch.hpp"
#include "Runtime/World/CVisorFlare.hpp"

namespace urde {
class CWeaponDescription;
namespace MP1 {
class CDrone : public CPatterned {
  CAssetId x568_;
  TLockedToken<CCollisionResponseData> x56c_;
  TUniqueId x578_lightId = kInvalidUniqueId;
  TUniqueId x57a_ = kInvalidUniqueId;
  std::vector<CVisorFlare::CFlareDef> x57c_flares;
  pas::EStepDirection x58c_prevDodgeDir = pas::EStepDirection::Left;
  CDamageInfo x590_;
  CDamageInfo x5ac_;
  float x5c8_ = 0.f;
  float x5cc_ = 0.f;
  float x5d0_ = 0.f;
  float x5d4_ = 0.f;
  float x5d8_ = 0.f;
  float x5dc_ = 0.f;
  float x5e0_ = 0.f;
  float x5e4_;
  mutable float x5e8_shieldTime = 0.f;
  float x5ec_turnSpeed;
  float x5f0_;
  float x5f4_;
  float x5f8_;
  float x5fc_;
  float x600_;
  float x604_ = 0.f;
  float x608_;
  float x60c_;
  float x610_;
  float x614_;
  float x618_;
  float x61c_;
  float x620_;
  float x624_ = 0.f;
  float x628_ = 0.f;
  float x62c_ = 0.f;
  float x630_ = 0.f;
  float x634_ = 0.f;
  float x638_ = 0.f;
  float x63c_;
  float x640_;
  float x644_ = 0.f;
  float x648_;
  float x64c_;
  float x650_;
  float x654_;
  float x658_;
  float x65c_;
  float x660_;
  float x664_;
  float x668_ = 0.f;
  float x66c_ = 0.f;
  zeus::CVector3f x670_;
  zeus::CVector3f x67c_;
  TUniqueId x688_teamMgr = kInvalidUniqueId;
  CCollidableSphere x690_colSphere;
  CPathFindSearch x6b0_pathFind;
  zeus::CAxisAngle x794_;
  zeus::CVector3f x7a0_;
  zeus::CVector3f x7ac_lightPos;
  float x7b8_ = 0.f;
  float x7bc_ = 0.f;
  float x7c0_ = 0.f;
  float x7c4_ = 0.f;
  s32 x7c8_ = 0;
  s16 x7cc_;
  CSfxHandle x7d0_;
  rstl::reserved_vector<TUniqueId, 2> x7d8_ = {{kInvalidUniqueId, kInvalidUniqueId}};
  rstl::reserved_vector<zeus::CVector3f, 2> x7e0_ = {{zeus::skZero3f, zeus::skZero3f}};
  rstl::reserved_vector<zeus::CVector3f, 2> x7fc_ = {{zeus::skZero3f, zeus::skZero3f}};
  rstl::reserved_vector<float, 2> x818_ = {{0.f, 0.f}};
  rstl::reserved_vector<bool, 2> x824_activeLasers = {{false, false}};
  std::unique_ptr<CModelData> x82c_shieldModel;
  u8 x832_a : 3 = 0;
  u8 x832_b : 3 = 0;
  bool x834_24_ : 1 = false;
  bool x834_25_ : 1 = false;
  bool x834_26_ : 1 = false;
  bool x834_27_ : 1 = false;
  bool x834_28_ : 1 = false;
  bool x834_29_codeTrigger : 1 = false;
  bool x834_30_visible : 1 = false;
  bool x834_31_attackOver : 1 = false;
  bool x835_24_ : 1 = false;
  bool x835_25_ : 1;
  bool x835_26_ : 1 = false;

  void UpdateTouchBounds(float radius);
  bool HitShield(const zeus::CVector3f& dir) const;
  void AddToTeam(CStateManager& mgr) const;
  void RemoveFromTeam(CStateManager& mgr) const;
  void UpdateLaser(CStateManager& mgr, u32 laserIdx, bool b1);
  void FireProjectile(CStateManager& mgr, const zeus::CTransform& xf, const TToken<CWeaponDescription>& weapon);
  void StrafeFromCompanions(CStateManager& mgr);
  void UpdateScanner(CStateManager& mgr, float dt);

  void sub_80163c40(float, CStateManager& mgr);
  void sub_801633a8(CStateManager& mgr);
  void sub_8015f25c(float dt, CStateManager& mgr);
  void sub_8015f158(float dt);
  void sub_80165984(CStateManager& mgr, const zeus::CTransform& xf);

public:
  DEFINE_PATTERNED(Drone);
  CDrone(TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo& info, const zeus::CTransform& xf,
         float f1, CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms,
         EMovementType movement, EColliderType colliderType, EBodyType bodyType, const CDamageInfo& dInfo1,
         CAssetId aId1, const CDamageInfo& dInfo2, CAssetId aId2, std::vector<CVisorFlare::CFlareDef> flares, float f2,
         float f3, float f4, float f5, float f6, float f7, float f8, float f9, float f10, float f11, float f12,
         float f13, float f14, float f15, float f16, float f17, float f18, float f19, float f20, CAssetId crscId,
         float f21, float f22, float f23, float f24, s32 w3, bool b1);

  void Accept(IVisitor& visitor) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) override;
  void AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) override;
  void PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) override;
  void Render(CStateManager& mgr) override;
  bool CanRenderUnsorted(const CStateManager& mgr) const override;
  const CDamageVulnerability* GetDamageVulnerability() const override { return CAi::GetDamageVulnerability(); }
  const CDamageVulnerability* GetDamageVulnerability(const zeus::CVector3f&, const zeus::CVector3f&,
                                                     const CDamageInfo&) const override;
  void Touch(CActor& act, CStateManager& mgr) override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&,
                                                         const CWeaponMode&, EProjectileAttrib) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) override;
  const CCollisionPrimitive* GetCollisionPrimitive() const override;
  void Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state) override;
  void KnockBack(const zeus::CVector3f&, CStateManager&, const CDamageInfo& info, EKnockBackType type, bool inDeferred,
                 float magnitude) override;
  void Patrol(CStateManager&, EStateMsg msg, float dt) override;
  void PathFind(CStateManager&, EStateMsg msg, float dt) override;
  void TargetPlayer(CStateManager&, EStateMsg msg, float dt) override;
  void TargetCover(CStateManager&, EStateMsg msg, float dt) override;
  void Deactivate(CStateManager&, EStateMsg msg, float dt) override;
  void Attack(CStateManager&, EStateMsg msg, float dt) override;
  void Active(CStateManager&, EStateMsg msg, float dt) override;
  void Flee(CStateManager&, EStateMsg msg, float dt) override;
  void ProjectileAttack(CStateManager&, EStateMsg msg, float dt) override;
  void TelegraphAttack(CStateManager&, EStateMsg msg, float dt) override;
  void Dodge(CStateManager&, EStateMsg msg, float dt) override;
  void Retreat(CStateManager&, EStateMsg msg, float dt) override;
  void Cover(CStateManager&, EStateMsg msg, float dt) override;
  void SpecialAttack(CStateManager&, EStateMsg msg, float dt) override;
  void PathFindEx(CStateManager&, EStateMsg msg, float dt) override;
  bool Leash(CStateManager&, float arg) override;
  bool InRange(CStateManager&, float arg) override;
  bool SpotPlayer(CStateManager&, float arg) override;
  bool AnimOver(CStateManager&, float arg) override;
  bool AttackOver(CStateManager& mgr, float arg) override;
  bool ShouldAttack(CStateManager&, float arg) override;
  bool HearShot(CStateManager&, float arg) override;
  bool CoverCheck(CStateManager&, float arg) override;
  bool LineOfSight(CStateManager&, float arg) override;
  bool ShouldMove(CStateManager&, float arg) override;
  bool CodeTrigger(CStateManager&, float arg) override;
  void Burn(float duration, float damage) override;
  CPathFindSearch* GetSearchPath() override;
  virtual void BuildNearList(EMaterialTypes includeMat, EMaterialTypes excludeMat,
                             rstl::reserved_vector<TUniqueId, 1024>& listOut, float radius, CStateManager& mgr);
  virtual void SetLightEnabled(CStateManager& mgr, bool activate);
  virtual void SetVisorFlareEnabled(CStateManager& mgr, bool activate);
  virtual void UpdateVisorFlare(CStateManager& mgr);
  virtual int sub_8015f150() { return 3; }
};
} // namespace MP1
} // namespace urde
