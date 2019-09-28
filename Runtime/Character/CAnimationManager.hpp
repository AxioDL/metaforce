#pragma once

#include <memory>

#include "Runtime/CToken.hpp"
#include "Runtime/Character/CAnimSysContext.hpp"

namespace urde {
class CAnimTreeNode;
class CAnimationDatabaseGame;
class CSimplePool;
class CTransitionDatabaseGame;
class IMetaAnim;

struct CMetaAnimTreeBuildOrders;

class CAnimationManager {
  TToken<CAnimationDatabaseGame> x0_animDB;
  CAnimSysContext x8_sysCtx;

public:
  CAnimationManager(const TToken<CAnimationDatabaseGame>& animDB, const CAnimSysContext& sysCtx)
  : x0_animDB(animDB), x8_sysCtx(sysCtx) {}

  const CAnimationDatabaseGame* GetAnimationDatabase() const;
  std::shared_ptr<CAnimTreeNode> GetAnimationTree(s32, const CMetaAnimTreeBuildOrders& orders) const;
  const std::shared_ptr<IMetaAnim>& GetMetaAnimation(s32) const;
};

} // namespace urde
