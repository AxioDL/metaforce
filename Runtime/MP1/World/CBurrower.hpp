#pragma once

#include "Runtime/Weapon/CProjectileInfo.hpp"
#include "Runtime/World/CPatterned.hpp"
#include "Runtime/World/CPathFindSearch.hpp"

namespace metaforce {
class CElementGen;
namespace MP1 {

class CBurrower : public CPatterned {
  CPathFindSearch x568_pathFindSearch;
  CProjectileInfo x64c_projectileInfo;
  std::unique_ptr<CElementGen> x674_jumpParticle;
  std::unique_ptr<CElementGen> x678_trailParticle;
  std::optional<TLockedToken<CGenDescription>> x67c_visorParticle;
  std::optional<TLockedToken<CGenDescription>> x68c_deathExplosionParticle;
  float x69c_attackTime = 0.f;
  float x6a0_lurkTimer = 0.f;
  float x6a4_invulnDamageTime = 0.f;
  TUniqueId x6a8_lastDestObj = kInvalidUniqueId;
  s16 x6aa_visorSfx;
  bool x6ac_24_doFacePlayer : 1 = false;
  bool x6ac_25_inAir : 1 = false;
public:
  DEFINE_PATTERNED(Burrower);
  CBurrower(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
            const CPatternedInfo&, const CActorParameters&, CAssetId, CAssetId, CAssetId, const CDamageInfo&, CAssetId,
            u32, CAssetId);

  void Think(float, CStateManager&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void AddToRenderer(const zeus::CFrustum&, CStateManager&) override;
  void Render(CStateManager& mgr) override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  const CDamageVulnerability* GetDamageVulnerability(const zeus::CVector3f&, const zeus::CVector3f&,
                                                     const CDamageInfo&) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) override;
  void Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state) override;
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TurnAround(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Active(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Lurk(CStateManager& mgr, EStateMsg msg, float dt) override;
  void ProjectileAttack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Retreat(CStateManager& mgr, EStateMsg msg, float dt) override;
  bool PathShagged(CStateManager& mgr, float arg) override;
  bool ShouldAttack(CStateManager& mgr, float arg) override;
  CPathFindSearch* GetSearchPath() override { return &x568_pathFindSearch; }
  CProjectileInfo* GetProjectileInfo() override { return &x64c_projectileInfo; }
  const std::optional<TLockedToken<CGenDescription>>& GetDeathExplosionParticle() const override;
};
} // namespace MP1
} // namespace metaforce
