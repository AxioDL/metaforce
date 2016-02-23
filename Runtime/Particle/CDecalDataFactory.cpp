#include "CDecalDataFactory.hpp"
#include "CDecalDescription.hpp"
#include "CSimplePool.hpp"

namespace pshag
{
CDecalDescription* CDecalDataFactory::GetGeneratorDesc(CInputStream& in, CSimplePool* resPool)
{
    return nullptr;
}

std::unique_ptr<IObj> FDecalDataFactory(const SObjectTag &tag, CInputStream &in, const CVParamTransfer &vparms)
{
    CSimplePool* sp = static_cast<CSimplePool*>(static_cast<TObjOwnerParam<IObjectStore*>*>(vparms.GetObj())->GetParam());
    return TToken<CDecalDescription>::GetIObjObjectFor(std::unique_ptr<CDecalDescription>(CDecalDataFactory::GetGeneratorDesc(in, sp)));
}

}
