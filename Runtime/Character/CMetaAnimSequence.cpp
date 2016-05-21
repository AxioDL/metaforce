#include "CMetaAnimSequence.hpp"
#include "CMetaAnimFactory.hpp"

namespace urde
{

std::vector<std::shared_ptr<IMetaAnim>> CMetaAnimSequence::CreateSequence(CInputStream& in)
{
    std::vector<std::shared_ptr<IMetaAnim>> ret;
    u32 seqCount = in.readUint32Big();
    ret.reserve(seqCount);

    for (u32 i=0 ; i<seqCount ; ++i)
        ret.push_back(std::move(CMetaAnimFactory::CreateMetaAnim(in)));

    return ret;
}

CMetaAnimSequence::CMetaAnimSequence(CInputStream& in)
: x4_sequence(CreateSequence(in)) {}

std::shared_ptr<CAnimTreeNode>
CMetaAnimSequence::GetAnimationTree(const CAnimSysContext& animSys,
                                    const CMetaAnimTreeBuildOrders& orders) const
{
    return {};
}

void CMetaAnimSequence::GetUniquePrimitives(std::set<CPrimitive>& primsOut) const
{
}

std::shared_ptr<CAnimTreeNode>
CMetaAnimSequence::VGetAnimationTree(const CAnimSysContext& animSys,
                                     const CMetaAnimTreeBuildOrders& orders) const
{
    return {};
}

}
