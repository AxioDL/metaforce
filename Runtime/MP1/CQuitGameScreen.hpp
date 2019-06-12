#pragma once

#include "RetroTypes.hpp"
#include "CToken.hpp"
#include "Graphics/Shaders/CColoredQuadFilter.hpp"

namespace urde {
struct CFinalInput;
class CGuiFrame;
class CGuiTableGroup;
class CGuiTextPane;
class CGuiWidget;

namespace MP1 {

enum class EQuitType { QuitGame, ContinueFromLastSave, SaveProgress, QuitNESMetroid, ContinuePlaying };

enum class EQuitAction { None, Yes, No };

class CQuitGameScreen {
  EQuitType x0_type;
  TLockedToken<CGuiFrame> x4_frame;
  CGuiFrame* x10_loadedFrame = nullptr;
  CGuiTableGroup* x14_tablegroup_quitgame = nullptr;
  EQuitAction x18_action = EQuitAction::None;
  std::optional<CColoredQuadFilter> m_blackScreen;
  void SetColors();

public:
  void FinishedLoading();
  void OnWidgetMouseUp(CGuiWidget* widget, bool cancel);
  void DoSelectionChange(CGuiTableGroup* caller, int oldSel);
  void DoAdvance(CGuiTableGroup* caller);
  EQuitAction Update(float dt);
  void Draw();
  void ProcessUserInput(const CFinalInput& input);
  CQuitGameScreen(EQuitType type);
};

} // namespace MP1
} // namespace urde
