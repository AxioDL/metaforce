#pragma once

#include "World/CPatterned.hpp"
#include "Collision/CCollidableOBBTreeGroup.hpp"

namespace urde::MP1 {

class CPuddleToadGamma final : public CPatterned {
  static constexpr std::string_view mMouthLocatorName = "MOUTH_LCTR_SDK"sv;
  static constexpr std::string_view mBellyLocatorName = "SAMUS_POS_LCTR_SDK"sv;
  static const zeus::CVector3f skBellyOffset;
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
  bool x5e8_24_playerInside : 1;
  bool x5e8_25_waitTimerActive : 1;
  bool x5e8_26_shotPlayer : 1;

  void SetSolid(CStateManager&, bool);

  static void CenterPlayer(CStateManager&, const zeus::CVector3f&, float);
  void ShootPlayer(CStateManager&, float);
  void SuckPlayer(CStateManager&, float);
  bool PlayerInVortexArea(const CStateManager&);
  void SetPlayerPosition(CStateManager&, const zeus::CVector3f&);

public:
  DEFINE_PATTERNED(PuddleToad)

  CPuddleToadGamma(TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo& info,
                   const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo,
                   const CActorParameters& aParms, float suckForceMultiplier, float suckAngle, float playerSuckRange,
                   const zeus::CVector3f& localShootDir, float playerShootSpeed, float shouldAttackWaitTime,
                   float spotPlayerWaitTime, const CDamageInfo& playerShootDamage, const CDamageInfo& dInfo2,
                   CAssetId dcln);

  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
  void Think(float dt, CStateManager& mgr);
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt);
  std::optional<zeus::CAABox> GetTouchBounds() const;

  const CDamageVulnerability* GetDamageVulnerability(const zeus::CVector3f&, const zeus::CVector3f&,
                                                     const CDamageInfo&) const;

  const CDamageVulnerability* GetDamageVulnerability() const { return &CDamageVulnerability::ImmuneVulnerabilty(); }

  const CCollisionPrimitive* GetCollisionPrimitive() const;

  zeus::CTransform GetPrimitiveTransform() const;

  void InActive(CStateManager&, EStateMsg, float);
  void Active(CStateManager&, EStateMsg, float);
  void Suck(CStateManager&, EStateMsg, float);
  void Attack(CStateManager&, EStateMsg, float);
  void Crouch(CStateManager&, EStateMsg, float);
  bool InAttackPosition(CStateManager&, float);
  bool SpotPlayer(CStateManager&, float);
  bool ShouldAttack(CStateManager&, float);
  bool LostInterest(CStateManager&, float);
  bool Inside(CStateManager&, float);
};

} // namespace urde::MP1
