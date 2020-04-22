#pragma once

#include <memory>

#include "Editor/ProjectResourceFactoryBase.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/MP1/CInGameGuiManager.hpp"
#include "Runtime/MP1/CPauseScreenBase.hpp"

namespace urde {
class CDependencyGroup;

namespace MP1 {

class CPauseScreen {
public:
  enum class ESubScreen { LogBook, Options, Inventory, ToGame, ToMap };

private:
  ESubScreen x0_initialSubScreen;
  u32 x4_ = 2;
  ESubScreen x8_curSubscreen = ESubScreen::ToGame;
  ESubScreen xc_nextSubscreen = ESubScreen::ToGame;
  float x10_alphaInterp = 0.f;
  TLockedToken<CStringTable> x14_strgPauseScreen;
  const CDependencyGroup& x20_suitDgrp;
  const CDependencyGroup& x24_ballDgrp;
  TLockedToken<CGuiFrame> x28_pauseScreenInstructions;
  CGuiFrame* x34_loadedPauseScreenInstructions = nullptr;
  CGuiTextPane* x38_textpane_l1 = nullptr;
  CGuiTextPane* x3c_textpane_r = nullptr;
  CGuiTextPane* x40_textpane_a = nullptr;
  CGuiTextPane* x44_textpane_b = nullptr;
  CGuiTextPane* x48_textpane_return = nullptr;
  CGuiTextPane* x4c_textpane_next = nullptr;
  CGuiTextPane* x50_textpane_back = nullptr;
  CAssetId x54_frmePauseScreenId;
  u32 x58_frmePauseScreenBufSz;
  std::unique_ptr<u8[]> x5c_frmePauseScreenBuf;
  std::shared_ptr<IDvdRequest> x60_loadTok;
  rstl::reserved_vector<std::unique_ptr<CGuiFrame>, 2> x64_frameInsts;
  u32 x78_activeIdx = 0;
  rstl::reserved_vector<std::unique_ptr<CPauseScreenBase>, 2> x7c_screens;
  bool x90_resourcesLoaded = false;
  bool x91_initialTransition = true;

  bool m_returnClicked : 1 = false;
  bool m_nextClicked : 1 = false;
  bool m_backClicked : 1 = false;
  bool m_lClicked : 1 = false;
  bool m_rClicked : 1 = false;
  bool m_returnDown : 1 = false;
  bool m_nextDown : 1 = false;
  bool m_backDown : 1 = false;
  bool m_lDown : 1 = false;
  bool m_rDown : 1 = false;

  std::unique_ptr<CPauseScreenBase> BuildPauseSubScreen(ESubScreen subscreen, const CStateManager& mgr,
                                                        CGuiFrame& frame) const;
  void StartTransition(float time, const CStateManager& mgr, ESubScreen subscreen, int);
  bool CheckLoadComplete(const CStateManager& mgr);
  void InitializeFrameGlue();
  bool InputEnabled() const;
  static ESubScreen GetPreviousSubscreen(ESubScreen screen);
  static ESubScreen GetNextSubscreen(ESubScreen screen);
  void TransitionComplete();

  void OnWidgetMouseDown(CGuiWidget* widget, bool resume);
  void OnWidgetMouseUp(CGuiWidget* widget, bool cancel);

public:
  CPauseScreen(ESubScreen subscreen, const CDependencyGroup& suitDgrp, const CDependencyGroup& ballDgrp);
  ~CPauseScreen();
  void ProcessControllerInput(const CStateManager& mgr, const CFinalInput& input);
  void Update(float dt, const CStateManager& mgr, CRandom16& rand, CArchitectureQueue& archQueue);
  void PreDraw();
  void Draw();
  bool IsLoaded() const { return x90_resourcesLoaded; }
  bool ShouldSwitchToMapScreen() const;
  bool ShouldSwitchToInGame() const;
  bool IsTransitioning() const { return x8_curSubscreen != xc_nextSubscreen; }
  float GetHelmetCamYOff() const;
};

} // namespace MP1
} // namespace urde
