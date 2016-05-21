#ifndef __URDE_CANIMATIONMANAGER_HPP__
#define __URDE_CANIMATIONMANAGER_HPP__

#include "CToken.hpp"
#include "CAnimSysContext.hpp"

namespace urde
{
class CAnimationDatabaseGame;
class CTransitionDatabaseGame;
class CSimplePool;

class CAnimationManager
{
    TToken<CAnimationDatabaseGame> x0_animDB;
    CAnimSysContext x8_sysCtx;
public:
    CAnimationManager(const TToken<CAnimationDatabaseGame>& animDB,
                      const CAnimSysContext& sysCtx)
    : x0_animDB(animDB), x8_sysCtx(sysCtx) {}

    const CAnimationDatabaseGame* GetAnimationDatabase() const;
};

}

#endif // __URDE_CANIMATIONMANAGER_HPP__
