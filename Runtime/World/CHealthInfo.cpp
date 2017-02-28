#include "CHealthInfo.hpp"

namespace urde
{

CHealthInfo::CHealthInfo(CInputStream& in)
{
    in.readUint32Big();
    x0_health = in.readFloatBig();
    x4_knockbackResistance = in.readFloatBig();
}

}
