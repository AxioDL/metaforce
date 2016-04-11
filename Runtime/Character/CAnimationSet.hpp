#ifndef __PSHAG_CANIMATIONSET_HPP__
#define __PSHAG_CANIMATIONSET_HPP__

#include "IOStreams.hpp"
#include "CAnimation.hpp"
#include "CTransition.hpp"
#include "CHalfTransition.hpp"

namespace urde
{

class CAdditiveAnimationInfo
{
    float x0_a = 0.f;
    float x4_b = 0.f;
public:
    void read(CInputStream& in)
    {
        x0_a = in.readFloatBig();
        x4_b = in.readFloatBig();
    }
    CAdditiveAnimationInfo() = default;
    CAdditiveAnimationInfo(CInputStream& in) {read(in);}
};

class CAnimationSet
{
    u16 x0_tableCount;
    std::vector<CAnimation> x4_animations;
    std::vector<CTransition> x14_transitions;
    std::shared_ptr<IMetaTrans> x24_defaultTransition;
    std::vector<std::pair<u32, CAdditiveAnimationInfo>> x28_additiveInfo;
    CAdditiveAnimationInfo x38_defaultAdditiveInfo;
    std::vector<CHalfTransition> x40_halfTransitions;
    std::vector<std::pair<TResId, TResId>> x50_animRes;

public:
    CAnimationSet(CInputStream& in);
};

}

#endif // __PSHAG_CANIMATIONSET_HPP__
