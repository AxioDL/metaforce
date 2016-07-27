#ifndef __URDE_MP1_CTWEAKPLAYER_HPP__
#define __URDE_MP1_CTWEAKPLAYER_HPP__

#include "RetroTypes.hpp"
#include "ITweak.hpp"
namespace urde
{
namespace MP1
{
class CTweakPlayer : public ITweak
{
    float x4_[8];
    float x24_[8];
    float x44_[8];
    float x64_[8];
    float x84_[8];
    float xa4_[8];
    float xc4_;
    float xc8_;
    float xcc_;
    float xd0_;
    float xd4_;
    float xd8_;
    float xdc_;
    float xe0_;
    float xe4_;
    float xe8_;
    float xec_;
    float xf0_;
    float xf4_;
    float xf8_;
    float xfc_;
    float x100_;
    float x104_;
    float x108_;
    float x10c_;
    float x110_;
    float x114_;
    float x118_;
    float x11c_;
    float x120_;
    float x124_;
    float x128_;
    float x12c_;
    float x130_;
    float x134_;
    float x138_;
    float x13c_;
    float x140_;
    float x144_;
    float x148_;
    float x14c_;
    float x150_;
    float x154_;
    float x158_[3];
    float x164_[3];
    float x170_[3];
    float x17c_;
    float x180_;
    float x184_;
    float x188_;
    float x18c_;
    float x190_;
    float x194_;
    float x198_;
    float x19c_;
    float x1a0_;
    float x1a4_;
    u32 x1a8_[2];
    u32 x1b0_[2];
    u32 x1b8_[2];
    u32 x1c0_[2];
    u32 x1c8_[2];
    u32 x1d0_[2];
    float x1d8_;
    float x1dc_;
    float x1e0_;
    float x1e4_;
    float x1e8_;
    float x1ec_;
    float x1f0_;
    float x1f4_;
    float x1f8_;
    float x1fc_;
    union
    {
        struct
        {
            bool x200_24_ : 1;
            bool x200_25_ : 1;
        };
        u8 _dummy1 = 0;
    };
    float x204_;
    float x208_;
    float x20c_;
    float x210_;
    float x214_;
    float x218_;
    union
    {
        struct
        {
            bool x21c_24_ : 1;
            bool x21c_25_ : 1;
        };
        u8 _dummy2 = 0;
    };
    float x220_;
    float x224_;
    union
    {
        struct
        {
            bool x228_24_ : 1;
            bool x228_25_ : 1;
            bool x228_26_ : 1;
            bool x228_27_ : 1;
            bool x228_28_ : 1;
            bool x228_29_ : 1;
            bool x228_30_ : 1;
            bool x228_31_ : 1;
            bool x229_24_ : 1;
            bool x229_25_ : 1;
            bool x229_26_ : 1;
            bool x229_27_ : 1;
            bool x229_28_ : 1;
            bool x229_29_ : 1;
            bool x229_30_ : 1;
            bool x229_31_ : 1;
            bool x22a_24_ : 1;
            bool x22a_25_ : 1;
            bool x22a_26_ : 1;
            bool x22a_27_ : 1;
            bool x22a_28_ : 1;
        };
        u32 _dummy3 = 0;
    };
    float x22c_;
    float x230_;
    float x234_;
    float x238_;
    float x23c_;
    float x240_;
    float x244_;
    float x248_;
    float x24c_;
    float x250_;
    float x254_;
    float x258_;
    float x25c_;
    float x260_;
    float x264_;
    float x268_;
    float x26c_;
    float x270_;
    float x274_;
    float x278_;
    float x27c_;
    float x280_;
    float x284_;
    float x288_;
    float x28c_;
    float x290_;
    float x294_;
    float x298_;
    float x29c_;
    float x2a0_;
    float x2a4_;
    float x2a8_;
    float x2ac_;
    float x2b0_;
    float x2b4_;
    float x2b8_;
    float x2bc_;
    float x2c0_;
    float x2c4_;
    float x2c8_;
    float x2cc_;
    u32 x2d0_;
    bool x2d4_;
    bool x2d5_;
    float x2d8_;
    float x2dc_;
    float x2e0_;
    float x2e4_;
    float x2e8_;
    float x2ec_;
    float x2f0_;
    bool x2f4_;
    float x2f8_;
    u32 x2fc_;
    float x300_;
    float x304_;
    float x308_;
public:
    CTweakPlayer(CInputStream&);
    virtual ~CTweakPlayer() {}
};
}
}
#endif // __URDE_MP1_CTWEAKPLAYER_HPP__
