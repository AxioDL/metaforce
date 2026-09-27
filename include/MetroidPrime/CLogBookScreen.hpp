#ifndef _CLOGBOOKSCREEN
#define _CLOGBOOKSCREEN

#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CPauseScreenBase.hpp"
#include "MetroidPrime/CWorldSaveGameInfo.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CArtifactDoll;
class CScannableObjectInfo;
class CPlayerState;

class CLogBookScreen : public CPauseScreenBase {
public:
  ~CLogBookScreen() override;
  bool InputDisabled() const override;
  void TransitioningAway() override;
  void Update(float dt, CRandom16& rand, CArchitectureQueue& queue) override;
  void Touch() override;
  void ProcessInput(const CFinalInput& input) override;
  void Draw(float transInterp, float totalAlpha, float yOff) const override;
  bool VReady() const override;
  void VActivate() override;
  void RightTableSelectionChanged(int oldSel, int newSel) override;
  void ChangedMode(EMode oldMode) override;
  void UpdateRightTable() override;
  bool ShouldLeftTableAdvance() override;
  bool ShouldRightTableAdvance() override;
  uint GetRightTableCount() const override;

  CLogBookScreen(const CStateManager& mgr, CGuiFrame& frame, const CStringTable& pauseStrg);

private:
  enum ELeavePauseState { kLPS_InPause, kLPS_LeavingPause, kLPS_LeftPause };

  typedef rstl::pair< CAssetId, bool > SScanComplete;
  typedef rstl::vector< SScanComplete > CScanCategory;
  typedef rstl::pair< TCachedToken< CScannableObjectInfo >,
                      rstl::optional_object< TCachedToken< CStringTable > > >
      SArticle;
  typedef rstl::vector< SArticle > CArticleList;

  static bool IsScanComplete(CWorldSaveGameInfo::EScanCategory category, CAssetId scan,
                             const CPlayerState& playerState);
  bool IsScanCategoryReady(CWorldSaveGameInfo::EScanCategory category);
  void InitializeLogBook();
  void PumpArticleLoad();
  int NextSurroundingArticleIndex(int cur);
  void UpdateRightTitles();
  void UpdateBodyText();
  void UpdateBodyImagesAndText();
  bool IsArtifactCategorySelected() const;
  int GetSelectedArtifactHeadScanIndex() const;

  rstl::reserved_vector< CScanCategory, 5 > mScanCompletes;
  CArticleList mCurViewScans;
  rstl::reserved_vector< CArticleList, 5 > mViewScans;
  float mViewInterp;
  rstl::single_ptr< CArtifactDoll > mArtifactDoll;
  ELeavePauseState mLeavePauseState;
  bool mLoaded : 1;
  bool mInTextScroll : 1;
  bool mExitTextScroll : 1;
};
CHECK_SIZEOF(CLogBookScreen, 0x264)

#endif // _CLOGBOOKSCREEN
