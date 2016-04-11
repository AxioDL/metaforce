#include "CMetaAnimPlay.hpp"

namespace urde
{

CMetaAnimPlay::CMetaAnimPlay(CInputStream& in)
: x4_primitive(in), x1c_(in.readFloatBig()), x20_(in.readUint32Big()) {}

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
