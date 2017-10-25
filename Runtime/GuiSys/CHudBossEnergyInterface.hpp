#ifndef __URDE_CHUDBOSSENERGYINTERFACE_HPP__
#define __URDE_CHUDBOSSENERGYINTERFACE_HPP__

#include "RetroTypes.hpp"
#include "zeus/CVector3f.hpp"

namespace urde
{
class CGuiFrame;
class CGuiWidget;
class CAuiEnergyBarT01;
class CGuiTextPane;

class CHudBossEnergyInterface
{
    float x0_alpha;
    float x4_fader = 0.f;
    float x8_curEnergy = 0.f;
    float xc_maxEnergy = 0.f;
    bool x10_24_visible : 1;
    CGuiWidget* x14_basewidget_bossenergystuff;
    CAuiEnergyBarT01* x18_energybart01_bossbar;
    CGuiTextPane* x1c_textpane_boss;
public:
    CHudBossEnergyInterface(CGuiFrame& selHud);
    void Update(float dt);
    void SetAlpha(float a) { x0_alpha = a; }
    void SetBossParams(bool visible, const std::u16string& name,
                       float curEnergy, float maxEnergy);
    static std::pair<zeus::CVector3f, zeus::CVector3f> BossEnergyCoordFunc(float t);
};

}

#endif // __URDE_CHUDBOSSENERGYINTERFACE_HPP__
