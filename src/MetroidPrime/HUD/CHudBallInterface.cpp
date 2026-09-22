#include "MetroidPrime/HUD/CHudBallInterface.hpp"

#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"

#include "GuiSys/CGuiCamera.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiGroup.hpp"
#include "GuiSys/CGuiModel.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "Kyoto/Basics/CBasics.hpp"

#include "rstl/string.hpp"
#include <stdio.h>

static const char skBombGroupName[] = "basewidget_bombstuff";
static const char skBombDigitsName[] = "textpane_bombdigits";
static const char skBombCounterBaseName[] = "group_bombcount";
static const char skBombTextName[] = "model_bomb_text";
static const char skPowerBombIconName[] = "model_bombicon";
static const char skEnergyDecoName[] = "basewidget_energydeco";
static const char skBombDecoName[] = "basewidget_bombdeco";

CHudBallInterface::CHudBallInterface(CGuiFrame& selHud, int pbAmount, int pbCapacity,
                                     int availableBombs, bool hasBombs, bool hasPb)
: x34_camPos(CVector3f::Zero())
, x40_pbAmount(pbAmount)
, x44_pbCapacity(pbCapacity)
, x48_availableBombs(availableBombs)
, x4c_hasPb(hasPb) {
  x0_camera = selHud.GetFrameCamera();

  x4_basewidget_bombstuff = selHud.FindWidget(skBombGroupName);
  x10_textpane_bombdigits = static_cast< CGuiTextPane* >(selHud.FindWidget(skBombDigitsName));
  xc_model_bombicon = static_cast< CGuiModel* >(selHud.FindWidget(skPowerBombIconName));
  x8_basewidget_bombdeco = selHud.FindWidget(skBombDecoName);

  for (int i = 0; i < 3; ++i) {
    rstl::string groupName(CBasics::Stringize("%s%d", skBombCounterBaseName, i));
    CGuiGroup* grp = static_cast< CGuiGroup* >(selHud.FindWidget(groupName));
    CGuiWidget* filled = grp->GetWorkerWidget(1);
    CGuiWidget* empty = grp->GetWorkerWidget(0);
    x14_group_bombfilled.push_back(filled);
    x24_group_bombempty.push_back(empty);
    if (filled)
      filled->SetColor(gpTweakGuiColors->GetBallBombFilledColor());
    if (empty)
      empty->SetColor(gpTweakGuiColors->GetBallBombEmptyColor());
  }

  x8_basewidget_bombdeco->SetColor(gpTweakGuiColors->GetBallBombDecoColor());
  x34_camPos = x0_camera->GetLocalPosition();

  if (CGuiWidget* w = selHud.FindWidget(rstl::string_l(skEnergyDecoName))) {
    w->SetColor(gpTweakGuiColors->GetBallBombEnergyColor());
  }
  SetBombParams(x40_pbAmount, pbCapacity, availableBombs, hasBombs, hasPb, true);
}

void CHudBallInterface::SetBombParams(int pbAmount, int pbCapacity, int availableBombs,
                                      bool hasBombs, bool hasPb, bool init) {

  if (pbAmount != x40_pbAmount || init) {
    char buffer[4];
#if NONMATCHING
    snprintf(buffer, sizeof(buffer), "%02d", pbAmount);
#else
    sprintf(buffer, "%02d", pbAmount);
#endif
    x10_textpane_bombdigits->TextSupport().SetText(rstl::string(buffer));
    x40_pbAmount = pbAmount;
    UpdatePowerBombReadoutColors();
  }

  if (pbCapacity != x44_pbCapacity || init) {
    x44_pbCapacity = pbCapacity;
    UpdatePowerBombReadoutColors();
  }

  if (hasPb != x4c_hasPb) {
    x4c_hasPb = hasPb;
    UpdatePowerBombReadoutColors();
  }

  for (int i = 0; i < 3; ++i) {
    bool lit = i < availableBombs;
    x14_group_bombfilled[i]->SetVisibility(lit && hasBombs, kTM_Children);
    x24_group_bombempty[i]->SetVisibility(!lit && hasBombs, kTM_Children);
  }

  x48_availableBombs = availableBombs;

  x8_basewidget_bombdeco->SetVisibility(hasBombs && x44_pbCapacity > 0, kTM_Children);
}

void CHudBallInterface::SetBallModeFactor(float t) {
  float tmp = gpTweakGui->GetBallViewportYReduction() * 448.0f * 0.5f;
  float zOffset = t * tmp - tmp;
  zOffset *= 0.01f;
  x0_camera->SetO2PTransform(CTransform4f::Translate(
      CVector3f(x34_camPos.GetX(), x34_camPos.GetY(), zOffset + x34_camPos.GetZ())));
}

void CHudBallInterface::UpdatePowerBombReadoutColors() {
  bool hasPbsCapacity = x44_pbCapacity > 0;
  bool hasPbsAmount = x40_pbAmount > 0;
  CColor clear(0);

  const CColor* fontColor = nullptr;
  if (hasPbsAmount) {
    fontColor = &gpTweakGuiColors->GetPowerBombDigitAvailableFont();
  } else if (hasPbsCapacity) {
    fontColor = &gpTweakGuiColors->GetPowerBombDigitDelpetedFont();
  } else {
    fontColor = &clear;
  }
  x10_textpane_bombdigits->TextSupport().SetFontColor(*fontColor);

  const CColor* outlineColor = nullptr;
  if (hasPbsAmount) {
    outlineColor = &gpTweakGuiColors->GetPowerBombDigitAvailableOutline();
  } else if (hasPbsCapacity) {
    outlineColor = &gpTweakGuiColors->GetPowerBombDigitDelpetedOutline();
  } else {
    outlineColor = &clear;
  }
  x10_textpane_bombdigits->TextSupport().SetOutlineColor(*outlineColor);

  const CColor* iconColor = nullptr;
  bool hasLastDisplay = false;
  if (hasPbsAmount && x4c_hasPb)
    hasLastDisplay = true;

  if (hasLastDisplay)
    iconColor = &gpTweakGuiColors->GetPowerBombIconAvailableColor();
  else if (hasPbsCapacity)
    iconColor = &gpTweakGuiColors->GetPowerBombIconDepletedColor();
  else
    iconColor = &clear;

  xc_model_bombicon->SetColor(*iconColor);
}
