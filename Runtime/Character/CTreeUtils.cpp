#include "CTreeUtils.hpp"
#include "CAnimTreeNode.hpp"
#include "CAnimSysContext.hpp"
#include "CTransitionDatabaseGame.hpp"
#include "IMetaTrans.hpp"

namespace urde
{

std::shared_ptr<CAnimTreeNode> CTreeUtils::GetTransitionTree(const std::shared_ptr<CAnimTreeNode>& a,
                                                             const std::shared_ptr<CAnimTreeNode>& b,
                                                             const CAnimSysContext& animCtx)
{
    CAnimTreeEffectiveContribution contribA = a->GetContributionOfHighestInfluence();
    CAnimTreeEffectiveContribution contribB = b->GetContributionOfHighestInfluence();
    return animCtx.x0_transDB->GetMetaTrans(contribA.GetAnimDatabaseIndex(),
                                            contribB.GetAnimDatabaseIndex())->GetTransitionTree(a, b, animCtx);
}

}
