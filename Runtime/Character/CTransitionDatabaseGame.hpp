#ifndef __PSHAG_CTRANSITIONDATABASEGAME_HPP__
#define __PSHAG_CTRANSITIONDATABASEGAME_HPP__

#include "CTransitionDatabase.hpp"

namespace pshag
{

class CTransitionDatabaseGame : public CTransitionDatabase
{
public:
    const IMetaTrans* GetMetaTrans(u32, u32);
};

}

#endif // __PSHAG_CTRANSITIONDATABASEGAME_HPP__
