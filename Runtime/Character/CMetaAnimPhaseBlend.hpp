#pragma once

#include "IMetaAnim.hpp"
#include "IOStreams.hpp"

namespace urde
{

class CMetaAnimPhaseBlend : public IMetaAnim
{
    std::shared_ptr<IMetaAnim> x4_animA;
    std::shared_ptr<IMetaAnim> x8_animB;
    float xc_blend;
    bool x10_;
public:
    CMetaAnimPhaseBlend(CInputStream& in);
    EMetaAnimType GetType() const {return EMetaAnimType::PhaseBlend;}

    void GetUniquePrimitives(std::set<CPrimitive>& primsOut) const;
    std::shared_ptr<CAnimTreeNode> VGetAnimationTree(const CAnimSysContext& animSys,
                                                     const CMetaAnimTreeBuildOrders& orders) const;
};

}

