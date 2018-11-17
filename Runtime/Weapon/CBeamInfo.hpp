#pragma once

#include "RetroTypes.hpp"
#include "IOStreams.hpp"
#include "zeus/CColor.hpp"

namespace urde
{
struct CBeamInfo
{
    u32 x0_;
    s32 x4_;
    CAssetId x8_;
    CAssetId xc_;
    CAssetId x10_;
    CAssetId x14_;
    s32 x18_;
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
        : x0_(in.readUint32Big()), x4_(in.readUint32Big()), x8_(in.readUint32Big()), xc_(in.readUint32Big()),
          x10_(in.readUint32Big()), x14_(in.readUint32Big()), x18_(in.readFloatBig()), x1c_(in.readFloatBig()),
          x20_(in.readFloatBig()), x24_(in.readFloatBig()), x28_(in.readFloatBig()), x2c_(in.readFloatBig()),
          x30_(in.readFloatBig()), x34_(in.readFloatBig()), x38_(in.readFloatBig()),
          x3c_(zeus::CColor::ReadRGBABig(in)), x40_(zeus::CColor::ReadRGBABig(in))
    {
    }

    CBeamInfo(s32 w1, CAssetId w2, CAssetId w3, CAssetId w4, CAssetId w5, s32 w6, float f1, float f2, float f3,
              float f4, float f5, float f6, float f7, const zeus::CColor& col1, const zeus::CColor& col2, float f8)
        : x4_(w1), x8_(w2), xc_(w3), x10_(w4), x14_(w5), x18_(w6), x1c_(f1), x20_(f2), x24_(f3), x28_(f4), x2c_(f5),
          x30_(f6), x34_(f7), x38_(f8), x3c_(col1), x40_(col2)
    {

    }
};

}
