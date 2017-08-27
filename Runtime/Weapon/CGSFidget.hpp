#ifndef __URDE_CGSFIDGET_HPP__
#define __URDE_CGSFIDGET_HPP__

#include "RetroTypes.hpp"
#include "CToken.hpp"

namespace urde
{
class CAnimData;
class CStateManager;
class CGSFidget
{
    std::vector<CToken> x0_anims;
    s32 x10_ = -1;
    s32 x14_gunId = -1;
    s32 x18_parm2 = -1;
public:
    bool Update(CAnimData& data, float dt, CStateManager& mgr);
    s32 SetAnim(CAnimData& data, s32 type, s32 gunId, s32 parm2, CStateManager& mgr);
    void LoadAnimAsync(CAnimData& data, s32 type, s32 gunId, s32 parm2, CStateManager& mgr);
    void UnLoadAnim();
    bool IsAnimLoaded() const;
};

}

#endif // __URDE_CGSFIDGET_HPP__
