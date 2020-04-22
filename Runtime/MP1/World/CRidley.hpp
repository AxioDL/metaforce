#pragma once

#include <string_view>

#include "Runtime/Camera/CCameraShakeData.hpp"
#include "Runtime/Character/CBoneTracking.hpp"
#include "Runtime/Collision/CCollisionActorManager.hpp"
#include "Runtime/Weapon/CBeamInfo.hpp"
#include "Runtime/Weapon/CProjectileInfo.hpp"
#include "Runtime/World/CDamageInfo.hpp"
#include "Runtime/World/CPatterned.hpp"
#include "Runtime/World/CProjectedShadow.hpp"

namespace urde {
class CParticleElectric;
namespace MP1 {
class CRidleyData {
  friend class CRidley;
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
  u16 xa8_;
  CAssetId xac_;
  CDamageInfo xb0_;
  CCameraShakeData xcc_;
  CAssetId x1a0_;
  CDamageInfo x1a4_;
  CCameraShakeData x1c0_;
  u16 x294_;
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
  u16 x3f8_;
  CDamageInfo x3fc_;

public:
  CRidleyData(CInputStream&, u32);
};

class CRidley : public CPatterned {
  CRidleyData x568_data;
  std::unique_ptr<CCollisionActorManager> x980_tailCollision;
  std::unique_ptr<CCollisionActorManager> x984_bodyCollision;
  TUniqueId x988_headId = kInvalidUniqueId;
  TUniqueId x98a_breastPlateId = kInvalidUniqueId;
  TLockedToken<CGenDescription> x98c_;
  CModelData x998_;
  CModelData x9e4_;
  CSegId xa30_breastPlateSegId;
  bool xa31_24_ : 1 = true;
  bool xa31_25_ : 1 = true;
  bool xa31_26_ : 1 = false;
  bool xa31_27_ : 1 = false;
  bool xa31_28_ : 1 = false;
  bool xa31_29_ : 1 = false;
  bool xa31_30_ : 1 = false;
  bool xa31_31_ : 1 = false;
  bool xa32_24_ : 1 = false;
  bool xa32_25_ : 1 = false;
  bool xa32_26_ : 1 = false;
  bool xa32_27_ : 1 = false;
  bool xa32_28_shotAt : 1 = false;
  bool xa32_29_ : 1 = false;
  bool xa32_30_ : 1 = false;
  bool xa32_31_ : 1 = true;
  bool xa33_24_ : 1 = false;
  bool xa33_25_ : 1 = true;
  bool xa33_26_ : 1 = false;
  bool xa33_27_ : 1 = true;
  bool xa33_28_ : 1 = false;
  bool xa33_29_doStrafe : 1 = false;
  bool xa33_30_ : 1 = false;
  bool xa33_31_ : 1 = false;
  bool xa34_24_ : 1 = false;
  bool xa34_25_ : 1 = false;
  bool xa34_26_ : 1 = false;
  CModelData xa38_;
  zeus::CTransform xa84_;
  float xab4_ = 20.f;
  float xab8_ = 12.f;
  float xabc_ = 40.f;
  float xac0_ = 10;
  zeus::CAABox xac4_ = zeus::skInvertedBox;
  float xadc_;
  float xae0_;
  u32 xae4_;
  float xae8_;
  zeus::CVector3f xaec_;
  zeus::CVector3f xaf8_;
  s32 xb04_ = 2;
  u32 xb08_;
  u32 xb0c_ = 0;
  float xb10_ = 0.f;
  float xb14_;
  float xb18_;
  float xb1c_;
  float xb20_ = 0.f;
  float xb24_ = 0.f;
  CSegId xb28_;
  CBoneTracking xb2c_;
  TUniqueId xb64_plasmaProjectile = kInvalidUniqueId;
  CProjectileInfo xb68_;
  CSegId xb90_headSegId;
  CSegId xb91_mouthSegId;
  u8 xb92_;
  u8 xb93_;
  zeus::CTransform xb94_;
  zeus::CVector3f xbc4_;
  zeus::CVector3f xbd0_;
  float xbdc_;
  float xbe0_;
  zeus::CVector3f xbe4_;
  zeus::CVector3f xbf0_ = zeus::skForward;
  zeus::CVector3f xbfc_;
  float xc08_ = 0.f;
  float xc0c_ = 0.f;
  float xc10_ = 120.f;
  CProjectileInfo xc14_;
  CProjectileInfo xc3c_;
  u32 xc64_aiStage = 2;
  zeus::CVector3f xc68_;
  u32 xc74_ = 0;
  float xc78_ = 0.f;
  float xc7c_ = 0.f;
  float xc80_ = 0.f;
  u32 xc84_;
  u32 xc88_ = 4;
  CDamageInfo xc8c_;
  CSfxHandle xca8_;
  CSfxHandle xcac_ = 0;
  u32 xcb0_ = 0;
  u32 xcb4_ = 0;
  float xcb8_ = 0.f;
  float xcbc_ = 0.f;
  u32 xcc0_ = 1;
  u32 xcc4_ = 1;
  float xcc8_ = 0.f;
  float xccc_;
  TLockedToken<CElectricDescription> xcd0_;
  std::unique_ptr<CParticleElectric> xce0_;
  std::vector<CSegId> xce4_wingBoneIds; // was rstl::reserved_vector<CSegId, 30>
  float xd08_;
  CSfxHandle xd0c_;
  std::unique_ptr<CProjectedShadow> xd10_;
  u32 xd14_;

  void SetupCollisionActorManager(CStateManager& mgr);
  void SetupCollisionActors(CStateManager& mgr);

  void sub80257650(CStateManager& mgr);
  void sub80256914(float f1, bool r4);
  void sub802560d0(float dt);
  void sub802563a8(float dt);
  void sub80256b14(float dt, CStateManager& mgr);
  void sub80256624(float dt, CStateManager& mgr);
  void ResetPlasmaProjectile(CStateManager& mgr, bool b1);
  void sub80255fe8(float f1, float f2, const zeus::CVector3f& vec);
  void sub80255e5c(CStateManager& mgr);
  void sub8025784c(CStateManager& mgr);
  void sub80255d58(CStateManager& mgr);
  void sub80257744(CStateManager& mgr);
  void FirePlasma(CStateManager& mgr);
  void FacePlayer(float arg, CStateManager& mgr);
  void SetSphereCollisionRadius(float f1, CStateManager& mgr);
  void sub80256580() {
    if (!xa31_24_)
      x3b4_speed = 1.2f;
  }

  void sub80253710(CStateManager& mgr);
  bool sub80253960() {
    return (GetTranslation()  + (1.75f * GetTransform().basis[1]) - xa84_.origin).magnitude() < 1.75f * (xab4_ + xabc_);
  }

public:
  DEFINE_PATTERNED(Ridley)
  CRidley(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&, const CPatternedInfo&,
          const CActorParameters&, CInputStream&, u32);

  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) override;
  void Render(CStateManager& mgr) override;
  void AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) override;
  zeus::CAABox GetSortingBounds(const CStateManager&) const override { return GetBoundingBox(); }
  const CDamageVulnerability* GetDamageVulnerability() const override {
    return &CDamageVulnerability::ImmuneVulnerabilty();
  }

  zeus::CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override {
    return GetLctrTransform((xc64_aiStage == 3 && !xa32_28_shotAt) ? xb90_headSegId : xa30_breastPlateSegId).origin;
  }

  float GetGravityConstant() const override { return 50.f; }
  EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&,
                                                         const CWeaponMode&, EProjectileAttrib) const override {
    return EWeaponCollisionResponseTypes::EnemyNormal;
  }

  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) override;
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
};
} // namespace MP1
} // namespace urde
