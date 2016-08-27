#include "CMetaAnimSequence.hpp"
#include "CMetaAnimFactory.hpp"
#include "CAnimTreeSequence.hpp"

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

void CMetaAnimSequence::GetUniquePrimitives(std::set<CPrimitive>& primsOut) const
{
    for (const std::shared_ptr<IMetaAnim>& anim : x4_sequence)
        anim->GetUniquePrimitives(primsOut);
}

std::shared_ptr<CAnimTreeNode>
CMetaAnimSequence::VGetAnimationTree(const CAnimSysContext& animSys,
                                     const CMetaAnimTreeBuildOrders& orders) const
{
    if (orders.x0_)
    {
        CMetaAnimTreeBuildOrders modOrders;
        modOrders.PreAdvanceForAll(*orders.x0_);
        return GetAnimationTree(animSys, modOrders);
    }

    for (const std::shared_ptr<IMetaAnim>& anim : x4_sequence)
    {

    }

    std::shared_ptr<CAnimTreeNode> ret =
        std::make_shared<CAnimTreeSequence>(x4_sequence, animSys, "");

    return ret;
}

}
