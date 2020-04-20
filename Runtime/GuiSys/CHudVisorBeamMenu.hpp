#pragma once

#include <cfloat>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/rstl.hpp"

namespace urde {
class CGuiFrame;
class CGuiModel;
class CGuiTextPane;
class CGuiWidget;

class CHudVisorBeamMenu {
public:
  enum class EHudVisorBeamMenu { Visor, Beam };

private:
  struct SMenuItem {
    CGuiModel* x0_model_loz = nullptr;
    CGuiModel* x4_model_icon = nullptr;
    float x8_positioner = 0.f;
    float xc_opacity = 0.f;
  };

  enum class EAnimPhase { None, Steady, SelectFlash, Animate };

  CGuiFrame& x0_baseHud;
  EHudVisorBeamMenu x4_type;
  int x8_selectedItem = 0;
  int xc_pendingSelection = 0;
  float x10_interp = 1.f;
  bool x14_24_visibleDebug : 1 = true;
  bool x14_25_visibleGame : 1 = true;
  bool x14_26_dirty : 1 = true;
  CGuiWidget* x18_basewidget_menu;
  CGuiWidget* x1c_basewidget_menutitle;
  CGuiTextPane* x20_textpane_menu;
  CGuiModel* x24_model_ghost;
  rstl::reserved_vector<SMenuItem, 4> x28_menuItems;
  EAnimPhase x6c_animPhase = EAnimPhase::Steady;
  float x70_ = FLT_EPSILON;
  float x74_ = FLT_EPSILON;
  float x78_textFader = 0.f;
  float x7c_animDur;
  bool x80_24_swapBeamControls : 1;

  void UpdateMenuWidgetTransform(size_t idx, CGuiWidget& w, float t);

public:
  CHudVisorBeamMenu(CGuiFrame& baseHud, EHudVisorBeamMenu type, const rstl::reserved_vector<bool, 4>& enables);
  void Update(float dt, bool init);
  void UpdateHudAlpha(float alpha);
  void SetIsVisibleGame(bool v);
  void SetPlayerHas(const rstl::reserved_vector<bool, 4>& enables);
  void SetSelection(int selection, int pending, float interp);
};

} // namespace urde
