#include "CMetaAnimBlend.hpp"

namespace urde
{

CMetaAnimBlend::CMetaAnimBlend(CInputStream& in)
{
}

std::shared_ptr<CAnimTreeNode>
CMetaAnimBlend::GetAnimationTree(const CAnimSysContext& animSys,
                                 const CMetaAnimTreeBuildOrders& orders) const
{
}

void CMetaAnimBlend::GetUniquePrimitives(std::set<CPrimitive>& primsOut) const
{
}

std::shared_ptr<CAnimTreeNode>
CMetaAnimBlend::VGetAnimationTree(const CAnimSysContext& animSys,
                                  const CMetaAnimTreeBuildOrders& orders) const
{
}

}
