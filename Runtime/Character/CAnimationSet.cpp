#include "CAnimationSet.hpp"
#include "CMetaTransFactory.hpp"

namespace urde
{

CAnimationSet::CAnimationSet(CInputStream& in)
: x0_tableCount(in.readUint16Big())
{
    u32 animationCount = in.readUint32Big();
    x4_animations.reserve(animationCount);
    for (u32 i=0 ; i<animationCount ; ++i)
        x4_animations.emplace_back(in);

    u32 transitionCount = in.readUint32Big();
    x14_transitions.reserve(transitionCount);
    for (u32 i=0 ; i<transitionCount ; ++i)
        x14_transitions.emplace_back(in);

    x24_defaultTransition = CMetaTransFactory::CreateMetaTrans(in);

    if (x0_tableCount > 1)
    {
        u32 additiveAnimCount = in.readUint32Big();
        x28_additiveInfo.reserve(additiveAnimCount);
        for (u32 i=0 ; i<additiveAnimCount ; ++i)
        {
            u32 id = in.readUint32Big();
            x28_additiveInfo.emplace_back(id, in);
        }
        x38_defaultAdditiveInfo.read(in);
    }

    if (x0_tableCount > 2)
    {
        u32 halfTransitionCount = in.readUint32Big();
        x40_halfTransitions.reserve(halfTransitionCount);
        for (u32 i=0 ; i<halfTransitionCount ; ++i)
            x40_halfTransitions.emplace_back(in);
    }

    if (x0_tableCount > 3)
    {
        u32 animResourcesCount = in.readUint32Big();
        x50_animRes.reserve(animResourcesCount);
        for (u32 i=0 ; i<animResourcesCount ; ++i)
        {
            TResId anim = in.readUint32Big();
            TResId evnt = in.readUint32Big();
            x50_animRes.emplace_back(anim, evnt);
        }
    }
}

}
