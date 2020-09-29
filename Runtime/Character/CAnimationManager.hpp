#pragma once

#include <memory>

#include "Runtime/CToken.hpp"
#include "Runtime/Character/CAnimationDatabaseGame.hpp"
#include "Runtime/Character/CAnimSysContext.hpp"

namespace urde {
class CAnimTreeNode;
class CSimplePool;
class IMetaAnim;

struct CMetaAnimTreeBuildOrders;

class CAnimationManager {
  TToken<CAnimationDatabaseGame> x0_animDB;
  CAnimSysContext x8_sysCtx;

public:
  CAnimationManager(TToken<CAnimationDatabaseGame> animDB, CAnimSysContext sysCtx)
  : x0_animDB(std::move(animDB)), x8_sysCtx(std::move(sysCtx)) {}

  const CAnimationDatabaseGame* GetAnimationDatabase() const;
  std::shared_ptr<CAnimTreeNode> GetAnimationTree(s32, const CMetaAnimTreeBuildOrders& orders) const;
  const std::shared_ptr<IMetaAnim>& GetMetaAnimation(s32) const;
};

} // namespace urde
