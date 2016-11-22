#ifndef __URDE_CGUNCONTROLLER_HPP__
#define __URDE_CGUNCONTROLLER_HPP__

#include "Character/CModelData.hpp"
#include "CGSFreeLook.hpp"
#include "CGSComboFire.hpp"
#include "CGSFidget.hpp"

namespace urde
{

class CGunController
{
    CModelData& x0_modelData;
    CGSFreeLook x4_freeLook;
    CGSComboFire x1c_comboFire;
    CGSFidget x30_fidget;
    u32 x50_ = 0;
    u32 x54_ = -1;

    union
    {
        struct
        {
            bool x58_24_ : 1;
            bool x58_25_ : 1;
        };
        u8 _dummy = 0;
    };
public:
    CGunController(CModelData& modelData);
};

}

#endif // __URDE_CGUNCONTROLLER_HPP__
