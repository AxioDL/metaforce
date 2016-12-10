#ifndef __URDE_CGSCOMBOFIRE_HPP__
#define __URDE_CGSCOMBOFIRE_HPP__

#include "RetroTypes.hpp"

namespace urde
{

class CAnimData;
class CStateManager;
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
public:
    bool IsComboOver() const;
    s32 GetLoopState() const;
    void SetLoopState(s32);
    void SetIdle(bool);
    void GetGunId() const;
    bool Update(CAnimData&, float, CStateManager&);
    void SetAnim(CAnimData&, s32, s32, s32, CStateManager&, float);
};

}

#endif // __URDE_CGSCOMBOFIRE_HPP__
