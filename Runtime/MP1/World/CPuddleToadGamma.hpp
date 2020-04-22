#pragma once

#include <memory>
#include <string_view>

#include "Runtime/Collision/CCollidableOBBTreeGroup.hpp"
#include "Runtime/World/CPatterned.hpp"

#include <zeus/CVector3f.hpp>

namespace urde::MP1 {

class CPuddleToadGamma final : public CPatterned {
  u32 x568_stateProg = 0;
  float x56c_waitTimer = 0.f;
  CDamageInfo x570_playerShootDamage;
  CDamageInfo x58c_;
  float x5a8_suckForceMultiplier;
  float x5ac_minSuckAngleProj;
  float x5b0_playerSuckRange;
  zeus::CVector3f x5b4_localShootDir;
  float x5c0_playerShootSpeed;
  float x5c4_shouldAttackWaitTime;
  float x5c8_spotPlayerWaitTime;
  zeus::CVector3f x5cc_suckPoint;
  zeus::CVector3f x5d8_damageablePoint;
  std::unique_ptr<CCollidableOBBTreeGroup> x5e4_collisionTreePrim;
  bool x5e8_24_playerInside : 1 = false;
  bool x5e8_25_waitTimerActive : 1 = false;
  bool x5e8_26_shotPlayer : 1 = false;

  void SetSolid(CStateManager&, bool);

  static void CenterPlayer(CStateManager&, const zeus::CVector3f&, float);
  void ShootPlayer(CStateManager&, float);
  void SuckPlayer(CStateManager&, float);
  bool PlayerInVortexArea(const CStateManager&) const;
  void SetPlayerPosition(CStateManager&, const zeus::CVector3f&);

public:
  DEFINE_PATTERNED(PuddleToad)

  CPuddleToadGamma(TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo& info,
                   const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo,
                   const CActorParameters& aParms, float suckForceMultiplier, float suckAngle, float playerSuckRange,
                   const zeus::CVector3f& localShootDir, float playerShootSpeed, float shouldAttackWaitTime,
                   float spotPlayerWaitTime, const CDamageInfo& playerShootDamage, const CDamageInfo& dInfo2,
                   CAssetId dcln);

  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void Think(float dt, CStateManager& mgr) override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) override;
  std::optional<zeus::CAABox> GetTouchBounds() const override;

  const CDamageVulnerability* GetDamageVulnerability(const zeus::CVector3f&, const zeus::CVector3f&,
                                                     const CDamageInfo&) const override;

  const CDamageVulnerability* GetDamageVulnerability() const override {
    return &CDamageVulnerability::ImmuneVulnerabilty();
  }

  const CCollisionPrimitive* GetCollisionPrimitive() const override;

  zeus::CTransform GetPrimitiveTransform() const override;

  void InActive(CStateManager&, EStateMsg, float) override;
  void Active(CStateManager&, EStateMsg, float) override;
  void Suck(CStateManager&, EStateMsg, float) override;
  void Attack(CStateManager&, EStateMsg, float) override;
  void Crouch(CStateManager&, EStateMsg, float) override;
  bool InAttackPosition(CStateManager&, float) override;
  bool SpotPlayer(CStateManager&, float) override;
  bool ShouldAttack(CStateManager&, float) override;
  bool LostInterest(CStateManager&, float) override;
  bool Inside(CStateManager&, float) override;
};

} // namespace urde::MP1
