#pragma once

#include "CToken.hpp"
#include "CAnimSysContext.hpp"

namespace urde {
class CAnimationDatabaseGame;
class CTransitionDatabaseGame;
class CSimplePool;
class CAnimTreeNode;
struct CMetaAnimTreeBuildOrders;
class IMetaAnim;

class CAnimationManager {
  TToken<CAnimationDatabaseGame> x0_animDB;
  CAnimSysContext x8_sysCtx;

public:
  CAnimationManager(const TToken<CAnimationDatabaseGame>& animDB, const CAnimSysContext& sysCtx)
  : x0_animDB(animDB), x8_sysCtx(sysCtx) {}

  const CAnimationDatabaseGame* GetAnimationDatabase() const;
  std::shared_ptr<CAnimTreeNode> GetAnimationTree(u32, const CMetaAnimTreeBuildOrders& orders) const;
  const std::shared_ptr<IMetaAnim>& GetMetaAnimation(u32) const;
};

} // namespace urde
