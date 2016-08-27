#include "CMetaAnimPlay.hpp"
#include "CAnimSysContext.hpp"
#include "CSimplePool.hpp"
#include "CAllFormatsAnimSource.hpp"
#include "CAnimTreeAnimReaderContainer.hpp"

namespace urde
{

CMetaAnimPlay::CMetaAnimPlay(CInputStream& in)
: x4_primitive(in), x1c_startTime(in) {}

std::shared_ptr<CAnimTreeNode>
CMetaAnimPlay::GetAnimationTree(const CAnimSysContext& animSys,
                                const CMetaAnimTreeBuildOrders& orders) const
{
    return {};
}

void CMetaAnimPlay::GetUniquePrimitives(std::set<CPrimitive>& primsOut) const
{
}

std::shared_ptr<CAnimTreeNode>
CMetaAnimPlay::VGetAnimationTree(const CAnimSysContext& animSys,
                                 const CMetaAnimTreeBuildOrders& orders) const
{
    if (orders.x0_)
    {
        CMetaAnimTreeBuildOrders modOrders;
        modOrders.PreAdvanceForAll(*orders.x0_);
        return GetAnimationTree(animSys, modOrders);
    }

    TLockedToken<CAllFormatsAnimSource> prim =
        animSys.xc_store.GetObj(SObjectTag{FOURCC('ANIM'), x4_primitive.GetAnimResId()});
    std::shared_ptr<CAnimTreeNode> ret =
        std::make_shared<CAnimTreeAnimReaderContainer>(x4_primitive.GetName(),
                                                       CAllFormatsAnimSource::GetNewReader(prim, x1c_startTime),
                                                       x4_primitive.GetAnimDbIdx());
    return ret;
}

}
