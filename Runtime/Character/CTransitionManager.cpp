#include "CTransitionManager.hpp"
#include "CTreeUtils.hpp"

namespace urde {

std::shared_ptr<CAnimTreeNode> CTransitionManager::GetTransitionTree(const std::shared_ptr<CAnimTreeNode>& a,
                                                                     const std::shared_ptr<CAnimTreeNode>& b) const {
  return CTreeUtils::GetTransitionTree(a, b, x0_animCtx);
}

} // namespace urde
