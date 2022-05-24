#include "Runtime/MP1/CQuitGameScreen.hpp"

#include <array>

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/Audio/CSfxManager.hpp"
#include "Runtime/Camera/CCameraFilter.hpp"
#include "Runtime/Input/CFinalInput.hpp"
#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/GuiSys/CGuiFrame.hpp"
#include "Runtime/GuiSys/CGuiTableGroup.hpp"
#include "Runtime/GuiSys/CGuiTextPane.hpp"
#include "Runtime/GuiSys/CGuiWidgetDrawParms.hpp"
#include "Runtime/GuiSys/CStringTable.hpp"

namespace metaforce::MP1 {

constexpr std::array Titles{24, 25, 26, 27, 28};

constexpr std::array DefaultSelections{1, 0, 1, 1, 0};

constexpr std::array VerticalOffsets{0.f, 1.6f, 1.f, 0.f, 1.f};

void CQuitGameScreen::SetColors() {
  x14_tablegroup_quitgame->SetColors(zeus::CColor{0.784313f, 0.784313f, 0.784313f, 1.f},
                                     zeus::CColor{0.196078f, 0.196078f, 0.196078f, 1.f});
}

void CQuitGameScreen::FinishedLoading() {
  x10_loadedFrame = x4_frame.GetObj();
  x10_loadedFrame->SetMaxAspect(1.33f);

  x14_tablegroup_quitgame = static_cast<CGuiTableGroup*>(x10_loadedFrame->FindWidget("tablegroup_quitgame"));
  x14_tablegroup_quitgame->SetVertical(false);
  x14_tablegroup_quitgame->SetMenuAdvanceCallback([this](CGuiTableGroup* caller) { DoAdvance(caller); });
  x14_tablegroup_quitgame->SetMenuSelectionChangeCallback(
      [this](CGuiTableGroup* caller, int oldSel) { DoSelectionChange(caller, oldSel); });

  static_cast<CGuiTextPane*>(x10_loadedFrame->FindWidget("textpane_title"))
      ->TextSupport()
      .SetText(g_MainStringTable->GetString(Titles[size_t(x0_type)]));

  static_cast<CGuiTextPane*>(x10_loadedFrame->FindWidget("textpane_yes"))
      ->TextSupport()
      .SetText(g_MainStringTable->GetString(22));
  static_cast<CGuiTextPane*>(x10_loadedFrame->FindWidget("textpane_no"))
      ->TextSupport()
      .SetText(g_MainStringTable->GetString(23));

  x14_tablegroup_quitgame->SetUserSelection(DefaultSelections[size_t(x0_type)]);
  x14_tablegroup_quitgame->SetWorkersMouseActive(true);
  x10_loadedFrame->SetMouseUpCallback([this](CGuiWidget* widget, bool cancel) { OnWidgetMouseUp(widget, cancel); });
  SetColors();
}

void CQuitGameScreen::OnWidgetMouseUp(CGuiWidget* widget, bool cancel) {
  if (!widget || cancel)
    return;
  DoAdvance(static_cast<CGuiTableGroup*>(widget->GetParent()));
}

void CQuitGameScreen::DoSelectionChange(CGuiTableGroup* caller, int oldSel) {
  SetColors();
  CSfxManager::SfxStart(SFXui_quit_change, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
}

void CQuitGameScreen::DoAdvance(CGuiTableGroup* caller) {
  if (caller->GetUserSelection() == 0) {
    /* Yes */
    CSfxManager::SfxStart(SFXui_advance, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
    x18_action = EQuitAction::Yes;
  } else {
    /* No */
    CSfxManager::SfxStart(SFXui_table_change_mode, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
    x18_action = EQuitAction::No;
  }
}

EQuitAction CQuitGameScreen::Update(float dt) {
  if (!x10_loadedFrame && x4_frame.IsLoaded())
    FinishedLoading();
  return x18_action;
}

void CQuitGameScreen::Draw() {
  SCOPED_GRAPHICS_DEBUG_GROUP("CQuitGameScreen::Draw", zeus::skPurple);
  if (x0_type == EQuitType::QuitGame) {
    CCameraFilterPass::DrawFilter(EFilterType::Blend, EFilterShape::Fullscreen, zeus::CColor{0.f, 0.5f}, nullptr, 1.f);
  }

  if (x10_loadedFrame) {
    x10_loadedFrame->Draw(CGuiWidgetDrawParms{1.f, zeus::CVector3f{0.f, 0.f, VerticalOffsets[size_t(x0_type)]}});
  }
}

void CQuitGameScreen::ProcessUserInput(const CFinalInput& input) {
  if (input.ControllerIdx() != 0) {
    return;
  }

  if (!x10_loadedFrame) {
    return;
  }

  x10_loadedFrame->ProcessMouseInput(
      input, CGuiWidgetDrawParms{1.f, zeus::CVector3f{0.f, 0.f, VerticalOffsets[size_t(x0_type)]}});
  x10_loadedFrame->ProcessUserInput(input);
  if ((input.PB() || input.PSpecialKey(aurora::SpecialKey::Esc)) && x0_type != EQuitType::ContinueFromLastSave) {
    x18_action = EQuitAction::No;
  }
}

CQuitGameScreen::CQuitGameScreen(EQuitType tp) : x0_type(tp) { x4_frame = g_SimplePool->GetObj("FRME_QuitScreen"); }

} // namespace metaforce::MP1
