#include "Runtime/GuiSys/CHudBallInterface.hpp"

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/GuiSys/CGuiCamera.hpp"
#include "Runtime/GuiSys/CGuiFrame.hpp"
#include "Runtime/GuiSys/CGuiGroup.hpp"
#include "Runtime/GuiSys/CGuiModel.hpp"
#include "Runtime/GuiSys/CGuiTextPane.hpp"

namespace urde {

CHudBallInterface::CHudBallInterface(CGuiFrame& selHud, int pbAmount, int pbCapacity, int availableBombs, bool hasBombs,
                                     bool hasPb)
: x40_pbAmount(pbAmount), x44_pbCapacity(pbCapacity), x48_availableBombs(availableBombs), x4c_hasPb(hasPb) {
  x0_camera = selHud.GetFrameCamera();
  x4_basewidget_bombstuff = selHud.FindWidget("basewidget_bombstuff");
  x8_basewidget_bombdeco = selHud.FindWidget("basewidget_bombdeco");
  xc_model_bombicon = static_cast<CGuiModel*>(selHud.FindWidget("model_bombicon"));
  x10_textpane_bombdigits = static_cast<CGuiTextPane*>(selHud.FindWidget("textpane_bombdigits"));
  for (int i = 0; i < 3; ++i) {
    CGuiGroup* grp = static_cast<CGuiGroup*>(selHud.FindWidget(fmt::format(fmt("group_bombcount{}"), i)));
    CGuiWidget* filled = grp->GetWorkerWidget(1);
    CGuiWidget* empty = grp->GetWorkerWidget(0);
    x14_group_bombfilled.push_back(filled);
    x24_group_bombempty.push_back(empty);
    if (filled)
      filled->SetColor(g_tweakGuiColors->GetBallBombFilledColor());
    if (empty)
      empty->SetColor(g_tweakGuiColors->GetBallBombEmptyColor());
  }
  x8_basewidget_bombdeco->SetColor(g_tweakGuiColors->GetBallBombDecoColor());
  x34_camPos = x0_camera->GetLocalPosition();
  if (CGuiWidget* w = selHud.FindWidget("basewidget_energydeco"))
    w->SetColor(g_tweakGuiColors->GetBallBombEnergyColor());
  SetBombParams(pbAmount, pbCapacity, availableBombs, hasBombs, hasPb, true);
}

void CHudBallInterface::UpdatePowerBombReadoutColors() {
  zeus::CColor fontColor;
  zeus::CColor outlineColor;
  if (x40_pbAmount > 0) {
    fontColor = g_tweakGuiColors->GetPowerBombDigitAvailableFont();
    outlineColor = g_tweakGuiColors->GetPowerBombDigitAvailableOutline();
  } else if (x44_pbCapacity > 0) {
    fontColor = g_tweakGuiColors->GetPowerBombDigitDelpetedFont();
    outlineColor = g_tweakGuiColors->GetPowerBombDigitDelpetedOutline();
  } else {
    fontColor = zeus::skClear;
    outlineColor = zeus::skClear;
  }
  x10_textpane_bombdigits->TextSupport().SetFontColor(fontColor);
  x10_textpane_bombdigits->TextSupport().SetOutlineColor(outlineColor);

  zeus::CColor iconColor;
  if (x40_pbAmount > 0 && x4c_hasPb)
    iconColor = g_tweakGuiColors->GetPowerBombIconAvailableColor();
  else if (x44_pbCapacity > 0)
    iconColor = g_tweakGuiColors->GetPowerBombIconDepletedColor();
  else
    iconColor = zeus::skClear;

  xc_model_bombicon->SetColor(iconColor);
}

void CHudBallInterface::SetBombParams(int pbAmount, int pbCapacity, int availableBombs, bool hasBombs, bool hasPb,
                                      bool init) {
  if (pbAmount != x40_pbAmount || init) {
    x10_textpane_bombdigits->TextSupport().SetText(fmt::format(fmt("{:02d}"), pbAmount));
    x40_pbAmount = pbAmount;
    UpdatePowerBombReadoutColors();
  }

  if (x44_pbCapacity != pbCapacity || init) {
    x44_pbCapacity = pbCapacity;
    UpdatePowerBombReadoutColors();
  }

  if (x4c_hasPb != hasPb) {
    x4c_hasPb = hasPb;
    UpdatePowerBombReadoutColors();
  }

  for (int i = 0; i < 3; ++i) {
    bool lit = i < availableBombs;
    x14_group_bombfilled[i]->SetVisibility(lit && hasBombs, ETraversalMode::Children);
    x24_group_bombempty[i]->SetVisibility(!lit && hasBombs, ETraversalMode::Children);
  }

  x48_availableBombs = availableBombs;

  x8_basewidget_bombdeco->SetVisibility(hasBombs && x44_pbCapacity > 0, ETraversalMode::Children);
}

void CHudBallInterface::SetBallModeFactor(float t) {
  float tmp = 0.5f * 448.f * g_tweakGui->GetBallViewportYReduction();
  x0_camera->SetLocalTransform(
      zeus::CTransform::Translate(x34_camPos + zeus::CVector3f(0.f, 0.f, (t * tmp - tmp) * 0.01f)));
}

} // namespace urde
