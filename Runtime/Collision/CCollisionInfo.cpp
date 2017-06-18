#include "CCollisionInfo.hpp"

namespace urde
{

CCollisionInfo CCollisionInfo::GetSwapped() const
{
    CCollisionInfo ret;
    ret.x0_point = x0_point;
    ret.xc_ = xc_;
    ret.x30_valid = x30_valid;
    ret.x31_ = x31_;
    ret.x38_materialLeft = x40_materialRight;
    ret.x40_materialRight = x38_materialLeft;
    ret.x48_normalLeft = x54_normalRight;
    ret.x54_normalRight = x48_normalLeft;
    return ret;
}

void CCollisionInfo::Swap()
{
    x48_normalLeft = -x48_normalLeft;
    x54_normalRight = -x54_normalRight;
    std::swap(x38_materialLeft, x40_materialRight);
}

zeus::CVector3f CCollisionInfo::GetExtreme() const
{
    return x0_point + xc_ + x18_ + x24_;
}

}
