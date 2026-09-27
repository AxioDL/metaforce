#include "Kyoto/Animation/CAnimationManager.hpp"

#include "Kyoto/Animation/IMetaAnim.hpp"

rstl::ncrc_ptr< CAnimTreeNode >
CAnimationManager::GetAnimationTree(uint animIdx, const CMetaAnimTreeBuildOrders& orders) const {
  const rstl::rc_ptr< IMetaAnim >& anim = mAnimDB.NonConstCopy()->GetMetaAnim(animIdx);
  return anim->GetAnimationTree(mSysCtx, orders);
}

rstl::rc_ptr< IMetaAnim > CAnimationManager::GetMetaAnimation(uint animIdx) const {
  return mAnimDB.NonConstCopy()->GetMetaAnim(animIdx);
}


void CAnimationManager::ForceOrdering() {
  mAnimDB.NonConstCopy();
}