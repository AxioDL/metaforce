#ifndef __URDE_CTRANSITIONDATABASEGAME_HPP__
#define __URDE_CTRANSITIONDATABASEGAME_HPP__

#include "CTransitionDatabase.hpp"

namespace urde
{
class CTransition;
class CHalfTransition;

class CTransitionDatabaseGame final : public CTransitionDatabase
{
    std::shared_ptr<IMetaTrans> x10_defaultTrans;
    std::vector<std::pair<std::pair<u32, u32>, std::shared_ptr<IMetaTrans>>> x14_transitions;
    std::vector<std::pair<u32, std::shared_ptr<IMetaTrans>>> x24_halfTransitions;
public:
    CTransitionDatabaseGame(const std::vector<CTransition>& transitions,
                            const std::vector<CHalfTransition>& halfTransitions,
                            const std::shared_ptr<IMetaTrans>& defaultTrans);
    const std::shared_ptr<IMetaTrans>& GetMetaTrans(u32, u32) const;
};

}

#endif // __URDE_CTRANSITIONDATABASEGAME_HPP__
