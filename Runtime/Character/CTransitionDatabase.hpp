#ifndef __RETRO_CTRANSITIONDATABASE_HPP__
#define __RETRO_CTRANSITIONDATABASE_HPP__

#include "../RetroTypes.hpp"

namespace Retro
{
class IMetaTrans;

class CTransitionDatabase
{
public:
    virtual const IMetaTrans* GetMetaTrans(u32, u32) const=0;
};

}

#endif // __RETRO_CTRANSITIONDATABASE_HPP__
