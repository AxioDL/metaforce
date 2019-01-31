#pragma once

#include "World/CPatterned.hpp"
#include "World/CPathFindSearch.hpp"
#include "Weapon/CProjectileInfo.hpp"

namespace urde::MP1 {
class CAtomicAlpha : public CPatterned {
  static const std::string_view skBombLocators[4];
  static constexpr u32 skBombCount = 4;
  struct SBomb {
    std::string x0_locatorName;
    pas::ELocomotionType x10_locomotionType;
    float x14_scaleTime = FLT_MAX;
    SBomb(const std::string_view locator, pas::ELocomotionType locomotionType)
    : x0_locatorName(locator.data()), x10_locomotionType(locomotionType) {}
  };
  bool x568_24_inRange : 1;
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

  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
  void Render(const CStateManager&) const;
  void AddToRenderer(const zeus::CFrustum& frustum, const CStateManager& mgr) const;
  void Think(float, CStateManager&);
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt);

  CPathFindSearch* GetSearchPath() { return &x580_pathFind; }

  EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&,
                                                         const CWeaponMode& wMode, EProjectileAttrib) const {
    return GetDamageVulnerability()->WeaponHits(wMode, false) ? EWeaponCollisionResponseTypes::AtomicAlpha
                                                              : EWeaponCollisionResponseTypes::AtomicAlphaReflect;
  }

  bool Leash(CStateManager& mgr, float);
  bool AggressionCheck(CStateManager&, float);
  void CollidedWith(TUniqueId, const CCollisionInfoList&, CStateManager&);
  void Patrol(CStateManager&, EStateMsg, float);
  void Attack(CStateManager&, EStateMsg, float);

  CProjectileInfo* GetProjectileInfo() { return &x668_bombProjectile; }
};
} // namespace urde::MP1
