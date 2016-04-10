#include "CMetaAnimPlay.hpp"

namespace urde
{

CMetaAnimPlay::CMetaAnimPlay(CInputStream& in)
{
}

std::shared_ptr<CAnimTreeNode>
CMetaAnimPlay::GetAnimationTree(const CAnimSysContext& animSys,
                                const CMetaAnimTreeBuildOrders& orders) const
{
}

void CMetaAnimPlay::GetUniquePrimitives(std::set<CPrimitive>& primsOut) const
{
}

std::shared_ptr<CAnimTreeNode>
CMetaAnimPlay::VGetAnimationTree(const CAnimSysContext& animSys,
                                 const CMetaAnimTreeBuildOrders& orders) const
{
}

}
