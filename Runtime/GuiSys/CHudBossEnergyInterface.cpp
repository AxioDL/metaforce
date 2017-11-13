#include "CHudBossEnergyInterface.hpp"
#include "CGuiFrame.hpp"
#include "CAuiEnergyBarT01.hpp"
#include "GameGlobalObjects.hpp"
#include "CGuiTextPane.hpp"

namespace urde
{

CHudBossEnergyInterface::CHudBossEnergyInterface(CGuiFrame& selHud)
{
    x10_24_visible = false;
    x14_basewidget_bossenergystuff = selHud.FindWidget("basewidget_bossenergystuff");
    x18_energybart01_bossbar = static_cast<CAuiEnergyBarT01*>(selHud.FindWidget("energybart01_bossbar"));
    x1c_textpane_boss = static_cast<CGuiTextPane*>(selHud.FindWidget("textpane_boss"));

    x18_energybart01_bossbar->SetCoordFunc(BossEnergyCoordFunc);
    x18_energybart01_bossbar->SetTesselation(0.2f);
    ITweakGuiColors::VisorEnergyBarColors barColors = g_tweakGuiColors->GetVisorEnergyBarColors(0);
    x18_energybart01_bossbar->SetFilledColor(barColors.filled);
    x18_energybart01_bossbar->SetShadowColor(barColors.shadow);
    x18_energybart01_bossbar->SetEmptyColor(barColors.empty);
}

void CHudBossEnergyInterface::Update(float dt)
{
    if (x10_24_visible)
        x4_fader = std::min(x4_fader + dt, 1.f);
    else
        x4_fader = std::max(0.f, x4_fader - dt);

    if (x4_fader > 0.f)
    {
        zeus::CColor color = zeus::CColor::skWhite;
        color.a = x0_alpha * x4_fader;
        x14_basewidget_bossenergystuff->SetColor(color);
        x14_basewidget_bossenergystuff->SetVisibility(true, ETraversalMode::Children);
    }
    else
    {
        x14_basewidget_bossenergystuff->SetVisibility(false, ETraversalMode::Children);
    }
}

void CHudBossEnergyInterface::SetBossParams(bool visible, std::u16string_view name,
                                            float curEnergy, float maxEnergy)
{
    x10_24_visible = visible;
    if (visible)
    {
        x18_energybart01_bossbar->SetFilledDrainSpeed(maxEnergy);
        x18_energybart01_bossbar->SetCurrEnergy(curEnergy, CAuiEnergyBarT01::ESetMode::Normal);
        x18_energybart01_bossbar->SetMaxEnergy(maxEnergy);
        x1c_textpane_boss->TextSupport().SetText(name);
    }
    x8_curEnergy = curEnergy;
    xc_maxEnergy = maxEnergy;
}

std::pair<zeus::CVector3f, zeus::CVector3f> CHudBossEnergyInterface::BossEnergyCoordFunc(float t)
{
    float x = 9.25f * t - 4.625f;
    return {zeus::CVector3f(x, 0.f, 0.f), zeus::CVector3f(x, 0.f, 0.4f)};
}

}
