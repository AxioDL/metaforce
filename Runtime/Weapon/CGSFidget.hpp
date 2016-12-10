#ifndef __URDE_CGSFIDGET_HPP__
#define __URDE_CGSFIDGET_HPP__

#include "RetroTypes.hpp"

namespace urde
{
class CAnimData;
class CStateManager;
class CGSFidget
{
    u32 x4_ = 0;
    u32 x8_ = 0;
    u32 xc_ = 0;
    u32 x10_ = -1;
    u32 x14_ = -1;
    u32 x18_ = -1;
public:
    bool Update(CAnimData&, float, CStateManager&);
    void SetAnim(CAnimData&, s32, s32, s32, CStateManager&);
    void LoadAnimAsync(CAnimData, s32, s32, s32, CStateManager&);
    void UnLoadAnim();
    bool IsAnimLoaded() const;
};

}

#endif // __URDE_CGSFIDGET_HPP__
