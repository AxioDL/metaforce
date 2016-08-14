#ifndef __URDE_CGSCOMBOFIRE_HPP__
#define __URDE_CGSCOMBOFIRE_HPP__

#include "RetroTypes.hpp"

namespace urde
{

class CGSComboFire
{
    float x0_ = 0.f;
    u32 x4_ = -1;
    u32 x8_ = -1;
    u32 xc_ = -1;

    union
    {
        struct
        {
            bool x10_24_ : 1;
            bool x10_25_ : 1;
        };
        u8 _dummy = 0;
    };
};

}

#endif // __URDE_CGSCOMBOFIRE_HPP__
