#include "CGSFreeLook.hpp"
#include "CStateManager.hpp"
#include "Character/CAnimData.hpp"

namespace urde
{
s32 CGSFreeLook::GetSetId() const
{
    return -1;
}

bool CGSFreeLook::Update(CAnimData& animData, float dt, CStateManager& mgr)
{
    return false;
}

u32 CGSFreeLook::SetAnim(CAnimData&, s32, s32, s32, CStateManager&, float)
{
    return 0;
}
}
