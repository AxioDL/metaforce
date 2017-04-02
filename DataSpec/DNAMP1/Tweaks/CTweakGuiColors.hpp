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
    DNAColor x34_;
    DNAColor x38_;
    DNAColor x3c_;
    DNAColor x40_;
    DNAColor x44_;
    DNAColor x48_;
    DNAColor x4c_;
    DNAColor x50_;
    DNAColor x54_;
    DNAColor x58_;
    DNAColor x5c_;
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
    DNAColor xb4_;
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
    DNAColor xe8_;
    DNAColor xec_;
    DNAColor xf0_;
    DNAColor xf4_;
    DNAColor xf8_;
    DNAColor xfc_;
    DNAColor x100_;
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
    DNAColor x1ac_;
    DNAColor x1b0_;
    DNAColor x1b4_;
    DNAColor x1b8_;
    DNAColor x1bc_;
    DNAColor x1c0_;
    struct UnkColors : BigYAML
    {
        DECL_YAML
        DNAColor x0_;
        DNAColor x4_;
        DNAColor x8_;
        DNAColor xc_;
        DNAColor x10_;
        DNAColor x14_;
        DNAColor x18_;
    };
    Value<atUint32> x1c4_count;
    Vector<UnkColors, DNA_COUNT(x1c4_count)> x1c4_;

    CTweakGuiColors() = default;
    CTweakGuiColors(athena::io::IStreamReader& r) { this->read(r); }

    zeus::CColor GetHudMessageFill() const { return x14_hudMessageFill; }
    zeus::CColor GetHudMessageOutline() const { return x18_hudMessageOutline; }
    zeus::CColor GetHudFrameColor() const { return x1c_hudFrameColor; }
    zeus::CColor GetTickDecoColor() const { return x88_tickDecoColor; }
    zeus::CColor GetHudCounterFill() const { return x180_hudCounterFill; }
    zeus::CColor GetHudCounterOutline() const { return x184_hudCounterOutline; }
};
}
}

#endif // __DNAMP1_CTWEAKGUICOLORS_HPP__
