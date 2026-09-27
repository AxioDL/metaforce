#ifndef _CBURROWER
#define _CBURROWER

#include "types.h"

#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

#include "Kyoto/TToken.hpp"

#include "rstl/optional_object.hpp"

class CElementGen;
class CGenDescription;

class CBurrower : public CPatterned {
public:
  CBurrower(const TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
            const CTransform4f& xf, const CModelData& mData, const CPatternedInfo& pInfo,
            const CActorParameters& actParms, const CAssetId& jumpParticle,
            const CAssetId& trailParticle, const CAssetId& projectile,
            const CDamageInfo& projectileDamage, const CAssetId& visorParticle, const uint visorSfx,
            const CAssetId deathExplosionParticle);

  // CEntity
  ~CBurrower() override;
  DECLARE_TYPES_MATCH;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CActor
  void AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  const CDamageVulnerability* GetDamageVulnerability(const CVector3f&, const CVector3f&,
                                                     const CDamageInfo&) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CAi
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;
  void Patrol(CStateManager& mgr, EStateMsg msg, float arg) override;
  void TargetPatrol(CStateManager& mgr, EStateMsg msg, float arg) override;
  void TurnAround(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Active(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Lurk(CStateManager& mgr, EStateMsg msg, float arg) override;
  void ProjectileAttack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Retreat(CStateManager& mgr, EStateMsg msg, float arg) override;
  bool ShouldAttack(CStateManager& mgr, float arg) override;
  bool PathShagged(CStateManager& mgr, float arg) override;

  // CPatterned
  CPathFindSearch* GetSearchPath() override { return &mPathFindSearch; }
  CProjectileInfo* ProjectileInfo() override;
  const rstl::optional_object< TCachedToken< CGenDescription > >&
  GetDeathExplosionParticle() const override;

private:
  CPathFindSearch mPathFindSearch;
  CProjectileInfo mProjectileInfo;
  rstl::single_ptr< CElementGen > mJumpParticle;
  rstl::single_ptr< CElementGen > mTrailParticle;
  rstl::optional_object< TLockedToken< CGenDescription > > mVisorParticle;
  rstl::optional_object< TCachedToken< CGenDescription > > mDeathExplosionParticle;
  float mAttackTime;
  float mLurkTimer;
  float mInvulnDamageTime;
  TUniqueId mLastDestObj;
  ushort mVisorSfx;
  bool mDoFacePlayer : 1;
  bool mInAir : 1;

  static const CDamageVulnerability skBombVulnerability;
};
CHECK_SIZEOF(CBurrower, (VERSION >= VERSION_GM8E_02 ? 0x6c0 : 0x6B0))

#endif // _CBURROWER
