#ifndef _CATOMICALPHA
#define _CATOMICALPHA

#include "types.h"

#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CSteeringBehaviors.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

#include "Kyoto/Animation/CharacterCommon.hpp"

#include "rstl/reserved_vector.hpp"
#include "rstl/string.hpp"

class CAtomicAlpha : public CPatterned {
public:
  CAtomicAlpha(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
               const CTransform4f& xf, const CModelData& mData, const CActorParameters& actParms,
               const CPatternedInfo& pInfo, CAssetId bombWeapon, const CDamageInfo& bombDamage,
               float bombDropDelay, float bombReappearDelay, float bombRappearTime, CAssetId cmdl,
               bool invisible, bool applyBeamAttraction);

  // CEntity
  ~CAtomicAlpha() override {}
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CActor
  void AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f&, const CVector3f&,
                                                         const CWeaponMode&, int) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CPhysicsActor
  void CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                    CStateManager& mgr) override;

  // CAi
  void Patrol(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Attack(CStateManager& mgr, EStateMsg msg, float arg) override;
  bool Leash(CStateManager& mgr, float arg) override;
  bool AggressionCheck(CStateManager& mgr, float arg) override;

  // CPatterned
  CProjectileInfo* ProjectileInfo() override { return &x668_bombProjectile; }
  CPathFindSearch* GetSearchPath() override { return &x580_pathFind; }

  bool CanDropBomb() const {
    return x578_bombTime >= x56c_bombDropDelay &&
           x6dc_bombLocators[0].x14_scaleTime > (x570_bombReappearDelay + x574_bombRappearTime);
  }

private:
  enum { kBombCount = 4 };

  struct SBomb {
    rstl::string x0_locatorName;
    pas::ELocomotionType x10_locomotionType;
    float x14_scaleTime;

    SBomb(const rstl::string& name, pas::ELocomotionType loco, float scale)
    : x0_locatorName(name), x10_locomotionType(loco), x14_scaleTime(scale) {}
  };

  bool x568_24_inRange : 1;
  bool x568_25_invisible : 1;
  bool x568_26_applyBeamAttraction : 1;
  float x56c_bombDropDelay;
  float x570_bombReappearDelay;
  float x574_bombRappearTime;
  float x578_bombTime;
  int x57c_curBomb;
  CPathFindSearch x580_pathFind;
  CSteeringBehaviors x664_steeringBehaviors;
  CProjectileInfo x668_bombProjectile;
  CModelData x690_bombModel;
  rstl::reserved_vector< SBomb, kBombCount > x6dc_bombLocators;
};
CHECK_SIZEOF(CAtomicAlpha, 0x740)

#endif // _CATOMICALPHA
