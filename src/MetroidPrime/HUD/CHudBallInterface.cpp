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
: mCamPos(CVector3f::Zero())
, mPbAmount(pbAmount)
, mPbCapacity(pbCapacity)
, mAvailableBombs(availableBombs)
, mHasPb(hasPb) {
  mCamera = selHud.GetFrameCamera();

  mBasewidget_bombstuff = selHud.FindWidget(skBombGroupName);
  mTextpane_bombdigits = static_cast< CGuiTextPane* >(selHud.FindWidget(skBombDigitsName));
  mModel_bombicon = static_cast< CGuiModel* >(selHud.FindWidget(skPowerBombIconName));
  mBasewidget_bombdeco = selHud.FindWidget(skBombDecoName);

  for (int i = 0; i < 3; ++i) {
    rstl::string groupName(CBasics::Stringize("%s%d", skBombCounterBaseName, i));
    CGuiGroup* grp = static_cast< CGuiGroup* >(selHud.FindWidget(groupName));
    CGuiWidget* filled = grp->GetWorkerWidget(1);
    CGuiWidget* empty = grp->GetWorkerWidget(0);
    mGroup_bombfilled.push_back(filled);
    mGroup_bombempty.push_back(empty);
    if (filled)
      filled->SetColor(gpTweakGuiColors->GetBallBombFilledColor());
    if (empty)
      empty->SetColor(gpTweakGuiColors->GetBallBombEmptyColor());
  }

  mBasewidget_bombdeco->SetColor(gpTweakGuiColors->GetBallBombDecoColor());
  mCamPos = mCamera->GetLocalPosition();

  if (CGuiWidget* w = selHud.FindWidget(rstl::string_l(skEnergyDecoName))) {
    w->SetColor(gpTweakGuiColors->GetBallBombEnergyColor());
  }
  SetBombParams(mPbAmount, pbCapacity, availableBombs, hasBombs, hasPb, true);
}

void CHudBallInterface::SetBombParams(int pbAmount, int pbCapacity, int availableBombs,
                                      bool hasBombs, bool hasPb, bool init) {

  if (pbAmount != mPbAmount || init) {
    char buffer[4];
#if NONMATCHING
    snprintf(buffer, sizeof(buffer), "%02d", pbAmount);
#else
    sprintf(buffer, "%02d", pbAmount);
#endif
    mTextpane_bombdigits->TextSupport().SetText(rstl::string(buffer));
    mPbAmount = pbAmount;
    UpdatePowerBombReadoutColors();
  }

  if (pbCapacity != mPbCapacity || init) {
    mPbCapacity = pbCapacity;
    UpdatePowerBombReadoutColors();
  }

  if (hasPb != mHasPb) {
    mHasPb = hasPb;
    UpdatePowerBombReadoutColors();
  }

  for (int i = 0; i < 3; ++i) {
    bool lit = i < availableBombs;
    mGroup_bombfilled[i]->SetVisibility(lit && hasBombs, kTM_Children);
    mGroup_bombempty[i]->SetVisibility(!lit && hasBombs, kTM_Children);
  }

  mAvailableBombs = availableBombs;

  mBasewidget_bombdeco->SetVisibility(hasBombs && mPbCapacity > 0, kTM_Children);
}

void CHudBallInterface::SetBallModeFactor(float t) {
  float tmp = gpTweakGui->GetBallViewportYReduction() * 448.0f * 0.5f;
  float zOffset = t * tmp - tmp;
  zOffset *= 0.01f;
  mCamera->SetO2PTransform(CTransform4f::Translate(
      CVector3f(mCamPos.GetX(), mCamPos.GetY(), zOffset + mCamPos.GetZ())));
}

void CHudBallInterface::UpdatePowerBombReadoutColors() {
  bool hasPbsCapacity = mPbCapacity > 0;
  bool hasPbsAmount = mPbAmount > 0;
  CColor clear(0);

  const CColor* fontColor = nullptr;
  if (hasPbsAmount) {
    fontColor = &gpTweakGuiColors->GetPowerBombDigitAvailableFont();
  } else if (hasPbsCapacity) {
    fontColor = &gpTweakGuiColors->GetPowerBombDigitDelpetedFont();
  } else {
    fontColor = &clear;
  }
  mTextpane_bombdigits->TextSupport().SetFontColor(*fontColor);

  const CColor* outlineColor = nullptr;
  if (hasPbsAmount) {
    outlineColor = &gpTweakGuiColors->GetPowerBombDigitAvailableOutline();
  } else if (hasPbsCapacity) {
    outlineColor = &gpTweakGuiColors->GetPowerBombDigitDelpetedOutline();
  } else {
    outlineColor = &clear;
  }
  mTextpane_bombdigits->TextSupport().SetOutlineColor(*outlineColor);

  const CColor* iconColor = nullptr;
  bool hasLastDisplay = false;
  if (hasPbsAmount && mHasPb)
    hasLastDisplay = true;

  if (hasLastDisplay)
    iconColor = &gpTweakGuiColors->GetPowerBombIconAvailableColor();
  else if (hasPbsCapacity)
    iconColor = &gpTweakGuiColors->GetPowerBombIconDepletedColor();
  else
    iconColor = &clear;

  mModel_bombicon->SetColor(*iconColor);
}
