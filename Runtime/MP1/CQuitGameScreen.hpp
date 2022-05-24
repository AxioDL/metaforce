#pragma once

#include <optional>

#include "Runtime/CToken.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/GuiSys/CGuiFrame.hpp"

namespace metaforce {
struct CFinalInput;
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

  void SetColors();

public:
  void FinishedLoading();
  void OnWidgetMouseUp(CGuiWidget* widget, bool cancel);
  void DoSelectionChange(CGuiTableGroup* caller, int oldSel);
  void DoAdvance(CGuiTableGroup* caller);
  EQuitAction Update(float dt);
  void Draw();
  void ProcessUserInput(const CFinalInput& input);
  explicit CQuitGameScreen(EQuitType type);
};

} // namespace MP1
} // namespace metaforce
