#ifndef __URDE_CANIMPERSEGMENTDATA_HPP__
#define __URDE_CANIMPERSEGMENTDATA_HPP__

#include "zeus/CQuaternion.hpp"
#include "zeus/CVector3f.hpp"

namespace urde
{

struct CAnimPerSegmentData
{
    zeus::CQuaternion x0_rotation;
    zeus::CVector3f x10_offset;
    bool x1c_hasOffset = false;
};

}

#endif // __URDE_CANIMPERSEGMENTDATA_HPP__
