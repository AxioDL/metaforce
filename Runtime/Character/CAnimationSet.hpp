#pragma once

#include "IOStreams.hpp"
#include "CAnimation.hpp"
#include "CTransition.hpp"
#include "CHalfTransition.hpp"
#include "CAdditiveAnimPlayback.hpp"

namespace urde
{

class CAnimationSet
{
    u16 x0_tableCount;
    std::vector<CAnimation> x4_animations;
    std::vector<CTransition> x14_transitions;
    std::shared_ptr<IMetaTrans> x24_defaultTransition;
    std::vector<std::pair<u32, CAdditiveAnimationInfo>> x28_additiveInfo;
    CAdditiveAnimationInfo x38_defaultAdditiveInfo;
    std::vector<CHalfTransition> x40_halfTransitions;
    std::vector<std::pair<CAssetId, CAssetId>> x50_animRes;

public:
    CAnimationSet(CInputStream& in);

    const std::vector<CAnimation>& GetAnimations() const { return x4_animations; }
    const std::vector<CTransition>& GetTransitions() const { return x14_transitions; }
    const std::shared_ptr<IMetaTrans>& GetDefaultTransition() const { return x24_defaultTransition; }
    const std::vector<CHalfTransition>& GetHalfTransitions() const { return x40_halfTransitions; }
    const std::vector<std::pair<u32, CAdditiveAnimationInfo>>& GetAdditiveInfo() const { return x28_additiveInfo; }
    const CAdditiveAnimationInfo& GetDefaultAdditiveInfo() const { return x38_defaultAdditiveInfo; }
    const std::vector<std::pair<CAssetId, CAssetId>>& GetAnimResIds() const { return x50_animRes; }
};
}

