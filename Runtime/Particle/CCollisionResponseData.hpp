#ifndef __PSHAG_CCOLLISIONRESPONSEDATA_HPP__
#define __PSHAG_CCOLLISIONRESPONSEDATA_HPP__

#include "RetroTypes.hpp"
#include "IObj.hpp"
#include "CToken.hpp"
#include "IOStreams.hpp"

namespace pshag
{
class CSimplePool;

class CCollisionResponseData
{
public:
    CCollisionResponseData(CInputStream& in, CSimplePool* resPool);
};

std::unique_ptr<IObj> FCollisionResponseDataFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms);
}

#endif // __PSHAG_CCOLLISIONRESPONSEDATA_HPP__
