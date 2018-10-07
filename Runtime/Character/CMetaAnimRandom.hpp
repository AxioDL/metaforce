#pragma once

#include "IMetaAnim.hpp"
#include "IOStreams.hpp"

namespace urde
{

class CMetaAnimRandom : public IMetaAnim
{
    using RandomData = std::vector<std::pair<std::shared_ptr<IMetaAnim>, u32>>;
    RandomData x4_randomData;
    static RandomData CreateRandomData(CInputStream& in);
public:
    CMetaAnimRandom(CInputStream& in);
    EMetaAnimType GetType() const {return EMetaAnimType::Random;}

    void GetUniquePrimitives(std::set<CPrimitive>& primsOut) const;
    std::shared_ptr<CAnimTreeNode> VGetAnimationTree(const CAnimSysContext& animSys,
                                                     const CMetaAnimTreeBuildOrders& orders) const;
};

}

