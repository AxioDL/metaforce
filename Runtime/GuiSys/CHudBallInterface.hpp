#pragma once

#include "Runtime/rstl.hpp"
#include <zeus/CVector3f.hpp>

namespace urde {
class CGuiCamera;
class CGuiFrame;
class CGuiModel;
class CGuiTextPane;
class CGuiWidget;

class CHudBallInterface {
  CGuiCamera* x0_camera;
  CGuiWidget* x4_basewidget_bombstuff;
  CGuiWidget* x8_basewidget_bombdeco;
  CGuiModel* xc_model_bombicon;
  CGuiTextPane* x10_textpane_bombdigits;
  rstl::reserved_vector<CGuiWidget*, 3> x14_group_bombfilled;
  rstl::reserved_vector<CGuiWidget*, 3> x24_group_bombempty;
  zeus::CVector3f x34_camPos;
  int x40_pbAmount;
  int x44_pbCapacity;
  int x48_availableBombs;
  bool x4c_hasPb;
  void UpdatePowerBombReadoutColors();

public:
  CHudBallInterface(CGuiFrame& selHud, int pbAmount, int pbCapacity, int availableBombs, bool hasBombs, bool hasPb);
  void SetBombParams(int pbAmount, int pbCapacity, int availableBombs, bool hasBombs, bool hasPb, bool init);
  void SetBallModeFactor(float t);
};

} // namespace urde
