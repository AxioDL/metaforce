#include "CMetaAnimSequence.hpp"

namespace urde
{

CMetaAnimSequence::CMetaAnimSequence(CInputStream& in)
{
}

std::shared_ptr<CAnimTreeNode>
CMetaAnimSequence::GetAnimationTree(const CAnimSysContext& animSys,
                                    const CMetaAnimTreeBuildOrders& orders) const
{
}

void CMetaAnimSequence::GetUniquePrimitives(std::set<CPrimitive>& primsOut) const
{
}

std::shared_ptr<CAnimTreeNode>
CMetaAnimSequence::VGetAnimationTree(const CAnimSysContext& animSys,
                                     const CMetaAnimTreeBuildOrders& orders) const
{
}

}
