#include "Runtime/Character/CTreeUtils.hpp"

#include "Runtime/Character/CAnimSysContext.hpp"
#include "Runtime/Character/CAnimTreeNode.hpp"
#include "Runtime/Character/CTransitionDatabaseGame.hpp"
#include "Runtime/Character/IMetaTrans.hpp"

namespace urde {

std::shared_ptr<CAnimTreeNode> CTreeUtils::GetTransitionTree(const std::weak_ptr<CAnimTreeNode>& a,
                                                             const std::weak_ptr<CAnimTreeNode>& b,
                                                             const CAnimSysContext& animCtx) {
  CAnimTreeEffectiveContribution contribA = a.lock()->GetContributionOfHighestInfluence();
  CAnimTreeEffectiveContribution contribB = b.lock()->GetContributionOfHighestInfluence();
  return animCtx.x0_transDB->GetMetaTrans(contribA.GetAnimDatabaseIndex(), contribB.GetAnimDatabaseIndex())
      ->GetTransitionTree(a, b, animCtx);
}

} // namespace urde
