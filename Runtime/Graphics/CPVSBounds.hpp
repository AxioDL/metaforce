#ifndef __URDE_CPVSBOUNDS_HPP__
#define __URDE_CPVSBOUNDS_HPP__

#include "RetroTypes.hpp"
#include "zeus/CVector3f.hpp"

namespace urde
{

class CPVSBounds
{
public:
    CPVSBounds(CInputStream& in);
    u32 GetBoundsFileSize();
    bool PointInBounds(const zeus::CVector3f&) const;
};

}

#endif // __URDE_CPVSBOUNDS_HPP__
