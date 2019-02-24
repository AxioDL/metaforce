#pragma once

#include "World/CWallWalker.hpp"
#include "World/CPathFindSearch.hpp"
#include "Weapon/CProjectileInfo.hpp"

namespace urde::MP1 {
class CSeedling : public CWallWalker {
  static const std::string skNeedleLocators[2][6];
  CPathFindSearch x5d8_searchPath;
  std::unique_ptr<CModelData> x6bc_spikeData;
  CProjectileInfo x6c0_projectileInfo;
  CDamageInfo x6e8_deathDamage;
  zeus::CAABox x704_modelBounds = zeus::skNullBox;
  float x71c_attackCoolOff = 0.f;
  TUniqueId x720_prevObj = kInvalidUniqueId;
  bool x722_24_renderOnlyClusterA : 1;
  bool x722_25_curNeedleCluster : 1;
  void LaunchNeedles(CStateManager&);

public:
  DEFINE_PATTERNED(Seedling)
  CSeedling(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
            const CPatternedInfo&, const CActorParameters&, CAssetId, CAssetId, const CDamageInfo&, const CDamageInfo&,
            float, float, float, float);

  void Accept(IVisitor&);
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
  void Think(float, CStateManager&);
  void Render(const CStateManager&) const;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt);
  CProjectileInfo* GetProjectileInfo() { return &x6c0_projectileInfo; }
  rstl::optional<zeus::CAABox> GetTouchBounds() const;
  void Touch(CActor&, CStateManager&);
  CPathFindSearch* GetSearchPath() { return &x5d8_searchPath; }
  void Patrol(CStateManager&, EStateMsg, float);
  void Active(CStateManager&, EStateMsg, float);
  void Enraged(CStateManager&, EStateMsg, float);
  void ProjectileAttack(CStateManager&, EStateMsg, float);
  void Generate(CStateManager&, EStateMsg, float);
  bool ShouldAttack(CStateManager&, float);
  void MassiveDeath(CStateManager&);
};
} // namespace urde::MP1
