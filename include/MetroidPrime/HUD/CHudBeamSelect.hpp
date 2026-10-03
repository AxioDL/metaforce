#ifndef _CHUDBEAMSELECT
#define _CHUDBEAMSELECT

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CGuiFrame;
class CGuiFrameLoader;
class CGuiWidget;
class CStringTable;
class CStateManager;

// Inferred class and method names from Trilogy's radial selection menus.
class CHudBeamSelect {
public:
  explicit CHudBeamSelect(const rstl::rc_ptr< TToken< CStringTable > >& stringTable);
  void Update(float dt, const CStateManager& mgr);
  void Draw() const;
  bool GetIsVisible() const;
  float GetAlpha() const;

private:
  void InitializeWidgets();
  void InitializeSelection(const CStateManager& mgr);
  bool HasSelectionChanged(const CStateManager& mgr) const;
  static void UpdateInterpolation(bool increase, float& value, float step);

  rstl::single_ptr< CGuiFrameLoader > mFrameLoader;
  rstl::single_ptr< CGuiFrame > mFrame;
  float mAlpha;
  float mSelectionFade;
  CGuiWidget* mBasewidgetSelect;
  CGuiWidget* mModelMainFrame;
  uint x18_;
  rstl::reserved_vector< CGuiWidget*, 4 > mHighlights;
  rstl::reserved_vector< CGuiWidget*, 4 > mIcons;
  rstl::reserved_vector< float, 4 > mHighlightAlpha;
  rstl::rc_ptr< TToken< CStringTable > > mStringTable;
  CSfxHandle mSelectionSfx;
  bool mSelectionChanged;
};
CHECK_SIZEOF(CHudBeamSelect, 0x68)

#endif // _CHUDBEAMSELECT
