#ifndef __DNAMP1_CTWEAKGUICOLORS_HPP__
#define __DNAMP1_CTWEAKGUICOLORS_HPP__

#include "../../DNACommon/Tweaks/ITweakGuiColors.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct CTweakGuiColors : public ITweakGuiColors
{
    DECL_YAML
    DNAColor x4_;
    DNAColor x8_;
    DNAColor xc_;
    DNAColor x10_;
    DNAColor x14_hudMessageFill;
    DNAColor x18_hudMessageOutline;
    DNAColor x1c_hudFrameColor;
    DNAColor x20_;
    DNAColor x24_;
    DNAColor x28_;
    DNAColor x2c_;
    DNAColor x30_;
    DNAColor x34_energyBarFilledLowEnergy;
    DNAColor x38_energyBarShadowLowEnergy;
    DNAColor x3c_energyBarEmptyLowEnergy;
    DNAColor x40_;
    DNAColor x44_;
    DNAColor x48_;
    DNAColor x4c_;
    DNAColor x50_;
    DNAColor x54_;
    DNAColor x58_;
    DNAColor x5c_energyWarningFont;
    DNAColor x60_;
    DNAColor x64_;
    DNAColor x68_;
    DNAColor x6c_;
    DNAColor x70_;
    DNAColor x74_;
    DNAColor x78_;
    DNAColor x7c_;
    DNAColor x80_;
    DNAColor x84_;
    DNAColor x88_tickDecoColor;
    DNAColor x8c_;
    DNAColor x90_;
    DNAColor x94_;
    DNAColor x98_;
    DNAColor x9c_;
    DNAColor xa0_;
    DNAColor xa4_;
    DNAColor xa8_;
    DNAColor xac_;
    DNAColor xb0_;
    DNAColor xb4_energyWarningOutline;
    DNAColor xb8_;
    DNAColor xbc_;
    DNAColor xc0_;
    DNAColor xc4_;
    DNAColor xc8_;
    DNAColor xcc_;
    DNAColor xd0_;
    DNAColor xd4_;
    DNAColor xd8_;
    DNAColor xdc_;
    DNAColor xe0_;
    DNAColor xe4_;
    DNAColor xe8_energyBarFlashColor;
    DNAColor xec_;
    DNAColor xf0_;
    DNAColor xf4_;
    DNAColor xf8_;
    DNAColor xfc_;
    DNAColor x100_xrayEnergyDecoColor;
    DNAColor x104_;
    DNAColor x108_;
    DNAColor x10c_;
    DNAColor x110_;
    DNAColor x114_;
    DNAColor x118_;
    DNAColor x11c_;
    DNAColor x120_;
    DNAColor x124_;
    DNAColor x128_;
    DNAColor x12c_;
    DNAColor x130_;
    DNAColor x134_;
    DNAColor x138_;
    DNAColor x13c_;
    DNAColor x140_;
    DNAColor x144_;
    DNAColor x148_;
    DNAColor x14c_;
    DNAColor x150_;
    DNAColor x154_;
    DNAColor x158_;
    DNAColor x15c_;
    DNAColor x160_;
    DNAColor x164_;
    DNAColor x168_;
    DNAColor x16c_;
    DNAColor x170_;
    DNAColor x174_;
    DNAColor x178_;
    DNAColor x17c_;
    DNAColor x180_hudCounterFill;
    DNAColor x184_hudCounterOutline;
    DNAColor x188_;
    DNAColor x18c_;
    DNAColor x190_;
    DNAColor x194_;
    DNAColor x198_;
    DNAColor x19c_;
    DNAColor x1a0_;
    DNAColor x1a4_;
    DNAColor x1a8_;
    DNAColor x1ac_thermalDecoColor;
    DNAColor x1b0_thermalOutlinesColor;
    DNAColor x1b4_;
    DNAColor x1b8_thermalLockColor;
    DNAColor x1bc_;
    DNAColor x1c0_;
    struct PerVisorColors : BigYAML
    {
        DECL_YAML
        DNAColor x0_energyBarFilled;
        DNAColor x4_energyBarEmpty;
        DNAColor x8_energyBarShadow;
        DNAColor xc_energyTankFilled;
        DNAColor x10_energyTankEmpty;
        DNAColor x14_energyDigitsFont;
        DNAColor x18_energyDigitsOutline;
    };
    Value<atUint32> x1c4_perVisorCount;
    /* Combat, Scan, XRay, Thermal, Ball */
    Vector<PerVisorColors, DNA_COUNT(x1c4_perVisorCount)> x1c4_perVisorColors;

    CTweakGuiColors() = default;
    CTweakGuiColors(athena::io::IStreamReader& r) { this->read(r); }

    zeus::CColor GetHudMessageFill() const { return x14_hudMessageFill; }
    zeus::CColor GetHudMessageOutline() const { return x18_hudMessageOutline; }
    zeus::CColor GetHudFrameColor() const { return x1c_hudFrameColor; }
    zeus::CColor GetEnergyBarFilledLowEnergy() const { return x34_energyBarFilledLowEnergy; }
    zeus::CColor GetEnergyBarShadowLowEnergy() const { return x38_energyBarShadowLowEnergy; }
    zeus::CColor GetEnergyBarEmptyLowEnergy() const { return x3c_energyBarEmptyLowEnergy; }
    zeus::CColor GetEnergyWarningFont() const { return x5c_energyWarningFont; }
    zeus::CColor GetTickDecoColor() const { return x88_tickDecoColor; }
    zeus::CColor GetEnergyWarningOutline() const { return xb4_energyWarningOutline; }
    zeus::CColor GetEnergyBarFlashColor() const { return xe8_energyBarFlashColor; }
    zeus::CColor GetXRayEnergyDecoColor() const { return x100_xrayEnergyDecoColor; }
    zeus::CColor GetHudCounterFill() const { return x180_hudCounterFill; }
    zeus::CColor GetHudCounterOutline() const { return x184_hudCounterOutline; }
    zeus::CColor GetThermalDecoColor() const { return x1ac_thermalDecoColor; }
    zeus::CColor GetThermalOutlinesColor() const { return x1b0_thermalOutlinesColor; }
    zeus::CColor GetThermalLockColor() const { return x1b8_thermalLockColor; }
    VisorEnergyInitColors GetVisorEnergyInitColors(int idx) const
    {
        const PerVisorColors& colors = x1c4_perVisorColors[idx];
        return {colors.xc_energyTankFilled, colors.x10_energyTankEmpty,
                colors.x14_energyDigitsFont, colors.x18_energyDigitsOutline};
    }
    VisorEnergyBarColors GetVisorEnergyBarColors(int idx) const
    {
        const PerVisorColors& colors = x1c4_perVisorColors[idx];
        return {colors.x0_energyBarFilled, colors.x4_energyBarEmpty, colors.x8_energyBarShadow};
    }
};
}
}

#endif // __DNAMP1_CTWEAKGUICOLORS_HPP__
