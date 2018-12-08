#include "CAnimationManager.hpp"
#include "CAnimationDatabaseGame.hpp"
#include "CTransitionDatabaseGame.hpp"
#include "IMetaAnim.hpp"

namespace urde {

const CAnimationDatabaseGame* CAnimationManager::GetAnimationDatabase() const { return x0_animDB.GetObj(); }

std::shared_ptr<CAnimTreeNode> CAnimationManager::GetAnimationTree(u32 animIdx,
                                                                   const CMetaAnimTreeBuildOrders& orders) const {
  const std::shared_ptr<IMetaAnim>& anim = x0_animDB->GetMetaAnim(animIdx);
  return anim->GetAnimationTree(x8_sysCtx, orders);
}

const std::shared_ptr<IMetaAnim>& CAnimationManager::GetMetaAnimation(u32 idx) const {
  return x0_animDB->GetMetaAnim(idx);
}

} // namespace urde
