#include "IMetaAnim.hpp"
#include "CCharAnimTime.hpp"
#include "IAnimReader.hpp"
#include "CBoolPOINode.hpp"

namespace urde
{

std::shared_ptr<CAnimTreeNode>
IMetaAnim::GetAnimationTree(const CAnimSysContext& animSys,
                            const CMetaAnimTreeBuildOrders& orders) const
{
    return {};
}

void IMetaAnim::AdvanceAnim(IAnimReader& anim, const CCharAnimTime& dt)
{
    CCharAnimTime remDt = dt;
    while (remDt > CCharAnimTime())
    {
        SAdvancementResults res = anim.VAdvanceView(remDt);
        remDt = res.x0_remTime;
    }
}

CCharAnimTime IMetaAnim::GetTime(const CPreAdvanceIndicator& ind, const IAnimReader& anim)
{
    if (ind.IsTime())
        return ind.GetTime();

    CBoolPOINode nodes[64];
    CCharAnimTime rem = anim.VGetTimeRemaining();
    u32 count = anim.VGetBoolPOIList(rem, nodes, 64, 0, 0);
    const char* cmpStr = ind.GetString();
    for (u32 i=0 ; i<count ; ++i)
    {
        CBoolPOINode& node = nodes[i];
        if (node.GetName().compare(cmpStr) || !node.GetValue())
            continue;
        return node.GetTime();
    }

    return {};
}

}
