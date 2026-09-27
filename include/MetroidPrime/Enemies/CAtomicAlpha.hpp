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
  CProjectileInfo* ProjectileInfo() override { return &mBombProjectile; }
  CPathFindSearch* GetSearchPath() override { return &mPathFind; }

  bool CanDropBomb() const {
    return mBombTime >= mBombDropDelay &&
           mBombLocators[0].mScaleTime > (mBombReappearDelay + mBombRappearTime);
  }

private:
  enum { kBombCount = 4 };

  struct SBomb {
    rstl::string mLocatorName;
    pas::ELocomotionType mLocomotionType;
    float mScaleTime;

    SBomb(const rstl::string& name, pas::ELocomotionType loco, float scale)
    : mLocatorName(name), mLocomotionType(loco), mScaleTime(scale) {}
  };

  bool mInRange : 1;
  bool mInvisible : 1;
  bool mApplyBeamAttraction : 1;
  float mBombDropDelay;
  float mBombReappearDelay;
  float mBombRappearTime;
  float mBombTime;
  int mCurBomb;
  CPathFindSearch mPathFind;
  CSteeringBehaviors mSteeringBehaviors;
  CProjectileInfo mBombProjectile;
  CModelData mBombModel;
  rstl::reserved_vector< SBomb, kBombCount > mBombLocators;
};
CHECK_SIZEOF(CAtomicAlpha, (VERSION >= VERSION_GM8E_02 ? 0x750 : 0x740))

#endif // _CATOMICALPHA
