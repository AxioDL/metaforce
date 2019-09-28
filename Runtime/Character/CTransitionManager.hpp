#pragma once

#include <memory>

#include "Runtime/CToken.hpp"
#include "Runtime/Character/CAnimSysContext.hpp"

namespace urde {
class CAnimTreeNode;
class CRandom16;
class CSimplePool;
class CTransitionDatabaseGame;

class CTransitionManager {
  CAnimSysContext x0_animCtx;

public:
  CTransitionManager(const CAnimSysContext& sysCtx) : x0_animCtx(sysCtx) {}
  std::shared_ptr<CAnimTreeNode> GetTransitionTree(const std::shared_ptr<CAnimTreeNode>& a,
                                                   const std::shared_ptr<CAnimTreeNode>& b) const;
};

} // namespace urde
