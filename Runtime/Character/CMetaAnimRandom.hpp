#ifndef __URDE_CMETAANIMRANDOM_HPP__
#define __URDE_CMETAANIMRANDOM_HPP__

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

    std::shared_ptr<CAnimTreeNode> GetAnimationTree(const CAnimSysContext& animSys,
                                                    const CMetaAnimTreeBuildOrders& orders) const;
    void GetUniquePrimitives(std::set<CPrimitive>& primsOut) const;
    std::shared_ptr<CAnimTreeNode> VGetAnimationTree(const CAnimSysContext& animSys,
                                                     const CMetaAnimTreeBuildOrders& orders) const;
};

}

#endif // __URDE_CMETAANIMRANDOM_HPP__
