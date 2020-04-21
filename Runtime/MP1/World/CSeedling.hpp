#pragma once

#include <memory>
#include <string>

#include "Runtime/Weapon/CProjectileInfo.hpp"
#include "Runtime/World/CPathFindSearch.hpp"
#include "Runtime/World/CWallWalker.hpp"

#include <zeus/CAABox.hpp>

namespace urde::MP1 {
class CSeedling : public CWallWalker {
  CPathFindSearch x5d8_searchPath;
  std::unique_ptr<CModelData> x6bc_spikeData;
  CProjectileInfo x6c0_projectileInfo;
  CDamageInfo x6e8_deathDamage;
  zeus::CAABox x704_modelBounds = zeus::skNullBox;
  float x71c_attackCoolOff = 0.f;
  TUniqueId x720_prevObj = kInvalidUniqueId;
  bool x722_24_renderOnlyClusterA : 1 = true;
  bool x722_25_curNeedleCluster : 1 = false;
  void LaunchNeedles(CStateManager&);

public:
  DEFINE_PATTERNED(Seedling)
  CSeedling(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
            const CPatternedInfo&, const CActorParameters&, CAssetId, CAssetId, const CDamageInfo&, const CDamageInfo&,
            float, float, float, float);

  void Accept(IVisitor&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void Think(float, CStateManager&) override;
  void Render(CStateManager&) override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) override;
  CProjectileInfo* GetProjectileInfo() override { return &x6c0_projectileInfo; }
  std::optional<zeus::CAABox> GetTouchBounds() const override;
  void Touch(CActor&, CStateManager&) override;
  CPathFindSearch* GetSearchPath() override { return &x5d8_searchPath; }
  void Patrol(CStateManager&, EStateMsg, float) override;
  void Active(CStateManager&, EStateMsg, float) override;
  void Enraged(CStateManager&, EStateMsg, float) override;
  void ProjectileAttack(CStateManager&, EStateMsg, float) override;
  void Generate(CStateManager&, EStateMsg, float) override;
  bool ShouldAttack(CStateManager&, float) override;
  void MassiveDeath(CStateManager&) override;
};
} // namespace urde::MP1
