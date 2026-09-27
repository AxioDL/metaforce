#ifndef _CPAUSESCREEN
#define _CPAUSESCREEN

#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CPauseScreenBase.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CDependencyGroup;
class CDvdRequest;

class CPauseScreen {
public:
  enum ESubScreen { kSS_LogBook, kSS_Options, kSS_Inventory, kSS_ToGame, kSS_ToMap };

  CPauseScreen(int subscreen, const CDependencyGroup& suitDgrp, const CDependencyGroup& ballDgrp);
  ~CPauseScreen();
  void ProcessControllerInput(const CStateManager& mgr, const CFinalInput& input);
  void Update(float dt, const CStateManager& mgr, CRandom16& rand, CArchitectureQueue& queue);
  void PreDraw();
  void Draw() const;
  bool IsLoaded() const;
  bool IsTransitioning() const { return mCurSubscreen != mNextSubscreen; }
  bool ShouldSwitchToMapScreen() const;
  bool ShouldSwitchToInGame() const;
  float GetHelmetCamYOff() const;

private:
  static bool IsInvalidSubscreen(ESubScreen screen) {
    return screen == kSS_ToGame || screen == kSS_ToMap;
  }
  static ESubScreen GetPreviousSubscreen(ESubScreen screen);
  static ESubScreen GetNextSubscreen(ESubScreen screen);
  CPauseScreenBase* BuildPauseSubScreen(ESubScreen screen, const CStateManager& mgr,
                                        CGuiFrame& frame) const;
  void StartTransition(float time, const CStateManager& mgr, ESubScreen screen, int direction);
  bool CheckLoadComplete(const CStateManager& mgr);
  void InitializeFrameGlue();
  bool InputEnabled() const;
  void TransitionComplete();

  ESubScreen mInitialSubscreen;
  int mDirection;
  ESubScreen mCurSubscreen;
  ESubScreen mNextSubscreen;
  float mAlphaInterp;
  TCachedToken< CStringTable > mStrgPauseScreen;
  const CDependencyGroup& mSuitDgrp;
  const CDependencyGroup& mBallDgrp;
  TCachedToken< CGuiFrame > mPauseScreenInstructions;
  CGuiFrame* mLoadedPauseScreenInstructions;
  CGuiTextPane* mTextpane_l1;
  CGuiTextPane* mTextpane_r;
  CGuiTextPane* mTextpane_a;
  CGuiTextPane* mTextpane_b;
  CGuiTextPane* mTextpane_return;
  CGuiTextPane* mTextpane_next;
  CGuiTextPane* mTextpane_back;
  CAssetId mFrmePauseScreenId;
  uint mFrmePauseScreenBufSz;
  rstl::single_ptr< char > mFrmePauseScreenBuf;
  rstl::single_ptr< CDvdRequest > mLoadTok;
  rstl::reserved_vector< rstl::auto_ptr< CGuiFrame >, 2 > mFrameInsts;
  uint mActiveIdx;
  rstl::reserved_vector< rstl::auto_ptr< CPauseScreenBase >, 2 > mScreens;
  bool mResourcesLoaded;
  bool mInitialTransition;
};
CHECK_SIZEOF(CPauseScreen, 0x94)

#endif // _CPAUSESCREEN
