#pragma once

#include <memory>

#include "Runtime/RetroTypes.hpp"

namespace urde {
class CAnimTreeNode;
struct CAnimSysContext;

class CTreeUtils {
public:
  static std::shared_ptr<CAnimTreeNode> GetTransitionTree(const std::weak_ptr<CAnimTreeNode>& a,
                                                          const std::weak_ptr<CAnimTreeNode>& b,
                                                          const CAnimSysContext& animCtx);
};

} // namespace urde
