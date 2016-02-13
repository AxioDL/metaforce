#ifndef __PSHAG_CTRANSITIONDATABASE_HPP__
#define __PSHAG_CTRANSITIONDATABASE_HPP__

#include "../RetroTypes.hpp"

namespace pshag
{
class IMetaTrans;

class CTransitionDatabase
{
public:
    virtual const IMetaTrans* GetMetaTrans(u32, u32) const=0;
};

}

#endif // __PSHAG_CTRANSITIONDATABASE_HPP__
