#pragma once

#include <string>

#include "Runtime/rstl.hpp"
#include "Runtime/Weapon/CProjectileInfo.hpp"
#include "Runtime/World/CPatterned.hpp"
#include "Runtime/World/CPathFindSearch.hpp"

namespace urde::MP1 {
class CAtomicAlpha : public CPatterned {
  static constexpr u32 skBombCount = 4;
  struct SBomb {
    std::string x0_locatorName;
    pas::ELocomotionType x10_locomotionType;
    float x14_scaleTime = FLT_MAX;
    SBomb(const std::string_view locator, pas::ELocomotionType locomotionType)
    : x0_locatorName(locator), x10_locomotionType(locomotionType) {}
  };
  bool x568_24_inRange : 1 = false;
  bool x568_25_invisible : 1;
  bool x568_26_applyBeamAttraction : 1;
  float x56c_bomdDropDelay;
  float x570_bombReappearDelay;
  float x574_bombRappearTime;
  float x578_bombTime = 0.f;
  u32 x57c_curBomb = 0;
  CPathFindSearch x580_pathFind;
  CSteeringBehaviors x664_steeringBehaviors;
  CProjectileInfo x668_bombProjectile;
  CModelData x690_bombModel;
  rstl::reserved_vector<SBomb, skBombCount> x6dc_bombLocators;

public:
  DEFINE_PATTERNED(AtomicAlpha)

  CAtomicAlpha(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
               const CActorParameters&, const CPatternedInfo&, CAssetId, const CDamageInfo&, float, float, float,
               CAssetId, bool, bool);

  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void Render(CStateManager&) override;
  void AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) override;
  void Think(float, CStateManager&) override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) override;

  CPathFindSearch* GetSearchPath() override { return &x580_pathFind; }

  EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&,
                                                         const CWeaponMode& wMode, EProjectileAttrib) const override {
    return GetDamageVulnerability()->WeaponHits(wMode, false) ? EWeaponCollisionResponseTypes::AtomicAlpha
                                                              : EWeaponCollisionResponseTypes::AtomicAlphaReflect;
  }

  bool Leash(CStateManager& mgr, float) override;
  bool AggressionCheck(CStateManager&, float) override;
  void CollidedWith(TUniqueId, const CCollisionInfoList&, CStateManager&) override;
  void Patrol(CStateManager&, EStateMsg, float) override;
  void Attack(CStateManager&, EStateMsg, float) override;

  CProjectileInfo* GetProjectileInfo() override { return &x668_bombProjectile; }
};
} // namespace urde::MP1
