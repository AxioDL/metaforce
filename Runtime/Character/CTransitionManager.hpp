#ifndef __URDE_CTRANSITIONMANAGER_HPP__
#define __URDE_CTRANSITIONMANAGER_HPP__

#include "CToken.hpp"
#include "CAnimSysContext.hpp"

namespace urde
{
class CTransitionDatabaseGame;
class CRandom16;
class CSimplePool;
class CAnimTreeNode;

class CTransitionManager
{
    CAnimSysContext x0_animCtx;
public:
    CTransitionManager(const CAnimSysContext& sysCtx) : x0_animCtx(sysCtx) {}
    std::shared_ptr<CAnimTreeNode> GetTransitionTree(const std::shared_ptr<CAnimTreeNode>& a,
                                                     const std::shared_ptr<CAnimTreeNode>& b) const;
};

}

#endif // __URDE_CTRANSITIONMANAGER_HPP__
