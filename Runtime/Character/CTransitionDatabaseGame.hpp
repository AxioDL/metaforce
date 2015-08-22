#ifndef __RETRO_CTRANSITIONDATABASEGAME_HPP__
#define __RETRO_CTRANSITIONDATABASEGAME_HPP__

#include "CTransitionDatabase.hpp"

namespace Retro
{

class CTransitionDatabaseGame : public CTransitionDatabase
{
public:
    const IMetaTrans* GetMetaTrans(u32, u32);
};

}

#endif // __RETRO_CTRANSITIONDATABASEGAME_HPP__
