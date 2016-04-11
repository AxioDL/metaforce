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

}

}
