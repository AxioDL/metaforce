#include "CMetaAnimPhaseBlend.hpp"
#include "CMetaAnimFactory.hpp"

namespace urde
{

CMetaAnimPhaseBlend::CMetaAnimPhaseBlend(CInputStream& in)
{
    x4_animA = CMetaAnimFactory::CreateMetaAnim(in);
    x8_animB = CMetaAnimFactory::CreateMetaAnim(in);
    xc_blend = in.readFloatBig();
    x10_ = in.readBool();
}

void CMetaAnimPhaseBlend::GetUniquePrimitives(std::set<CPrimitive>& primsOut) const
{
}

std::shared_ptr<CAnimTreeNode>
CMetaAnimPhaseBlend::VGetAnimationTree(const CAnimSysContext& animSys,
                                       const CMetaAnimTreeBuildOrders& orders) const
{
    return {};
}

}
