#include "CMetaAnimRandom.hpp"
#include "CMetaAnimFactory.hpp"

namespace urde
{

CMetaAnimRandom::RandomData CMetaAnimRandom::CreateRandomData(CInputStream& in)
{
    CMetaAnimRandom::RandomData ret;
    u32 randCount = in.readUint32Big();
    ret.reserve(randCount);

    for (u32 i=0 ; i<randCount ; ++i)
    {
        std::shared_ptr<IMetaAnim> metaAnim = CMetaAnimFactory::CreateMetaAnim(in);
        ret.emplace_back(std::move(metaAnim), in.readUint32Big());
    }

    return ret;
}

CMetaAnimRandom::CMetaAnimRandom(CInputStream& in)
: x4_randomData(CreateRandomData(in)) {}

std::shared_ptr<CAnimTreeNode>
CMetaAnimRandom::GetAnimationTree(const CAnimSysContext& animSys,
                                  const CMetaAnimTreeBuildOrders& orders) const
{
}

void CMetaAnimRandom::GetUniquePrimitives(std::set<CPrimitive>& primsOut) const
{
}

std::shared_ptr<CAnimTreeNode>
CMetaAnimRandom::VGetAnimationTree(const CAnimSysContext& animSys,
                                   const CMetaAnimTreeBuildOrders& orders) const
{
}

}
