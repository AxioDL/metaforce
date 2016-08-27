#include "CMetaAnimBlend.hpp"
#include "CMetaAnimFactory.hpp"

namespace urde
{

CMetaAnimBlend::CMetaAnimBlend(CInputStream& in)
{
    x4_animA = CMetaAnimFactory::CreateMetaAnim(in);
    x8_animB = CMetaAnimFactory::CreateMetaAnim(in);
    xc_blend = in.readFloatBig();
    x10_ = in.readBool();
}

void CMetaAnimBlend::GetUniquePrimitives(std::set<CPrimitive>& primsOut) const
{
}

std::shared_ptr<CAnimTreeNode>
CMetaAnimBlend::VGetAnimationTree(const CAnimSysContext& animSys,
                                  const CMetaAnimTreeBuildOrders& orders) const
{
    return {};
}

}
