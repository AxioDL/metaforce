#include "CDecalDataFactory.hpp"
#include "CDecalDescription.hpp"
#include "CSimplePool.hpp"

namespace pshag
{
std::unique_ptr<IObj> FDealDataFactory(const SObjectTag &tag, CInputStream &in, const CVParamTransfer &vparms)
{
    CSimplePool* sp = static_cast<CSimplePool*>(static_cast<TObjOwnerParam<IObjectStore*>*>(vparms.GetObj())->GetParam());
    return TToken<CDecalDescription>::GetIObjObjectFor(std::unique_ptr<CDecalDescription>(CDecalDataFactory::GetGeneratorDesc(in, sp)));
}
}
