#include "Runtime/Character/CAnimTreeNode.hpp"

namespace urde {

CAnimTreeEffectiveContribution CAnimTreeNode::GetContributionOfHighestInfluence() const {
  return VGetContributionOfHighestInfluence();
}

u32 CAnimTreeNode::GetNumChildren() const { return VGetNumChildren(); }

std::shared_ptr<IAnimReader> CAnimTreeNode::GetBestUnblendedChild() const { return VGetBestUnblendedChild(); }

} // namespace urde
