#ifndef __URDE_CWORLDSHADOW_HPP__
#define __URDE_CWORLDSHADOW_HPP__

#include "RetroTypes.hpp"
#include "zeus/CAABox.hpp"

namespace urde
{
class CStateManager;

class CWorldShadow
{
public:
    CWorldShadow(u32, u32, bool);
    void EnableModelProjectedShadow(const zeus::CTransform& pos, s32 lightIdx, float f1);
    void DisableModelProjectedShadow();
    void BuildLightShadowTexture(const CStateManager& mgr, TAreaId aid, s32 lightIdx,
                                 const zeus::CAABox& aabb, bool b1, bool b2);
    void ResetBlur();
};

}

#endif // __URDE_CWORLDSHADOW_HPP__
