#pragma once

#include <memory>

#include "Runtime/CToken.hpp"
#include "Runtime/Character/CAnimSysContext.hpp"
#include "Runtime/Character/CTransitionDatabaseGame.hpp"

namespace urde {
class CAnimTreeNode;
class CRandom16;
class CSimplePool;

class CTransitionManager {
  CAnimSysContext x0_animCtx;

public:
  explicit CTransitionManager(CAnimSysContext sysCtx) : x0_animCtx(std::move(sysCtx)) {}
  std::shared_ptr<CAnimTreeNode> GetTransitionTree(const std::shared_ptr<CAnimTreeNode>& a,
                                                   const std::shared_ptr<CAnimTreeNode>& b) const;
};

} // namespace urde
