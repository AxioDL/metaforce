#pragma once

#include <memory>
#include <utility>
#include <vector>

#include "Runtime/CSaveWorld.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/MP1/CPauseScreenBase.hpp"

namespace metaforce {
class CPlayerState;
class CScannableObjectInfo;
class CStringTable;
} // namespace metaforce

namespace metaforce::MP1 {
class CArtifactDoll;

class CLogBookScreen : public CPauseScreenBase {
  rstl::reserved_vector<std::vector<std::pair<CAssetId, bool>>, 5> x19c_scanCompletes;
  std::vector<std::pair<TCachedToken<CScannableObjectInfo>, TCachedToken<CStringTable>>> x1f0_curViewScans;
  rstl::reserved_vector<std::vector<std::pair<TLockedToken<CScannableObjectInfo>, TLockedToken<CStringTable>>>, 5>
      x200_viewScans;
  float x254_viewInterp = 0.f;
  std::unique_ptr<CArtifactDoll> x258_artifactDoll;
  enum class ELeavePauseState {
    InPause = 0,
    LeavingPause = 1,
    LeftPause = 2
  } x25c_leavePauseState = ELeavePauseState::InPause;
  bool x260_24_loaded : 1 = false;
  bool x260_25_inTextScroll : 1 = false;
  bool x260_26_exitTextScroll : 1 = false;

  void InitializeLogBook();
  void UpdateRightTitles();
  void PumpArticleLoad();
  bool IsScanCategoryReady(CSaveWorld::EScanCategory category) const;
  void UpdateBodyText();
  void UpdateBodyImagesAndText();
  int NextSurroundingArticleIndex(int cur) const;
  bool IsArtifactCategorySelected() const;
  int GetSelectedArtifactHeadScanIndex() const;
  static bool IsScanComplete(CSaveWorld::EScanCategory category, CAssetId scan, const CPlayerState& playerState);

public:
  CLogBookScreen(const CStateManager& mgr, CGuiFrame& frame, const CStringTable& pauseStrg);
  ~CLogBookScreen() override;

  bool InputDisabled() const override;
  void TransitioningAway() override;
  void Update(float dt, CRandom16& rand, CArchitectureQueue& archQueue) override;
  void Touch() override;
  void ProcessControllerInput(const CFinalInput& input) override;
  void Draw(float transInterp, float totalAlpha, float yOff) override;
  bool VReady() const override;
  void VActivate() override;
  void RightTableSelectionChanged(int oldSel, int newSel) override;
  void ChangedMode(EMode oldMode) override;
  void UpdateRightTable() override;
  bool ShouldLeftTableAdvance() const override;
  bool ShouldRightTableAdvance() const override;
  u32 GetRightTableCount() const override;
};

} // namespace metaforce::MP1
