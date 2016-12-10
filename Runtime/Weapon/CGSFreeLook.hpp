#ifndef __URDE_CGSFREELOOK_HPP__
#define __URDE_CGSFREELOOK_HPP__

#include "RetroTypes.hpp"

namespace urde
{

class CAnimData;
class CStateManager;
class CGSFreeLook
{
    friend class CGunController;
    float x0_ = 0.f;
    s32 x4_ = -1;
    u32 x8_ = -1;
    u32 xc_ = 0;
    u32 x10_ = -1;
    bool x14_ = false;
public:
    s32 GetSetId() const;
    void SetLoopState(s32);
    s32 GetLoopState() const;
    s32 GetGunId() const;
    bool Update(CAnimData&, float, CStateManager&);
    u32 SetAnim(CAnimData&, s32, s32, s32, CStateManager&, float);

};

}

#endif // __URDE_CGSFREELOOK_HPP__
