#ifndef __URDE_CBEAMINFO_HPP__
#define __URDE_CBEAMINFO_HPP__

#include "RetroTypes.hpp"
#include "IOStreams.hpp"
#include "zeus/CColor.hpp"

namespace urde
{
struct CBeamInfo
{
    u32 x0_;
    u32 x4_;
    ResId x8_;
    ResId xc_;
    ResId x10_;
    ResId x14_;
    u32 x18_;
    float x1c_;
    float x20_;
    float x24_;
    float x28_;
    float x2c_;
    float x30_;
    float x34_;
    float x38_;
    zeus::CColor x3c_;
    zeus::CColor x40_;

    CBeamInfo(CInputStream& in)
    : x0_(in.readUint32Big())
    , x4_(in.readUint32Big())
    , x8_(in.readUint32Big())
    , xc_(in.readUint32Big())
    , x10_(in.readUint32Big())
    , x14_(in.readUint32Big())
    , x18_(in.readFloatBig())
    , x1c_(in.readFloatBig())
    , x20_(in.readFloatBig())
    , x24_(in.readFloatBig())
    , x28_(in.readFloatBig())
    , x2c_(in.readFloatBig())
    , x30_(in.readFloatBig())
    , x34_(in.readFloatBig())
    , x38_(in.readFloatBig())
    , x3c_(zeus::CColor::ReadRGBABig(in))
    , x40_(zeus::CColor::ReadRGBABig(in))
    {
    }
};

}
#endif // __URDE_CBEAMINFO_HPP__
