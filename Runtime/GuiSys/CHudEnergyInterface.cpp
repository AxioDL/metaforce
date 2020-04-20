#include "Runtime/GuiSys/CHudEnergyInterface.hpp"

#include <array>

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Audio/CSfxManager.hpp"
#include "Runtime/GuiSys/CAuiEnergyBarT01.hpp"
#include "Runtime/GuiSys/CAuiMeter.hpp"
#include "Runtime/GuiSys/CGuiFrame.hpp"
#include "Runtime/GuiSys/CGuiTextPane.hpp"
#include "Runtime/GuiSys/CStringTable.hpp"

namespace urde {

constexpr std::array<CAuiEnergyBarT01::FCoordFunc, 5> CoordFuncs{
    CHudEnergyInterface::CombatEnergyCoordFunc, CHudEnergyInterface::CombatEnergyCoordFunc,
    CHudEnergyInterface::XRayEnergyCoordFunc,   CHudEnergyInterface::ThermalEnergyCoordFunc,
    CHudEnergyInterface::BallEnergyCoordFunc,
};

constexpr std::array Tesselations{
    0.2f, 0.2f, 0.1f, 0.2f, 1.f,
};

CHudEnergyInterface::CHudEnergyInterface(CGuiFrame& selHud, float tankEnergy, int totalEnergyTanks, int numTanksFilled,
                                         bool energyLow, EHudType hudType)
: x0_hudType(hudType)
, xc_tankEnergy(tankEnergy)
, x10_totalEnergyTanks(totalEnergyTanks)
, x14_numTanksFilled(numTanksFilled)
, x1c_27_energyLow(energyLow) {
  x20_textpane_energydigits = static_cast<CGuiTextPane*>(selHud.FindWidget("textpane_energydigits"));
  x24_meter_energytanks = static_cast<CAuiMeter*>(selHud.FindWidget("meter_energytanks"));
  x28_textpane_energywarning = static_cast<CGuiTextPane*>(selHud.FindWidget("textpane_energywarning"));
  x2c_energybart01_energybar = static_cast<CAuiEnergyBarT01*>(selHud.FindWidget("energybart01_energybar"));

  x2c_energybart01_energybar->SetCoordFunc(CoordFuncs[size_t(hudType)]);
  x2c_energybart01_energybar->SetTesselation(Tesselations[size_t(hudType)]);

  ITweakGuiColors::VisorEnergyBarColors barColors = g_tweakGuiColors->GetVisorEnergyBarColors(int(hudType));
  ITweakGuiColors::VisorEnergyInitColors initColors = g_tweakGuiColors->GetVisorEnergyInitColors(int(hudType));

  x20_textpane_energydigits->TextSupport().SetFontColor(initColors.digitsFont);
  x20_textpane_energydigits->TextSupport().SetOutlineColor(initColors.digitsOutline);

  x2c_energybart01_energybar->SetMaxEnergy(CPlayerState::GetBaseHealthCapacity());
  x2c_energybart01_energybar->SetFilledColor(barColors.filled);
  x2c_energybart01_energybar->SetShadowColor(barColors.shadow);
  x2c_energybart01_energybar->SetEmptyColor(barColors.empty);
  x2c_energybart01_energybar->SetFilledDrainSpeed(g_tweakGui->GetEnergyBarFilledSpeed());
  x2c_energybart01_energybar->SetShadowDrainSpeed(g_tweakGui->GetEnergyBarShadowSpeed());
  x2c_energybart01_energybar->SetShadowDrainDelay(g_tweakGui->GetEnergyBarDrainDelay());
  x2c_energybart01_energybar->SetIsAlwaysResetTimer(g_tweakGui->GetEnergyBarAlwaysResetDelay());

  x24_meter_energytanks->SetMaxCapacity(14);

  if (x28_textpane_energywarning) {
    x28_textpane_energywarning->TextSupport().SetFontColor(g_tweakGuiColors->GetEnergyWarningFont());
    x28_textpane_energywarning->TextSupport().SetOutlineColor(g_tweakGuiColors->GetEnergyWarningOutline());
    if (x1c_27_energyLow)
      x28_textpane_energywarning->TextSupport().SetText(g_MainStringTable->GetString(9));
    else
      x28_textpane_energywarning->TextSupport().SetText(u"");
  }

  for (int i = 0; i < 14; ++i) {
    CGuiGroup* g = static_cast<CGuiGroup*>(x24_meter_energytanks->GetWorkerWidget(i));
    if (CGuiWidget* w = g->GetWorkerWidget(0))
      w->SetColor(initColors.tankFilled);
    if (CGuiWidget* w = g->GetWorkerWidget(1))
      w->SetColor(initColors.tankEmpty);
  }
}

void CHudEnergyInterface::Update(float dt, float energyLowPulse) {
  if (x28_textpane_energywarning) {
    if (x1c_27_energyLow) {
      x4_energyLowFader = std::min(x4_energyLowFader + 2.f * dt, 1.f);
      zeus::CColor color = zeus::skWhite;
      color.a() = x4_energyLowFader * energyLowPulse;
      x28_textpane_energywarning->SetColor(color);
    } else {
      x4_energyLowFader = std::max(0.f, x4_energyLowFader - 2.f * dt);
      zeus::CColor color = zeus::skWhite;
      color.a() = x4_energyLowFader * energyLowPulse;
      x28_textpane_energywarning->SetColor(color);
    }

    if (x28_textpane_energywarning->GetGeometryColor().a())
      x28_textpane_energywarning->SetIsVisible(true);
    else
      x28_textpane_energywarning->SetIsVisible(false);
  }

  if (x2c_energybart01_energybar->GetFilledEnergy() != x18_cachedBarEnergy || x1c_26_barDirty) {
    x1c_26_barDirty = false;
    x18_cachedBarEnergy = x2c_energybart01_energybar->GetFilledEnergy();
    std::string string =
        fmt::format(FMT_STRING("{:02d}"), int(std::fmod(x18_cachedBarEnergy, CPlayerState::GetEnergyTankCapacity())));
    x20_textpane_energydigits->TextSupport().SetText(string);
  }

  ITweakGuiColors::VisorEnergyBarColors barColors = g_tweakGuiColors->GetVisorEnergyBarColors(int(x0_hudType));
  zeus::CColor emptyColor = x1c_27_energyLow ? g_tweakGuiColors->GetEnergyBarEmptyLowEnergy() : barColors.empty;
  zeus::CColor filledColor = x1c_27_energyLow ? g_tweakGuiColors->GetEnergyBarFilledLowEnergy() : barColors.filled;
  zeus::CColor shadowColor = x1c_27_energyLow ? g_tweakGuiColors->GetEnergyBarShadowLowEnergy() : barColors.shadow;
  zeus::CColor useFillColor = zeus::CColor::lerp(filledColor, g_tweakGuiColors->GetEnergyBarFlashColor(), x8_flashMag);
  if (x1c_27_energyLow)
    useFillColor = zeus::CColor::lerp(useFillColor, zeus::CColor(1.f, 0.8f, 0.4f, 1.f), energyLowPulse);
  x2c_energybart01_energybar->SetFilledColor(useFillColor);
  x2c_energybart01_energybar->SetShadowColor(shadowColor);
  x2c_energybart01_energybar->SetEmptyColor(emptyColor);
}

void CHudEnergyInterface::SetEnergyLow(bool energyLow) {
  if (x1c_27_energyLow == energyLow)
    return;

  std::u16string string;
  if (energyLow)
    string = g_MainStringTable->GetString(9);

  if (x28_textpane_energywarning)
    x28_textpane_energywarning->TextSupport().SetText(string);

  if (energyLow)
    CSfxManager::SfxStart(SFXui_energy_low, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);

  x1c_27_energyLow = energyLow;
}

void CHudEnergyInterface::SetFlashMagnitude(float mag) { x8_flashMag = zeus::clamp(0.f, mag, 1.f); }

void CHudEnergyInterface::SetNumFilledEnergyTanks(int numTanksFilled) {
  x14_numTanksFilled = numTanksFilled;
  x24_meter_energytanks->SetCurrValue(numTanksFilled);
}

void CHudEnergyInterface::SetNumTotalEnergyTanks(int totalEnergyTanks) {
  x10_totalEnergyTanks = totalEnergyTanks;
  x24_meter_energytanks->SetCapacity(totalEnergyTanks);
}

void CHudEnergyInterface::SetCurrEnergy(float tankEnergy, bool wrapped) {
  xc_tankEnergy = tankEnergy;
  x2c_energybart01_energybar->SetCurrEnergy(tankEnergy, tankEnergy == 0.f ? CAuiEnergyBarT01::ESetMode::Insta
                                                                          : CAuiEnergyBarT01::ESetMode(wrapped));
}

std::pair<zeus::CVector3f, zeus::CVector3f> CHudEnergyInterface::CombatEnergyCoordFunc(float t) {
  float theta = 0.46764705f * t - 0.15882353f;
  float x = 17.f * std::sin(theta);
  float y = 17.f * std::cos(theta) - 17.f;
  return {zeus::CVector3f(x, y, 0.4f), zeus::CVector3f(x, y, 0.f)};
}

std::pair<zeus::CVector3f, zeus::CVector3f> CHudEnergyInterface::XRayEnergyCoordFunc(float t) {
  float theta = 1.8207964f - 0.69f * t;
  float x = std::cos(theta);
  float z = std::sin(theta);
  return {zeus::CVector3f(9.4f * x, 0.f, 9.4f * z), zeus::CVector3f(9.f * x, 0.f, 9.f * z)};
}

std::pair<zeus::CVector3f, zeus::CVector3f> CHudEnergyInterface::ThermalEnergyCoordFunc(float t) {
  float x = 8.1663399f * t;
  return {zeus::CVector3f(x, 0.f, 0.f), zeus::CVector3f(x, 0.f, 0.4355512f)};
}

std::pair<zeus::CVector3f, zeus::CVector3f> CHudEnergyInterface::BallEnergyCoordFunc(float t) {
  float x = 1.6666f * t;
  return {zeus::CVector3f(x, 0.f, 0.f), zeus::CVector3f(x, 0.f, 0.088887997f)};
}

} // namespace urde
