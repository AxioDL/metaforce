#include "CCollisionInfo.hpp"

namespace urde
{

zeus::CVector3f CCollisionInfo::GetExtreme() const
{
    return (x18_ + (x0_aabox.min + x0_aabox.max) + x24_);
}

}
