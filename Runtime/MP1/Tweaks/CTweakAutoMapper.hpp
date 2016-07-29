#ifndef __URDE_MP1_CTWEAKAUTOMAPPER_HPP__
#define __URDE_MP1_CTWEAKAUTOMAPPER_HPP__

#include "RetroTypes.hpp"
#include "AutoMapper/ITweakAutoMapper.hpp"
#include "zeus/CColor.hpp"

namespace urde
{
namespace MP1
{
class CTweakAutoMapper : ITweakAutoMapper
{
private:
    union
    {
        struct
        {
            bool x4_24_ : 1;
            bool x4_25_ : 1;
            bool x4_26_ : 1;
        };
    };
    float x8_;
    float xc_;
    float x10_;
    float x14_;
    float x18_;
    float x1c_;
    float x20_;
    zeus::CColor x24_;
    float x28_;
    float x2c_;
    float x30_;
    float x34_;
    zeus::CColor x38_;
    zeus::CColor x3c_;
    zeus::CColor x40_;
    zeus::CColor x44_;
    zeus::CColor x48_;
    zeus::CColor x4c_;
    zeus::CColor x50_;
    float x54_;
    float x58_;
    float x5c_;
    float x60_ = 0.4f;
    float x64_;
    float x68_;
    float x6c_;
    float x70_;
    float x74_;
    float x78_;
    zeus::CColor x7c_;
    zeus::CColor x80_;
    float x84_;
    float x88_;
    float x8c_;
    float x90_;
    float x94_;
    float x98_;
    float x9c_;
    float xa0_;
    /* Originally 4 separate floats */
    zeus::CVector3f xa4_doorCenter;
    float xb0_;
    float xb4_;
    float xb8_;
    float xbc_;
    float xc0_;
    float xc4_;
    float xc8_;
    bool xcc_;
    float xd0_;
    float xd4_;
    float xd8_;
    float xdc_;
    float xe0_;
    float xe4_;
    float xe8_;
    float xec_;
    zeus::CColor xf0_;
    zeus::CColor xf4_;
    zeus::CColor xf8_;
    zeus::CColor xfc_;
    u32 x100_doorColorCount;
    std::vector<zeus::CColor> x104_doorColors;
    zeus::CColor x118_doorBorderColor;
    zeus::CColor x11c_openDoorColor;
public:
    CTweakAutoMapper(CInputStream&);
    const zeus::CVector3f& GetDoorCenter() const { return xa4_doorCenter; }
};
}
}
#endif // __URDE_MP1_CTWEAKAUTOMAPPER_HPP__
