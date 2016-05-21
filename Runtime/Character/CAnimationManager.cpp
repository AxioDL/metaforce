#include "CAnimationManager.hpp"
#include "CAnimationDatabaseGame.hpp"
#include "CTransitionDatabaseGame.hpp"

namespace urde
{

const CAnimationDatabaseGame* CAnimationManager::GetAnimationDatabase() const
{
    return x0_animDB.GetObj();
}

}
