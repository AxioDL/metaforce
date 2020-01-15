#include "Runtime/Character/CAnimationManager.hpp"

#include "Runtime/Character/CAnimationDatabaseGame.hpp"
#include "Runtime/Character/IMetaAnim.hpp"

namespace urde {

const CAnimationDatabaseGame* CAnimationManager::GetAnimationDatabase() const { return x0_animDB.GetObj(); }

std::shared_ptr<CAnimTreeNode> CAnimationManager::GetAnimationTree(s32 animIdx,
                                                                   const CMetaAnimTreeBuildOrders& orders) const {
  const std::shared_ptr<IMetaAnim>& anim = x0_animDB->GetMetaAnim(animIdx);
  return anim->GetAnimationTree(x8_sysCtx, orders);
}

const std::shared_ptr<IMetaAnim>& CAnimationManager::GetMetaAnimation(s32 idx) const {
  return x0_animDB->GetMetaAnim(idx);
}

} // namespace urde
