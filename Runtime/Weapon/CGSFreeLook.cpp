#include "CGSFreeLook.hpp"
#include "CStateManager.hpp"
#include "Character/CPASAnimParmData.hpp"

namespace urde
{

bool CGSFreeLook::Update(CAnimData& data, float dt, CStateManager& mgr)
{
    if (x4_cueAnimId != -1)
    {
        x0_delay -= dt;
        if (x0_delay <= 0.f)
        {
            data.EnableLooping(x8_loopState == 1);
            CAnimPlaybackParms aparms(x4_cueAnimId, -1, 1.f, true);
            data.SetAnimation(aparms, false);
            x0_delay = 0.f;
            x4_cueAnimId = -1;
        }
    }
    else if (!data.IsAnimTimeRemaining(0.001f, "Whole Body"))
    {
        switch (x8_loopState)
        {
        case 0:
            SetAnim(data, xc_gunId, x10_setId, 1, mgr, 0.f);
            break;
        case 2:
            x8_loopState = -1;
            return true;
        default:
            break;
        }
    }
    return false;
}

s32 CGSFreeLook::SetAnim(CAnimData& data, s32 gunId, s32 setId, s32 loopState, CStateManager& mgr, float delay)
{
    s32 useLoopState = 1;
    if (!x14_idle)
        useLoopState = loopState;
    x14_idle = false;
    const CPASDatabase& pas = data.GetCharacterInfo().GetPASDatabase();
    CPASAnimParmData parms(3, CPASAnimParm::FromInt32(gunId),
                           CPASAnimParm::FromInt32(setId), CPASAnimParm::FromEnum(useLoopState));
    auto anim = pas.FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
    xc_gunId = gunId;
    x10_setId = pas.GetAnimState(3)->GetAnimParmData(anim.second, 1).GetInt32Value();
    x8_loopState = useLoopState;
    if (delay != 0.f)
    {
        x0_delay = delay;
        x4_cueAnimId = anim.second;
    }
    else
    {
        data.EnableLooping(loopState == 1);
        CAnimPlaybackParms aparms(anim.second, -1, 1.f, true);
        data.SetAnimation(aparms, false);
    }
    return anim.second;
}

}
