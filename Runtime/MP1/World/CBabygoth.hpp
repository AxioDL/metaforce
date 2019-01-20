#pragma once

#include <Runtime/Collision/CJointCollisionDescription.hpp>
#include "World/CPatterned.hpp"
#include "World/CPathFindSearch.hpp"
#include "Weapon/CProjectileInfo.hpp"
#include "Character/CBoneTracking.hpp"
namespace urde {
class CCollisionActorManager;
class CWeaponDescription;
} // namespace urde

namespace urde::MP1 {
struct CBabygothData {
  float x0_;
  float x4_;
  CAssetId x8_;
  CDamageInfo xc_;
  CDamageInfo x28_;
  CAssetId x44_;
  CAssetId x48_fireBreathRes;
  CDamageInfo x4c_fireBreathDamage;
  CDamageVulnerability x68_;
  CDamageVulnerability xd0_shellVulnerabilities;
  CAssetId x138_;
  CAssetId x13c_;
  float x140_shellHitPoints;
  s16 x144_shellCrackSfx;
  CAssetId x148_;
  CAssetId x14c_;
  CAssetId x150_;
  CAssetId x154_;
  s16 x158_;
  s16 x15a_;
  s16 x15c_;
  float x160_;
  float x164_;
  float x168_;
  CAssetId x16c_;
  s16 x170_;
  CAssetId x174_;

public:
  CBabygothData(CInputStream&);
  CAssetId GetShellModelResId() const;
  void GetFireballDamage() const;
  CAssetId GetFireballResID() const;
  void GetFireballAttackVariance() const;
  void GetFireballAttackTime() const;
  CAssetId GetFireBreathResId() const { return x48_fireBreathRes; }
  CDamageInfo GetFireBreathDamage() const { return x4c_fireBreathDamage; }
  const CDamageVulnerability& GetShellDamageVulnerability() const { return xd0_shellVulnerabilities; }
  float GetShellHitPoints() const { return x140_shellHitPoints; }
  s16 GetShellCrackSfx() { return x144_shellCrackSfx; }
};

class CBabygoth final : public CPatterned {
public:
  enum class EPathFindMode { Zero, One };

private:
  static constexpr s32 skSphereJointCount = 5;
  static const SSphereJointInfo skSphereJointList[skSphereJointCount];
  static const std::string_view skpMouthDamageJoint;
  u32 x568_ = -1;
  u32 x56c_ = 0;
  CBabygothData x570_babyData;
  TUniqueId x6e8_teamMgr = kInvalidUniqueId;
  CPathFindSearch x6ec_;
  CPathFindSearch x7d0_;
  EPathFindMode x8b4_pathFindMode;
  zeus::CVector3f x8b8_;
  zeus::CVector3f x8c4_;
  float x8d0_;
  float x8d4_stepBackwardDist = 0.f;
  float x8d8_ = 0.f;
  float x8dc_ = 0.f;
  float x8e0_ = 0.f;
  float x8e4_ = 0.f;
  float x8e8_ = 0.f;
  float x8ec_ = 0.f;
  CBoneTracking x8f0_boneTracking;
  std::unique_ptr<CCollisionActorManager> x928_colActMgr;
  CCollidableAABox x930_aabox;
  CProjectileInfo x958_;
  TUniqueId x980_flameThrower = kInvalidUniqueId;
  TToken<CWeaponDescription> x984_flameThrowerDesc;
  CDamageVulnerability x98c_;
  CSegId x9f4_mouthLocator;
  TUniqueId x9f6_ = kInvalidUniqueId;
  rstl::reserved_vector<TUniqueId, 1> x9f8_shellIds;
  float xa00_shellHitPoints;
  u32 xa04_ = 0;
  TLockedToken<CSkinnedModel> xa08_noShellModel;
  TToken<CGenDescription> xa14_;
  TToken<CGenDescription> xa20_;
  TToken<CGenDescription> xa2c_;
  TLockedToken<CGenDescription> xa38_; // Used to be an optional, not necessary in URDE
  union {
    struct {
      bool xa48_24_isAlert : 1;
      bool xa48_25_ : 1;
      bool xa48_26_ : 1;
      bool xa48_27_ : 1;
      bool xa48_28_ : 1;
      bool xa48_29_ : 1;
      bool xa48_30_ : 1;
      bool xa48_31_ : 1;
      bool xa49_24_ : 1;
      bool xa49_25_ : 1;
      bool xa49_26_ : 1;
      bool xa49_27_ : 1;
      bool xa49_28_ : 1;
      bool xa49_29_ : 1;
    };
    u32 _dummy = 0;
  };
  void AddSphereCollisionList(const SSphereJointInfo*, s32, std::vector<CJointCollisionDescription>&);
  void SetupCollisionManager(CStateManager&);
  void SetupHealthInfo(CStateManager&);
  void CreateFlameThrower(CStateManager&);
  void ApplyContactDamage(TUniqueId, CStateManager&);
  void RemoveFromTeam(CStateManager&);
  void ApplySeparationBehavior(CStateManager&);
  bool IsMouthCollisionActor(TUniqueId uid) { return x9f6_ == uid; }
  bool IsShell(TUniqueId uid) {
    for (TUniqueId shellId : x9f8_shellIds) {
      if (shellId == uid)
        return true;
    }
    return false;
  }
  void CrackShell(CStateManager&, const TLockedToken<CGenDescription>&, const zeus::CTransform&, s16, bool);
  void sub8021d478(CStateManager&, TUniqueId);
  void AvoidPlayerCollision(float, CStateManager&);
  s32 sub8023a180(TUniqueId, CStateManager&);
  void sub8021d6e8(CStateManager&);
  void sub8021e2c4(float);
  void sub8021e708(CStateManager&);
  void UpdateParticleEffects(float, CStateManager&);
  void TryToGetUp(CStateManager& mgr);
  bool CheckShouldWakeUp(CStateManager&, float);
  void SetProjectilePasshtrough(CStateManager&);
  void UpdateTouchBounds();
  void UpdateAttackPosition(CStateManager&, zeus::CVector3f&);
  void sub8021d644(CStateManager&);
  bool IsDestinationObstructed(CStateManager&);

public:
  DEFINE_PATTERNED(Babygoth)
  CBabygoth(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
            const CPatternedInfo&, const CActorParameters&, const CBabygothData&);
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr);
  void PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) {
    CPatterned::PreRender(mgr, frustum);
    xb4_drawFlags.x1_matSetIdx = u8(xa04_);
  }

  void Think(float, CStateManager&);
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt);
  float GetGravityConstant() const { return 10.f * 24.525f; }
  void SetPathFindMode(EPathFindMode mode) { x8b4_pathFindMode = mode; }
  const CCollisionPrimitive* GetCollisionPrimitive() const { return &x930_aabox; }
  EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f& v1, const zeus::CVector3f& v2,
                                                         const CWeaponMode& wMode, EProjectileAttrib attrib) const {
    if (wMode.GetType() == EWeaponType::Ice)
      return EWeaponCollisionResponseTypes::None;
    if (x56c_ != 3)
      return EWeaponCollisionResponseTypes::Unknown66;
    return CPatterned::GetCollisionResponseType(v1, v2, wMode, attrib);
  }

  void TakeDamage(const zeus::CVector3f&, float) {
    if (x400_25_alive)
      x428_damageCooldownTimer = 0.33f;
  }
  void Shock(CStateManager&, float, float);

  void TurnAround(CStateManager&, EStateMsg, float);
  void GetUp(CStateManager&, EStateMsg, float);
  void Enraged(CStateManager&, EStateMsg, float);
  void FollowPattern(CStateManager&, EStateMsg, float);
  void Taunt(CStateManager&, EStateMsg, float);
  void Crouch(CStateManager&, EStateMsg, float);
  void Deactivate(CStateManager&, EStateMsg, float);
  void Generate(CStateManager&, EStateMsg, float);
  void TargetPatrol(CStateManager&, EStateMsg, float);
  void Patrol(CStateManager&, EStateMsg, float);
  void Approach(CStateManager&, EStateMsg, float);
  void PathFind(CStateManager&, EStateMsg, float);
  void SpecialAttack(CStateManager&, EStateMsg, float);
  void Attack(CStateManager&, EStateMsg, float);
  void ProjectileAttack(CStateManager&, EStateMsg, float);

  bool AnimOver(CStateManager&, float) { return x568_ == 4; }

  bool SpotPlayer(CStateManager& mgr, float arg) {
    if (xa48_24_isAlert)
      return true;
    return CPatterned::SpotPlayer(mgr, arg);
  }
  bool InPosition(CStateManager&, float) { return (x8b8_ - GetTranslation()).magSquared() < 9.f; }

  bool ShotAt(CStateManager&, float) { return x400_24_hitByPlayerProjectile; }
};

} // namespace urde::MP1
