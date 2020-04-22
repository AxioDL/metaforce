#pragma once

#include <memory>
#include <vector>

#include "Runtime/CPlayerState.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/Character/CActorLights.hpp"

namespace urde {
class CModel;
namespace MP1 {

class CArtifactDoll {
  std::vector<TLockedToken<CModel>> x0_models;
  std::vector<CLight> x10_lights;
  std::unique_ptr<CActorLights> x20_actorLights;
  float x24_fader = 0.f;
  bool x28_24_loaded : 1 = false;
  void UpdateActorLights();

public:
  CArtifactDoll();
  static int GetArtifactHeadScanIndex(CAssetId scanId);
  static CAssetId GetArtifactHeadScanFromItemType(CPlayerState::EItemType item);
  static void UpdateArtifactHeadScan(const CStateManager& mgr, float delta);
  static void CompleteArtifactHeadScan(const CStateManager& mgr);
  void Draw(float alpha, const CStateManager& mgr, bool inArtifactCategory, int selectedArtifact);
  void Update(float dt, const CStateManager& mgr);
  void Touch();
  bool CheckLoadComplete();
  bool IsLoaded() const { return x28_24_loaded; }
};

} // namespace MP1
} // namespace urde
