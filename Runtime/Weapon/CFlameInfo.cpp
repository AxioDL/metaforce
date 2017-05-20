#include "Weapon/CFlameInfo.hpp"

namespace urde
{

CFlameInfo::CFlameInfo(s32 w1, s32 w2, ResId flameFxId, s32 w3, float f1, float f2, float f3)
    : x0_(w1)
    , x4_(w2)
    , x8_flameFxId(flameFxId)
    , xc_(w3)
    , x10_(f1)
    , x18_(f2)
    , x1c_(f3)
{
}

}
